#include "raster_pipeline.hpp"

void RasterPipeline::createFramebuffer(vk::Device device, std::vector<vk::ImageView> attachments) {
	vk::FramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = faust::WIDTH;
	framebufferInfo.height = faust::HEIGHT;
	framebufferInfo.layers = 1;
	framebuffer = device.createFramebuffer(framebufferInfo);
}

void RasterPipeline::create(Context& context, std::vector<vk::ImageView> attachments, Buffer& cameraBuffer) {
	vk::Device device = *context.device;
	// render pass
	vk::AttachmentDescription colorAttachment = {};
	colorAttachment.format = vk::Format::eR16G16B16A16Sfloat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::eGeneral;

	vk::AttachmentDescription motionAttachment = {};
	motionAttachment.format = vk::Format::eR16G16Sfloat;
	motionAttachment.samples = vk::SampleCountFlagBits::e1;
	motionAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	motionAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	motionAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	motionAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	motionAttachment.initialLayout = vk::ImageLayout::eUndefined;
	motionAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	vk::AttachmentDescription meshIdAttachment = {};
	meshIdAttachment.format = vk::Format::eR32Uint;
	meshIdAttachment.samples = vk::SampleCountFlagBits::e1;
	meshIdAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	meshIdAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	meshIdAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	meshIdAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	meshIdAttachment.initialLayout = vk::ImageLayout::eUndefined;
	meshIdAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	vk::AttachmentDescription depthAttachment = {};
	depthAttachment.format = vk::Format::eD32Sfloat;
	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	std::vector<vk::AttachmentDescription> attachmentDescriptions = { colorAttachment, motionAttachment, meshIdAttachment, depthAttachment };

	// References for attachments
	vk::AttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference motionVecRef = {};
	motionVecRef.attachment = 1;
	motionVecRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference meshIdAttachmentRef = {};
	meshIdAttachmentRef.attachment = 2;
	meshIdAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 3;
	depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	std::vector<vk::AttachmentReference> colorAttachments = {
		colorAttachmentRef,
		motionVecRef,
		meshIdAttachmentRef
	};

	// Subpass description
	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
	subpass.pColorAttachments = colorAttachments.data();
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo.attachmentCount = attachmentDescriptions.size();
	renderPassInfo.pAttachments = attachmentDescriptions.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPass = device.createRenderPass(renderPassInfo);

	// framebuffer
	createFramebuffer(*context.device, attachments);

	const std::vector<char> vertCode = faust::readFile("C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\shaders\\gbuffer.vert.spv");
	const std::vector<char> fragCode = faust::readFile("C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\shaders\\gbuffer.frag.spv");
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
	auto vertexBindingDescriptions = std::vector<vk::VertexInputBindingDescription>{ Vertex::getBindingDescription() };
	auto vertexAttributeDescriptions = Vertex::getAttributeDescriptions();
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
	vk::PipelineColorBlendAttachmentState blendAttachmentState{};
	blendAttachmentState.blendEnable = VK_FALSE;
	blendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR |
		vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB |
		vk::ColorComponentFlagBits::eA;
	std::array<vk::PipelineColorBlendAttachmentState, 3> blendAttachments = { blendAttachmentState, blendAttachmentState, blendAttachmentState };

	vk::PipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.attachmentCount = static_cast<uint32_t>(blendAttachments.size());
	colorBlending.pAttachments = blendAttachments.data();

	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.setBindings(bindings);
	descriptorSetLayout = device.createDescriptorSetLayoutUnique(descriptorSetLayoutInfo);

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

	//updateDescriptorSet(*context.device, cameraBuffer);
}

void RasterPipeline::updateDescriptorSet(vk::Device device, const Buffer& buffer, const Image& image) {
	std::vector<vk::WriteDescriptorSet> writes(bindings.size());
	for (int i = 0; i < bindings.size(); i++) {
		writes[i].setDstSet(*descriptorSet);
		writes[i].setDescriptorType(bindings[i].descriptorType);
		writes[i].setDescriptorCount(bindings[i].descriptorCount);
		writes[i].setDstBinding(bindings[i].binding);
	}
	writes[0].setBufferInfo(buffer.descBufferInfo);
	//writes[1].setImageInfo(image.descImageInfo);
	device.updateDescriptorSets(writes, nullptr);
}

