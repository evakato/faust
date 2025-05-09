#pragma once

#include <array>

#include "context.hpp"
#include "image.hpp"

struct GbufferSet {
	std::array<Image, 2> normalImages;
	std::array<Image, 2> depthImages;

	GbufferSet(Context& context, uint32_t width, uint32_t height) {
		for (int i = 0; i < 2; ++i) {
			normalImages[i] = Image{
				context,
				{ width, height },
				vk::Format::eR16G16B16A16Sfloat,
				vk::ImageUsageFlagBits::eColorAttachment |
				vk::ImageUsageFlagBits::eSampled |
				vk::ImageUsageFlagBits::eStorage,
				vk::ImageAspectFlagBits::eColor,
				vk::ImageLayout::eColorAttachmentOptimal
			};
			normalImages[i].descImageInfo.setImageLayout(vk::ImageLayout::eGeneral);

			depthImages[i] = Image{
				context,
				{ width, height },
				vk::Format::eD32Sfloat,
				vk::ImageUsageFlagBits::eDepthStencilAttachment |
				vk::ImageUsageFlagBits::eSampled,
				vk::ImageAspectFlagBits::eDepth,
				vk::ImageLayout::eDepthStencilAttachmentOptimal
			};
			depthImages[i].descImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
			depthImages[i].createSampler(*context.device);
		}
	}

	Image& currentNormal(uint32_t frameIndex) { return normalImages[frameIndex % 2]; }
	Image& previousNormal(uint32_t frameIndex) { return normalImages[(frameIndex + 1) % 2]; }

	Image& currentDepth(uint32_t frameIndex) { return depthImages[frameIndex % 2]; }
	Image& previousDepth(uint32_t frameIndex) { return depthImages[(frameIndex + 1) % 2]; }
};
