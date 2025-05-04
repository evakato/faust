#pragma once

#include "context.hpp"

struct Image {
	Image() = default;
	Image(const Context& context, vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlagBits aspectFlag = vk::ImageAspectFlagBits::eColor, vk::ImageLayout outImageLayout = vk::ImageLayout::eGeneral);
	static vk::AccessFlags toAccessFlags(vk::ImageLayout layout);
	static void setImageLayout(vk::CommandBuffer commandBuffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlagBits aspectFlag = vk::ImageAspectFlagBits::eColor);
	static void copyImage(vk::CommandBuffer commandBuffer, vk::Image srcImage, vk::Image dstImage);
	void createSampler(vk::Device device);
	void updateDescInfo();

	vk::UniqueImage image;
	vk::UniqueImageView view;
	vk::UniqueDeviceMemory memory;
	vk::DescriptorImageInfo descImageInfo;
	vk::Sampler sampler;
	vk::Format format;

	void updateDescriptor() {
		descImageInfo.setSampler(sampler);
		descImageInfo.setImageView(*view);
		descImageInfo.setImageLayout(vk::ImageLayout::eGeneral);
	}

	void setDescImageLayout(vk::ImageLayout imageLayout) {
		descImageInfo.setImageLayout(imageLayout);
	}

	void transitionImageLayout(
		vk::CommandBuffer commandBuffer,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		uint32_t mipLevels,
		uint32_t layerCount,
		vk::ImageAspectFlags aspectMask = {},
		uint32_t baseMipLevel = 0,
		uint32_t baseArrayLayer = 0,
		uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED
	) {

		descImageInfo.setImageLayout(newLayout);
		if (!aspectMask) {
			if (format == vk::Format::eD32Sfloat ||
				format == vk::Format::eD16Unorm ||
				format == vk::Format::eX8D24UnormPack32) {
				aspectMask = vk::ImageAspectFlagBits::eDepth;
			}
			else if (format == vk::Format::eD24UnormS8Uint ||
				format == vk::Format::eD32SfloatS8Uint) {
				aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
			}
			else {
				aspectMask = vk::ImageAspectFlagBits::eColor;
			}
		}

		vk::ImageSubresourceRange subresourceRange{
			aspectMask,
			baseMipLevel,
			mipLevels,
			baseArrayLayer,
			layerCount
		};

		vk::AccessFlags srcAccessMask;
		vk::AccessFlags dstAccessMask;
		vk::PipelineStageFlags srcStage;
		vk::PipelineStageFlags dstStage;

		if (oldLayout == vk::ImageLayout::eUndefined &&
			newLayout == vk::ImageLayout::eTransferDstOptimal) {
			srcAccessMask = {};
			dstAccessMask = vk::AccessFlagBits::eTransferWrite;
			srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
			dstStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eGeneral &&
			newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			srcAccessMask = vk::AccessFlagBits::eShaderRead |
				vk::AccessFlagBits::eShaderWrite |
				vk::AccessFlagBits::eTransferRead |
				vk::AccessFlagBits::eTransferWrite;
			dstAccessMask = vk::AccessFlagBits::eTransferWrite;
			srcStage = vk::PipelineStageFlagBits::eTransfer;
			dstStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
			newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
			srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			dstAccessMask = vk::AccessFlagBits::eShaderRead;
			srcStage = vk::PipelineStageFlagBits::eTransfer;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined &&
			newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
			srcAccessMask = {};
			dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead |
				vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
			dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined &&
			newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
			srcAccessMask = {};
			dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
			dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		}
		else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal &&
			newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
			srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			dstAccessMask = vk::AccessFlagBits::eShaderRead;
			srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
			newLayout == vk::ImageLayout::eGeneral) {
			srcAccessMask = vk::AccessFlagBits::eShaderRead;
			dstAccessMask = vk::AccessFlagBits::eShaderWrite;
			srcStage = vk::PipelineStageFlagBits::eFragmentShader; // or eComputeShader, depending on usage
			dstStage = vk::PipelineStageFlagBits::eComputeShader;
		}
		else if (oldLayout == vk::ImageLayout::eGeneral &&
			newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
			srcAccessMask = vk::AccessFlagBits::eShaderWrite;
			dstAccessMask = vk::AccessFlagBits::eShaderRead;
			srcStage = vk::PipelineStageFlagBits::eComputeShader;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader; // or eComputeShader, depending on usage
		}
		else if (oldLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal &&
			newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
			srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			dstAccessMask = vk::AccessFlagBits::eShaderRead;
			srcStage = vk::PipelineStageFlagBits::eLateFragmentTests;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader; // or eComputeShader if used there
		}
		else {
			throw std::runtime_error("Unsupported layout transition");
		}


		vk::ImageMemoryBarrier barrier{
			srcAccessMask,
			dstAccessMask,
			oldLayout,
			newLayout,
			srcQueueFamilyIndex,
			dstQueueFamilyIndex,
			*image,
			subresourceRange
		};

		commandBuffer.pipelineBarrier(
			srcStage,
			dstStage,
			{}, // dependencyFlags
			nullptr,
			nullptr,
			barrier
		);
	}


};

