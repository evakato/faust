#include "gui.hpp"

static void check_vk_result(VkResult err)
{
	if (err == VK_SUCCESS)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

FaustGui::FaustGui(Context& context) : context{ context } {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO();
	//io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
}

FaustGui::~FaustGui() {
	vkDestroyDescriptorPool(*context.device, imguiPool, nullptr);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void FaustGui::initGui(GLFWwindow* window, VkRenderPass renderPass) {
	VkDescriptorPoolSize pool_sizes[] = {
	{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
	{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
	{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
	{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
	{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
	{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
	{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
	{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
	{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
	{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
	{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes); // large enough
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	VkResult err = vkCreateDescriptorPool(*context.device, &pool_info, nullptr, &imguiPool);

	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = *context.instance;
	init_info.PhysicalDevice = context.physicalDevice;
	init_info.Device = *context.device;
	init_info.QueueFamily = context.queueFamilyIndex;
	init_info.Queue = context.queue;
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

}

void FaustGui::mainWindow() {

	ImGui::SetNextWindowSize(ImVec2(350, 150));
	ImGui::Begin("Faust");
	ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
	ImGui::Spacing();

	if (ImGui::CollapsingHeader("SVGF options", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Use SVGF", &userParams.useSVGF);
		if (userParams.useSVGF) {
			ImGui::Checkbox("Temporal Only", &userParams.temporalOnly);
			if (!userParams.temporalOnly) {
				ImGui::PushItemWidth(200);
				ImGui::SliderInt("Wavelet iterations", &userParams.waveletIterations, 1, 5);
				ImGui::PopItemWidth();
			}
		}

	}

	ImGui::End();
}
