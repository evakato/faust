#pragma once

#include <algorithm>
#include <array>

#include "device.hpp"
#include "gui.hpp"
#include "image.hpp"
#include "window.hpp"

const int MAX_FRAMES_IN_FLIGHT = 2;

struct DrawFrameParams {
	uint32_t currentFrame;
	VkPipelineBindPoint bindPoint;
	VkPipeline pipeline;
	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;
	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	int indexCount;
};

class FaustRenderer {
public:
	FaustRenderer(FaustDevice& device, FaustWindow& window);
	~FaustRenderer();
	VkRenderPass getRenderPass() const { return renderPass; }
	VkExtent2D getExtent() const { return swapChainExtent; }
	void recreateSwapChain();
	bool beginFrame(uint32_t currentFrame);
	void drawFrame(DrawFrameParams& params);

private:
	void createSwapChain();
	void createImageViews();
	void cleanupSwapChain();
	void createRenderPass();
	void createFramebuffers();
	void createCommandBuffers();
	void createSyncObjects();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, DrawFrameParams& params);
	void createDepthResources();

	FaustDevice& device;
	FaustWindow& window;
	FaustGui gui{ device };

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkRenderPass renderPass;

	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	uint32_t imageIndex;
	VkResult result;

	Image depthImage{ device };
};