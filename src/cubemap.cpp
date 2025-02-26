#include "cubemap.hpp"

Cubemap::Cubemap(FaustDevice& device, const std::string& imagePath) : Texture{ device } {
	createCubemapImage(imagePath);
	createTextureSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_COMPARE_OP_NEVER);
	createImageView(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_IMAGE_VIEW_TYPE_CUBE, 6);
}

Cubemap::~Cubemap() {}

void Cubemap::createCubemapImage(const std::string& imagePath) {
	std::array<std::string, 6> cubemapPaths = {
	"posx.jpg", "negx.jpg", "posy.jpg",
	"negy.jpg", "posz.jpg", "negz.jpg"
	};

	int width, height;
	std::array<stbi_uc*, 6> pixels;

	for (size_t i = 0; i < 6; i++) {
		auto path = imagePath + "/" + cubemapPaths[i];
		auto [texWidth, texHeight, currPixels] = loadImageSTB(path);
		pixels[i] = currPixels;

		if (i == 0) {
			width = texWidth;
			height = texHeight;
		}
		else if (texWidth != width || texHeight != height) {
			throw std::runtime_error("Cubemap face " + path + " has different dimensions!");
		}
	}

	VkDeviceSize imageSize = width * height * 4;
	VkDeviceSize totalSize = imageSize * 6;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Buffer::createBuffer(totalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device.getDevice(), device.getPhysicalDevice());

	void* data;
	vkMapMemory(device.getDevice(), stagingBufferMemory, 0, totalSize, 0, &data);
	for (int i = 0; i < 6; ++i) {
		memcpy(static_cast<char*>(data) + i * imageSize, pixels[i], static_cast<size_t>(imageSize));
	}
	vkUnmapMemory(device.getDevice(), stagingBufferMemory);

	for (size_t i = 0; i < 6; i++) {
		stbi_image_free(pixels[i]);
	}

	// create
	createImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, VK_SAMPLE_COUNT_1_BIT, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

	transitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6);
	copyBufferToImages(stagingBuffer, width, height, imageSize);
	transitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6);

	vkDestroyBuffer(device.getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device.getDevice(), stagingBufferMemory, nullptr);
}

void Cubemap::copyBufferToImages(VkBuffer buffer, uint32_t width, uint32_t height, VkDeviceSize imageSize) {
	VkCommandBuffer commandBuffer = Buffer::beginSingleTimeCommands(device);

	std::vector<VkBufferImageCopy> bufferCopyRegions;
	for (uint32_t face = 0; face < 6; face++) {
		for (uint32_t level = 0; level < 1; level++) {
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = level;
			bufferCopyRegion.imageSubresource.baseArrayLayer = face;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = width >> level;
			bufferCopyRegion.imageExtent.height = height >> level;
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = face * imageSize;
			bufferCopyRegions.push_back(bufferCopyRegion);
		}
	}

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data()); // assume that the image has already been transitioned to an optimal layout for copying pixels to

	Buffer::endSingleTimeCommands(device, commandBuffer);
}
