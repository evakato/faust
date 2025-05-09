#pragma once

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "filebrowser/imfilebrowser.h"

#include "context.hpp"

struct UserParams {
	int waveletIterations = 5;
	bool temporalOnly = false;
	bool useSVGF = true;
};

struct FaustGui {
public:
	FaustGui(Context& context);
	~FaustGui();
	void startFrame();
	void render(VkCommandBuffer commandBuffer);
	void initGui(GLFWwindow* window, VkRenderPass renderPass);

	UserParams userParams;

private:
	void viewFileBrowser();
	void mainWindow();

	Context& context;
	VkDescriptorPool imguiPool;
	ImGui::FileBrowser fileDialog;
	ImGuiIO* io;
};
