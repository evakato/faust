#pragma once

#include <vulkan/vulkan.hpp>

#include "buffer.hpp"
#include "constants.hpp"
#include "context.hpp"
#include "image.hpp"
#include "io_utils.hpp"

struct RasterPipeline {
	static inline const std::array<vk::ClearValue, 2> clearValues = {
		vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 1.f}),
		vk::ClearDepthStencilValue{ 1.0f, 0 }
	};

	vk::RenderPass renderPass;
	vk::Framebuffer framebuffer;
	vk::UniquePipelineLayout pipelineLayout;
	vk::Pipeline pipeline;
	vk::UniqueDescriptorSet descriptorSet;

	void create(Context& context, std::array<vk::ImageView, 2> attachments, Buffer& cameraBuffer);
	void destroy(vk::Device device);
};

