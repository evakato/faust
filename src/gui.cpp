#include "gui.hpp"

static void check_vk_result(VkResult err)
{
	if (err == VK_SUCCESS)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

FaustGui::FaustGui(FaustDevice& device) : device{ device } {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO();
	//io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
}

FaustGui::~FaustGui() {
	vkDestroyDescriptorPool(device.getDevice(), imguiPool, nullptr);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void FaustGui::initGui(GLFWwindow* window, VkRenderPass renderPass) {
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE },
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 0;
	for (VkDescriptorPoolSize& pool_size : pool_sizes)
		pool_info.maxSets += pool_size.descriptorCount;
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	auto err = vkCreateDescriptorPool(device.getDevice(), &pool_info, nullptr, &imguiPool);
	check_vk_result(err);

	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = device.getInstance();
	init_info.PhysicalDevice = device.getPhysicalDevice();
	init_info.Device = device.getDevice();
	init_info.QueueFamily = device.getGraphicsFamily();
	init_info.Queue = device.getGraphicsQueue();
	init_info.PipelineCache = nullptr;
	init_info.DescriptorPool = imguiPool;
	init_info.RenderPass = renderPass;
	init_info.Subpass = 0;
	init_info.MinImageCount = 2;
	init_info.ImageCount = 2;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info);

	fileDialog.SetTitle("Open model");
	fileDialog.SetTypeFilters({ ".obj" });
	fileDialog.SetDirectory("./assets/models");
}

void FaustGui::startFrame() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	mainWindow();
	ImGui::Render();
}

void FaustGui::render(VkCommandBuffer commandBuffer) {
	ImDrawData* draw_data = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
}

void FaustGui::viewFileBrowser() {
	if (ImGui::Button("open file dialog"))
		fileDialog.Open();

	fileDialog.Display();

	if (fileDialog.HasSelected())
	{
		FaustState& state = FaustState::getInstance();
		state.modelPath = fileDialog.GetSelected().string();
		state.modelChanged = true;
		fileDialog.ClearSelected();
	}
}

void FaustGui::mainWindow() {
	FaustState& state = FaustState::getInstance();

	ImGui::Begin("Faust");
	ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Camera")) {
		ImGui::Text("Position: (%.2f, %.2f, %.2f)", state.cameraPos.x, state.cameraPos.y, state.cameraPos.z);
		ImGui::Text("View direction: (%.2f, %.2f, %.2f)", state.cameraViewDir.x, state.cameraViewDir.y, state.cameraViewDir.z);
		ImGui::Text("Up direction: (%.2f, %.2f, %.2f)", state.cameraUpDir.x, state.cameraUpDir.y, state.cameraUpDir.z);
	}

	if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Position: (%.2f, %.2f, %.2f)", state.pointLightPos.x, state.pointLightPos.y, state.pointLightPos.z);
		ImGui::ColorEdit4("Pick a color", (float*)&state.pointLightCol);
	}

	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Filepath: %s", state.modelPath.c_str());
		ImGui::Text("Number of tris: %d", state.numTris);
		viewFileBrowser();
	}

	int currentIndex = static_cast<int>(state.shadingSetting);

	if (ImGui::Combo("Shading Mode", &currentIndex, shadingOptions, IM_ARRAYSIZE(shadingOptions))) {
		state.shadingSetting = static_cast<ShadingSettings>(currentIndex);
	}

	ImGui::End();
}
