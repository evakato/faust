#pragma once

#include "context.hpp"

struct Image {
	Image() = default;
	Image(const Context& context, vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlagBits aspectFlag = vk::ImageAspectFlagBits::eColor, vk::ImageLayout outImageLayout = vk::ImageLayout::eGeneral);
	static vk::AccessFlags toAccessFlags(vk::ImageLayout layout);
	static void setImageLayout(vk::CommandBuffer commandBuffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlagBits aspectFlag = vk::ImageAspectFlagBits::eColor);
	static void copyImage(vk::CommandBuffer commandBuffer, vk::Image srcImage, vk::Image dstImage);

	vk::UniqueImage image;
	vk::UniqueImageView view;
	vk::UniqueDeviceMemory memory;
	vk::DescriptorImageInfo descImageInfo;

	void setDescImageLayout(vk::ImageLayout imageLayout) {
		descImageInfo.setImageLayout(imageLayout);
	}
};

