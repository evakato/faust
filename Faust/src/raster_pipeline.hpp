#pragma once

#include <vulkan/vulkan.hpp>

#include "buffer.hpp"
#include "constants.hpp"
#include "context.hpp"
#include "image.hpp"
#include "io_utils.hpp"

struct RasterPipeline {
	static inline const std::vector<vk::ClearValue> clearValues = {
		vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 1.f}),
		vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.f}),
		vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.f}),
		vk::ClearDepthStencilValue{ 1.0f, 0 }
	};

	vk::RenderPass renderPass;
	vk::Framebuffer framebuffer;
	vk::UniquePipelineLayout pipelineLayout;
	vk::Pipeline pipeline;
	vk::UniqueDescriptorSetLayout descriptorSetLayout;
	vk::UniqueDescriptorSet descriptorSet;

	std::vector<vk::DescriptorSetLayoutBinding> bindings = {
		{ 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },
	};

	void createFramebuffer(vk::Device device, std::vector<vk::ImageView> attachments);
	void updateDescriptorSet(vk::Device device, const Buffer& buffer, const Image& image);

	void create(Context& context, std::vector<vk::ImageView> attachments, Buffer& cameraBuffer);
	void destroy(vk::Device device);
};

