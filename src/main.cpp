#pragma once

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>

#include "buffer.hpp"
#include "camera.hpp"
#include "cubemap.hpp"
#include "math.hpp"
#include "model.hpp"
#include "image.hpp"
#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "renderer.hpp"
#include "texture.hpp"
#include "state.hpp"
#include "light.hpp"

const uint32_t WIDTH = 1600;
const uint32_t HEIGHT = 900;

const glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0, 3.0, -1.0));

const AmbientLight al{ glm::vec4{1.0f, 1.0f, 1.0f, 0.1f } };

auto& state = FaustState::getInstance();

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) glm::mat4 normalMat;
	alignas(16) glm::mat4 invView;
	alignas(16) glm::vec4 directionalLight;
	alignas(16) PointLight pointLight;
	alignas(16) AmbientLight ambientLight;
};

std::string floorPath = "assets/models/floor.obj";

class HelloTriangleApplication {
public:
	void run() {
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	FaustWindow window{ WIDTH, HEIGHT };
	FaustDevice device2{ window };
	FaustRenderer renderer{ device2, window };

	FaustPipeline pipeline{ device2, "shaders/basic.vert.spv", "shaders/basic.frag.spv" };
	FaustPipeline pointLightPipeline{ device2, "shaders/point_light.vert.spv", "shaders/point_light.frag.spv" };
	FaustPipeline cubemapPipeline{ device2, "shaders/cubemap.vert.spv", "shaders/cubemap.frag.spv" };

	Texture texture{ device2, FaustState::getInstance().texturePath };
	Camera camera{ glm::vec3(0.f, 2.f, 8.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.0f, 1.0f, 0.0f) };
	Model mainModel{ device2, FaustState::getInstance().modelPath };
	//Model floorModel{ device2, floorPath };
	std::vector<Model*> models;

	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<Buffer> uniformBuffers;
	std::vector<void*> uniformBuffersMapped;

	uint32_t currentFrame = 0;

	PointLight pointLight1;

	Cubemap cubemap{ device2, FaustState::getInstance().cubemapPath };

	void initVulkan() {
		models.push_back(&mainModel);
		//models.push_back(&floorModel);
		state.numTris = static_cast<float>(mainModel.getIndexCount()) / 3.0f;
		state.setPointLightCol(pointLight1.color);
		state.pointLightPos = pointLight1.position;

		createDescriptorSetLayout();
		createPipelineLayout();

		auto renderPass = renderer.getRenderPass();

		{
			auto bindingDescriptions = { Vertex::getBindingDescription() };
			auto attributeDescriptions = Vertex::getAttributeDescriptions();
			PipelineCreateInfo createInfo{ bindingDescriptions, attributeDescriptions };
			pipeline.createGraphicsPipeline(renderPass, pipelineLayout, createInfo);
		}
		{
			PipelineCreateInfo createInfo{ };
			createInfo.enableAlphaBlending();
			pointLightPipeline.createGraphicsPipeline(renderPass, pipelineLayout, createInfo);
		}
		{
			PipelineCreateInfo createInfo{ };
			createInfo.rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
			createInfo.depthStencil.depthWriteEnable = VK_FALSE;
			createInfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			cubemapPipeline.createGraphicsPipeline(renderPass, pipelineLayout, createInfo);
		}

		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();

		camera.setPerspectiveProjection(glm::radians(45.0f), (float)renderer.getExtent().width / (float)renderer.getExtent().height, 0.1f, 100.0f);
	}

	void mainLoop() {
		while (!window.shouldClose()) {
			window.pollEvents();

			while (!renderer.beginFrame(currentFrame)) {};

			auto& state = FaustState::getInstance();
			state.currentKeyPress = window.detectKeypress();
			auto mouseMovement = window.detectMouseMovement();
			camera.rotate(mouseMovement.x, mouseMovement.y);

			switch (state.currentKeyPress) {
			case KeyPress::CameraLeft:
				camera.translate(glm::vec3{ cameraTranslateNeg, 0.f, 0.f });
				break;
			case KeyPress::CameraRight:
				camera.translate(glm::vec3{ cameraTranslatePos, 0.f, 0.f });
				break;
			case KeyPress::CameraUp:
				camera.translate(glm::vec3{ 0.f, cameraTranslatePos, 0.f });
				break;
			case KeyPress::CameraDown:
				camera.translate(glm::vec3{ 0.f, cameraTranslateNeg, 0.f });
				break;
			case KeyPress::CameraForward:
				camera.translate(glm::vec3{ 0.f, 0.f, cameraTranslatePos });
				break;
			case KeyPress::CameraBackward:
				camera.translate(glm::vec3{ 0.f, 0.f, cameraTranslateNeg });
				break;
			case KeyPress::CameraViewLeft:
				camera.rotate(-cameraPan, 0.0f);
				break;
			case KeyPress::CameraViewRight:
				camera.rotate(cameraPan, 0.0f);
				break;
			case KeyPress::CameraViewUp:
				camera.rotate(0.0f, cameraPan);
				break;
			case KeyPress::CameraViewDown:
				camera.rotate(0.0f, -cameraPan);
				break;
			}
			if (state.currentKeyPress != KeyPress::None)
				state.currentKeyPress = KeyPress::None;

			if (state.modelChanged) {
				mainModel.setupNewModel(state.modelPath);
				state.modelChanged = false;
			}

			updateUniformBuffer(currentFrame);

			DrawFrameParams params;
			params.currentFrame = currentFrame;
			params.pipelineLayout = pipelineLayout;
			params.descriptorSets = descriptorSets;
			params.models = models;
			params.pipeline = &pipeline;
			params.pointLightPipeline = &pointLightPipeline;
			params.cubemapPipeline = &cubemapPipeline;
			renderer.drawFrame(params);
		}

		vkDeviceWaitIdle(device2.getDevice());
	}

	void cleanup() {
		auto device = device2.getDevice();
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	}

	void createPipelineLayout() {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		if (vkCreatePipelineLayout(device2.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void createDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1; // number of uniform buffer objects
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // which shader stage the descriptor is going to be referenced
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional, for image sampling

		// combined image sampler descriptor
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // where color of fragment will be determined

		VkDescriptorSetLayoutBinding cubemapSamplerLayoutBinding{};
		cubemapSamplerLayoutBinding.binding = 2;
		cubemapSamplerLayoutBinding.descriptorCount = 1;
		cubemapSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		cubemapSamplerLayoutBinding.pImmutableSamplers = nullptr;
		cubemapSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 3> bindings = { uboLayoutBinding, samplerLayoutBinding, cubemapSamplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device2.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void createDescriptorPool() {
		std::array<VkDescriptorPoolSize, 3> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(device2.getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void createDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout); // all descriptor sets have the same layout
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		// allocate descriptor sets, each with one uniform buffer descriptor
		if (vkAllocateDescriptorSets(device2.getDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo bufferInfo{}; // our descriptors refer to buffers
			bufferInfo.buffer = uniformBuffers[i].getBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			// specify combined image sampler
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texture.getImageView();
			imageInfo.sampler = texture.getSampler();

			VkDescriptorImageInfo cubemapInfo{};
			cubemapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			cubemapInfo.imageView = cubemap.getImageView();
			cubemapInfo.sampler = cubemap.getSampler();

			std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = descriptorSets[i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pImageInfo = &cubemapInfo;

			vkUpdateDescriptorSets(device2.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			uniformBuffers.emplace_back(device2);
			uniformBuffers.back().createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			vkMapMemory(device2.getDevice(), uniformBuffers[i].getBufferMemory(), 0, bufferSize, 0, &uniformBuffersMapped[i]); // buffer stays mapped to pointer for the app's whole lifetime "persistent mapping"
		}
	}

	void updateUniformBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		//ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.model = glm::mat4(1.0f);
		ubo.view = camera.getView();
		ubo.invView = camera.getInvView();
		ubo.proj = camera.getProjection();
		ubo.normalMat = glm::inverse(glm::transpose(ubo.model));
		ubo.directionalLight = glm::vec4{ lightDir, static_cast<int>(FaustState::getInstance().shadingSetting) };

		pointLight1.color = state.getPointLightCol();

		ubo.pointLight = pointLight1;
		ubo.ambientLight = al;

		//state.pointLightPos = ubo.pointLight.position;

		memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}