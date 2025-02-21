#pragma once

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "buffer.hpp"
#include "device.hpp"
#include "image.hpp"

class Texture : public Image {
public:
	Texture(FaustDevice& device, const std::string& imagePath);
	~Texture();
	VkSampler getSampler() const { return textureSampler; }

private:
	void createTextureImage(const std::string& imagePath);
	void createTextureSampler();
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void generateMipmaps(int32_t texWidth, int32_t texHeight, VkFormat imageFormat);

	VkSampler textureSampler;
	uint32_t mipLevels;
};