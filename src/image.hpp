#pragma once

#include <vulkan/vulkan.h>

#include "device.hpp"

class Image {
public:
	Image(FaustDevice& device);
	~Image();

	VkImageView getImageView() const { return imageView; }
	void createImageView(VkFormat format, VkImageAspectFlags aspectFlags);
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

protected:
	FaustDevice& device;

	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
};