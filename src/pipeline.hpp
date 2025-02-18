#pragma once 

#include <vulkan/vulkan.h>
#include <array>

#include "device.hpp"
#include "math.hpp"
#include "model.hpp"

class FaustPipeline {
public:
	FaustPipeline(FaustDevice& device);
	~FaustPipeline();

	void createGraphicsPipeline(VkRenderPass renderPass, VkPipelineLayout pipelineLayout);
	VkPipeline getPipeline() const { return graphicsPipeline; }

private:
	VkShaderModule createShaderModule(const std::vector<char>& code);

	VkPipeline graphicsPipeline;
	FaustDevice& device;
};