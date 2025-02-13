#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "device.hpp"
#include "window.hpp"

class FaustGui {
public:
	FaustGui(FaustWindow& window, FaustDevice& device);
	~FaustGui();
	void startFrame();
	void render(VkCommandBuffer commandBuffer);
	void initGui(VkRenderPass renderPass);

private:

	VkDescriptorPool imguiPool;
	FaustDevice& device;
	FaustWindow& window;
};