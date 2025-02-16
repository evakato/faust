#pragma once

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "buffer.hpp"
#include "device.hpp"

class Texture {
public:
	Texture(FaustDevice& device);
	~Texture();
	VkImageView getImageView() const { return textureImageView; }
	VkSampler getSampler() const { return textureSampler; }

private:
	void createTextureImage();
	void createTextureImageView();
	void createTextureSampler();
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	FaustDevice& device;

	VkImage textureImage;
	VkImageView textureImageView;
	VkSampler textureSampler;
	VkDeviceMemory textureImageMemory;

};