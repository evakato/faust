#include <vector>
#include <fstream>

#include "pipeline.hpp"

FaustPipeline::FaustPipeline(FaustDevice& device, const std::string& vertShader, const std::string& fragShader) : device{ device }
{
	setShaders(vertShader, fragShader);
};

FaustPipeline::~FaustPipeline() {
	vkDestroyPipeline(device.getDevice(), pipeline, nullptr);
}

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

VkShaderModule FaustPipeline::createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device.getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void FaustPipeline::setShaders(const std::string& vertShader, const std::string& fragShader) {
	auto vertShaderCode = readFile(vertShader);
	auto fragShaderCode = readFile(fragShader);

	shaderModules.resize(2);

	shaderModules[0] = createShaderModule(vertShaderCode);
	shaderModules[1] = createShaderModule(fragShaderCode);

	shaderStages.resize(2);

	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = shaderModules[0];
	shaderStages[0].pName = "main";

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = shaderModules[1];
	shaderStages[1].pName = "main";
}

void FaustPipeline::createGraphicsPipeline(VkRenderPass renderPass, VkPipelineLayout pipelineLayout, PipelineCreateInfo& createInfo) {
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &createInfo.vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &createInfo.inputAssembly;
	pipelineInfo.pViewportState = &createInfo.viewportState;
	pipelineInfo.pRasterizationState = &createInfo.rasterizer;
	pipelineInfo.pMultisampleState = &createInfo.multisampling;
	pipelineInfo.pColorBlendState = &createInfo.colorBlending;
	pipelineInfo.pDepthStencilState = &createInfo.depthStencil;
	pipelineInfo.pDynamicState = &createInfo.dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(device.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	for (size_t i = 0; i < shaderModules.size(); i++) {
		vkDestroyShaderModule(device.getDevice(), shaderModules[i], nullptr);
	}
}


