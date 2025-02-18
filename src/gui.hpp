#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "device.hpp"
#include "state.hpp"

class FaustGui {
public:
	FaustGui(FaustDevice& device);
	~FaustGui();
	void startFrame();
	void render(VkCommandBuffer commandBuffer);
	void initGui(GLFWwindow* window, VkRenderPass renderPass);

private:
	FaustDevice& device;
	VkDescriptorPool imguiPool;
};
