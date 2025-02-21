#pragma once

#include <vulkan/vulkan.h>

#include "device.hpp"

class Image {
public:
	Image(FaustDevice& device);
	~Image();

	VkImageView getImageView() const { return imageView; }
	void createImageView(VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels = 1);
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t mipLevels = 1, VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT);

protected:
	FaustDevice& device;

	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
};