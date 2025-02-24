#pragma once

#include <stdexcept>

#include <stb_image.h>
#include <vulkan/vulkan.h>

#include "buffer.hpp"
#include "device.hpp"
#include "image.hpp"

struct ImageSTB {
	int width;
	int height;
	stbi_uc* pixels;
};

ImageSTB loadImageSTB(const std::string& imagePath);

class Texture : public Image {
public:
	Texture(FaustDevice& device);
	Texture(FaustDevice& device, const std::string& imagePath);
	~Texture();
	VkSampler getSampler() const { return textureSampler; }

protected:
	void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount = 1);
	void createTextureSampler(VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT, VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS);

private:
	void createTextureImage(const std::string& imagePath);
	void copyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height);
	void generateMipmaps(int32_t texWidth, int32_t texHeight, VkFormat imageFormat);

	VkSampler textureSampler;
	uint32_t mipLevels = 1;
};