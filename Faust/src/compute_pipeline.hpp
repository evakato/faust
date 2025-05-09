#pragma once

#include <vulkan/vulkan.hpp>
#include <fstream>
#include <vector>
#include <cassert>

#include "context.hpp"
#include "io_utils.hpp"

std::vector<vk::DescriptorSetLayoutBinding> temporalAccumulationBindings = {
	{ 0, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute },
	{ 1, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute },
	{ 2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute },
	{ 3, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute },
	{ 4, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute },
	{ 5, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute },
	{ 6, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute },
	{ 7, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute },
	{ 8, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute },
	{ 9, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute },
	{ 10, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute },
	{ 11, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute },
	{ 12, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute },
};

std::vector<vk::DescriptorSetLayoutBinding> spatialFilterBindings = {
	{ 0, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute }, // normals
	{ 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute }, // depth
	{ 2, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute }, // integrated color (moment 1)
	{ 3, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute }, // variance input
	{ 4, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute }, // variance output
	{ 5, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute }, // output image
};

struct ComputePipeline {
	vk::Pipeline pipeline;
	vk::PipelineLayout layout;
	vk::UniqueDescriptorSetLayout descriptorSetLayout;
	//vk::UniqueDescriptorSet descriptorSet;
	std::vector<vk::UniqueDescriptorSet> descriptorSets;
	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	int descriptorSetsCount;

	void allocateDescriptorSets(Context& context) {
		descriptorSets.resize(descriptorSetsCount);
		for (int i = 0; i < descriptorSetsCount; ++i) {
			descriptorSets[i] = context.allocateDescriptorSet(*descriptorSetLayout);
		}
	}

	ComputePipeline(Context& context, const std::string& shaderFilename, std::vector<vk::DescriptorSetLayoutBinding> bindings, int descriptorSetsCount = 1) : bindings{ bindings }, descriptorSetsCount{ descriptorSetsCount } {
		const std::vector<char> spirv = faust::readFile(shaderFilename);

		vk::ShaderModule shaderModule = context.device->createShaderModule({ {}, spirv.size(), reinterpret_cast<const uint32_t*>(spirv.data()) });

		vk::DescriptorSetLayoutCreateInfo dslCI({}, bindings.size(), bindings.data());
		descriptorSetLayout = context.device->createDescriptorSetLayoutUnique(dslCI);

		vk::PushConstantRange pushRange;
		pushRange.setOffset(0);
		pushRange.setSize(sizeof(int));
		pushRange.setStageFlags(vk::ShaderStageFlagBits::eCompute);

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.setPushConstantRanges(pushRange);
		pipelineLayoutInfo.setSetLayouts(descriptorSetLayout.get());
		layout = context.device->createPipelineLayout(pipelineLayoutInfo);

		vk::PipelineShaderStageCreateInfo stageCI({}, vk::ShaderStageFlagBits::eCompute, shaderModule, "main");

		vk::ComputePipelineCreateInfo pipelineCI({}, stageCI, layout);
		pipeline = context.device->createComputePipeline({}, pipelineCI).value;

		context.device->destroyShaderModule(shaderModule);

		allocateDescriptorSets(context);
	}

	void updateDescriptorSet(vk::Device device, const std::vector<vk::DescriptorImageInfo>& imageDescInfos, int index = 0) {
		assert(bindings.size() == imageDescInfos.size());
		std::vector<vk::WriteDescriptorSet> writes(bindings.size());
		for (int i = 0; i < bindings.size(); i++) {
			writes[i].setDstSet(*descriptorSets[index]);
			writes[i].setDescriptorType(bindings[i].descriptorType);
			writes[i].setDescriptorCount(bindings[i].descriptorCount);
			writes[i].setDstBinding(bindings[i].binding);
			writes[i].setImageInfo(imageDescInfos[i]);
		}
		device.updateDescriptorSets(writes, nullptr);
	}

};

