#pragma once
#include "context.hpp"
#include "image.hpp"
#include "buffer.hpp"
#include "io_utils.hpp"

enum class FilterOptions {
	OddWavelet,
	EvenWavelet,
	NoSVGF
};

class FullscreenPresentPipeline {
public:
	vk::RenderPass renderPass;
	std::vector<vk::UniqueFramebuffer> framebuffers;
	vk::Pipeline pipeline;
	vk::PipelineLayout pipelineLayout;
	vk::DescriptorSetLayout descriptorSetLayout;
	std::vector<vk::UniqueDescriptorSet> descriptorSets;

	static inline const std::vector<vk::ClearValue> clearValues = {
		vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 1.f}),
	};

	std::vector<vk::DescriptorSetLayoutBinding> bindings = {
		// binding 0: filteredImage sampler2D
		{ 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment }
	};

	void createFramebuffers(vk::Device device, const std::vector<vk::UniqueImageView>& swapchainImageViews) {
		framebuffers.resize(swapchainImageViews.size());

		for (size_t i = 0; i < swapchainImageViews.size(); ++i) {
			vk::ImageView attachments[] = { *swapchainImageViews[i] };

			vk::FramebufferCreateInfo framebufferInfo{};
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = faust::WIDTH;
			framebufferInfo.height = faust::HEIGHT;
			framebufferInfo.layers = 1;

			framebuffers[i] = device.createFramebufferUnique(framebufferInfo);
		}
	}

	void create(Context& context, const std::vector<vk::UniqueImageView>& swapchainImageViews, int descriptorSetsCount) {
		vk::Device device = *context.device;

		// Render pass for presenting
		vk::AttachmentDescription colorAttachment = {};
		colorAttachment.format = vk::Format::eB8G8R8A8Unorm;
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

		vk::AttachmentReference colorAttachmentRef = { 0, vk::ImageLayout::eColorAttachmentOptimal };

		vk::SubpassDescription subpass = {};
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		vk::RenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		renderPass = device.createRenderPass(renderPassInfo);

		// Create framebuffer for current swapchain image view
		createFramebuffers(device, swapchainImageViews);

		// Load shaders
		auto vertCode = faust::readFile("C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\shaders\\fullscreen.vert.spv");
		auto fragCode = faust::readFile("C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\shaders\\fullscreen.frag.spv");

		vk::ShaderModule vertShader = device.createShaderModule({ {}, vertCode.size(), (uint32_t*)vertCode.data() });
		vk::ShaderModule fragShader = device.createShaderModule({ {}, fragCode.size(), (uint32_t*)fragCode.data() });

		vk::PipelineShaderStageCreateInfo shaderStages[] = {
			{ {}, vk::ShaderStageFlagBits::eVertex, vertShader, "main" },
			{ {}, vk::ShaderStageFlagBits::eFragment, fragShader, "main" },
		};

		// No vertex input
		vk::PipelineVertexInputStateCreateInfo vertexInput = {};

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;

		vk::Viewport viewport = { 0.0f, 0.0f, float(faust::WIDTH), float(faust::HEIGHT), 0.0f, 1.0f };
		vk::Rect2D scissor = { {0, 0}, {faust::WIDTH, faust::HEIGHT} };
		vk::PipelineViewportStateCreateInfo viewportState = { {}, 1, &viewport, 1, &scissor };

		vk::PipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.polygonMode = vk::PolygonMode::eFill;
		rasterizer.cullMode = vk::CullModeFlagBits::eNone;
		rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
		rasterizer.lineWidth = 1.0f;

		vk::PipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

		vk::PipelineColorBlendAttachmentState blendState = {};
		blendState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		vk::PipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &blendState;

		vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.setBindings(bindings);
		descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.setSetLayouts(descriptorSetLayout);
		pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

		vk::GraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInput;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		pipeline = device.createGraphicsPipeline({}, pipelineInfo).value;
		allocateDescriptorSets(context, descriptorSetsCount);
	}

	void allocateDescriptorSets(Context& context, int descriptorSetsCount) {
		descriptorSets.resize(descriptorSetsCount);
		for (int i = 0; i < descriptorSetsCount; ++i) {
			descriptorSets[i] = context.allocateDescriptorSet(descriptorSetLayout);
		}
	}

	void updateDescriptorSet(vk::Device device, const Image& filteredImage, int index) {
		vk::DescriptorImageInfo imageInfo = filteredImage.descImageInfo;
		vk::WriteDescriptorSet write = {};
		write.dstSet = *descriptorSets[index];
		write.dstBinding = 0;
		write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		write.descriptorCount = 1;
		write.pImageInfo = &imageInfo;
		device.updateDescriptorSets(write, nullptr);
	}
};
