#include "raster_pipeline.hpp"

void RasterPipeline::create(Context& context, std::array<vk::ImageView, 2> attachments, Buffer& cameraBuffer) {
	vk::Device device = *context.device;
	// render pass
	vk::AttachmentDescription colorAttachment = {};
	colorAttachment.format = vk::Format::eR16G16B16A16Sfloat;  // Normal texture format
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	// Depth attachment
	vk::AttachmentDescription depthAttachment = {};
	depthAttachment.format = vk::Format::eD32Sfloat;  // Depth format
	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	// References for attachments
	vk::AttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	// Subpass description
	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;  // Use depth attachment

	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo.attachmentCount = 2;  // Now we have 2 attachments (color and depth)
	renderPassInfo.pAttachments = new vk::AttachmentDescription[2]{ colorAttachment, depthAttachment };
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPass = device.createRenderPass(renderPassInfo);

	// framebuffer
	vk::FramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = 2;
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = faust::WIDTH;
	framebufferInfo.height = faust::HEIGHT;
	framebufferInfo.layers = 1;
	framebuffer = device.createFramebuffer(framebufferInfo);

	const std::vector<char> vertCode = faust::readFile("C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\shaders\\normal.vert.spv");
	const std::vector<char> fragCode = faust::readFile("C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\shaders\\normal.frag.spv");
	// Correct shader module creation
	vk::ShaderModule vertShaderModule = device.createShaderModule({ {}, vertCode.size(), reinterpret_cast<const uint32_t*>(vertCode.data()) });
	vk::ShaderModule fragShaderModule = device.createShaderModule({ {}, fragCode.size(), reinterpret_cast<const uint32_t*>(fragCode.data()) });

	// Set the modules in the pipeline stages
	vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";


	std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStagesRasterization = { vertShaderStageInfo, fragShaderStageInfo };

	// Vertex input (positions and normals)
	auto vertexBindingDescriptions = std::vector<vk::VertexInputBindingDescription>{ VertexRasterization::getBindingDescription() };
	auto vertexAttributeDescriptions = VertexRasterization::getAttributeDescriptions();
	auto vertexInputInfo = vk::PipelineVertexInputStateCreateInfo{}
		.setVertexBindingDescriptions(vertexBindingDescriptions)
		.setVertexAttributeDescriptions(vertexAttributeDescriptions);

	// Input assembly
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;

	// Viewport + Scissor
	vk::Viewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)faust::WIDTH;
	viewport.height = (float)faust::HEIGHT;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor = { {0, 0}, {faust::WIDTH, faust::HEIGHT} };
	vk::PipelineViewportStateCreateInfo viewportState = {};
	viewportState.setViewports(viewport).setScissors(scissor);

	// Rasterizer
	vk::PipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.polygonMode = vk::PolygonMode::eFill;
	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
	rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
	rasterizer.lineWidth = 1.0f;

	// Multisampling (no MSAA)
	vk::PipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

	// Color blending
	vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
		vk::ColorComponentFlagBits::eR |
		vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB |
		vk::ColorComponentFlagBits::eA;

	vk::PipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	std::vector<vk::DescriptorSetLayoutBinding> bindings = {
		//{ 0, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eFragment },
		{ 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment }
	};
	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.setBindings(bindings);
	vk::UniqueDescriptorSetLayout descriptorSetLayout = device.createDescriptorSetLayoutUnique(descriptorSetLayoutInfo);

	// Pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.setSetLayouts(*descriptorSetLayout);
	pipelineLayout = context.device->createPipelineLayoutUnique(pipelineLayoutInfo);

	std::array<vk::DynamicState, 1> dynamicStates = {
		vk::DynamicState::eLineWidth
	};

	vk::PipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	vk::PipelineDepthStencilStateCreateInfo depthStencilState = {};
	depthStencilState.setDepthTestEnable(true);
	depthStencilState.setDepthWriteEnable(true);
	depthStencilState.setDepthCompareOp(vk::CompareOp::eLess);
	depthStencilState.setStencilTestEnable(false);

	// Pipeline creation
	vk::GraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStagesRasterization.size());
	pipelineInfo.pStages = shaderStagesRasterization.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDepthStencilState = &depthStencilState;
	pipelineInfo.layout = *pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	pipeline = device.createGraphicsPipeline({}, pipelineInfo).value;

	descriptorSet = context.allocateDescriptorSet(*descriptorSetLayout);
	std::vector<vk::WriteDescriptorSet> writes(bindings.size());
	for (int i = 0; i < bindings.size(); i++) {
		writes[i].setDstSet(*descriptorSet);
		writes[i].setDescriptorType(bindings[i].descriptorType);
		writes[i].setDescriptorCount(bindings[i].descriptorCount);
		writes[i].setDstBinding(bindings[i].binding);
	}
	//normalImage.setDescImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
	//writes[0].setImageInfo(normalImage.descImageInfo);
	writes[0].setBufferInfo(cameraBuffer.descBufferInfo);
	context.device->updateDescriptorSets(writes, nullptr);

}

