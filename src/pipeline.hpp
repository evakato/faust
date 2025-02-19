#pragma once 

#include <vulkan/vulkan.h>
#include <array>

#include "device.hpp"
#include "math.hpp"
#include "model.hpp"

struct PipelineParams {
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	bool enableAlphaBlending = false;
};

class FaustPipeline {
public:
	FaustPipeline(FaustDevice& device, const std::string& vertShader, const std::string& fragShader);
	~FaustPipeline();

	void createGraphicsPipeline(PipelineParams& params);
	VkPipeline getPipeline() const { return graphicsPipeline; }
	void bind(VkCommandBuffer commandBuffer);

private:
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void enableAlphaBlending(VkPipelineColorBlendAttachmentState& colorBlendAttachment);

	FaustDevice& device;

	VkPipeline graphicsPipeline;
	VkPipelineBindPoint bindPoint;
	const std::string vertShader;
	const std::string fragShader;
};