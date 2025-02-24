#include "image.hpp"

Image::Image(FaustDevice& device) : device{ device } {}

Image::~Image() {
	vkDestroyImageView(device.getDevice(), imageView, nullptr);
	vkDestroyImage(device.getDevice(), image, nullptr);
	vkFreeMemory(device.getDevice(), imageMemory, nullptr);
}

void Image::createImageView(VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageViewType viewType, uint32_t layerCount) {
	// maybe combine this with renderer creatImageViews() function
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = viewType;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = layerCount;

	if (vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}
}

void Image::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t mipLevels, VkSampleCountFlagBits numSamples, uint32_t arrayLayers, VkImageCreateFlags flags) {

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = arrayLayers;
	imageInfo.format = format;
	imageInfo.tiling = tiling; // optimal access order
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // not usable by gpu and texels not preserved during first transition
	imageInfo.usage = usage; // usage sampled bit = enable access to the image from the shader
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = numSamples;
	imageInfo.flags = flags;
	if (vkCreateImage(device.getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	// allocate memory for an image
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device.getDevice(), image, &memRequirements);
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FaustDevice::findMemoryType(memRequirements.memoryTypeBits, properties, device.getPhysicalDevice());

	if (vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}
	vkBindImageMemory(device.getDevice(), image, imageMemory, 0);
}


