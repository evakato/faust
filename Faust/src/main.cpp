#include <string>
#include <set>
#include <fstream>
#include <iostream>
#include <functional>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include "model.hpp"
#include "io_utils.hpp"
#include "camera.hpp"
#include "context.hpp"
#include "buffer.hpp"
#include "constants.hpp"
#include "image.hpp"
#include "accel.hpp"
#include "raster_pipeline.hpp"
#include "compute_pipeline.hpp"
#include "fullscreen_pipeline.hpp"

int main() {
	Context context;

	Camera camera{ glm::vec3(0.f, -1.f, 5.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.0f, 1.0f, 0.0f) };
	camera.setPerspectiveProjection(glm::radians(30.0f), (float)faust::WIDTH / (float)faust::HEIGHT, 0.1f, 100.0f);
	CameraData cameraData{ camera.getView(), camera.getProjection(), camera.getView(), camera.getProjection() };
	Buffer cameraBuffer{ context, Buffer::Type::Uniform, sizeof(CameraData), &cameraData };

	vk::SwapchainCreateInfoKHR swapchainInfo;
	swapchainInfo.setSurface(*context.surface);
	swapchainInfo.setMinImageCount(3);
	swapchainInfo.setImageFormat(vk::Format::eB8G8R8A8Unorm);
	swapchainInfo.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear);
	swapchainInfo.setImageExtent({ faust::WIDTH, faust::HEIGHT });
	swapchainInfo.setImageArrayLayers(1);
	swapchainInfo.setImageUsage(
		vk::ImageUsageFlagBits::eColorAttachment |
		vk::ImageUsageFlagBits::eTransferDst
	);
	swapchainInfo.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity);
	swapchainInfo.setPresentMode(vk::PresentModeKHR::eFifo);
	swapchainInfo.setClipped(true);
	swapchainInfo.setQueueFamilyIndices(context.queueFamilyIndex);
	vk::UniqueSwapchainKHR swapchain = context.device->createSwapchainKHRUnique(swapchainInfo);

	std::vector<vk::Image> swapchainImages = context.device->getSwapchainImagesKHR(*swapchain);

	std::vector<vk::UniqueImageView> swapchainImageViews;
	swapchainImageViews.reserve(swapchainImages.size());

	for (vk::Image image : swapchainImages) {
		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.image = image;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = vk::Format::eB8G8R8A8Unorm;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		swapchainImageViews.push_back(context.device->createImageViewUnique(viewInfo));
	}

	vk::CommandBufferAllocateInfo commandBufferInfo;
	commandBufferInfo.setCommandPool(*context.commandPool);
	commandBufferInfo.setCommandBufferCount(static_cast<uint32_t>(swapchainImages.size()));
	std::vector<vk::UniqueCommandBuffer> commandBuffers = context.device->allocateCommandBuffersUnique(commandBufferInfo);

	Image rtOutputImage = Image{ context, {faust::WIDTH, faust::HEIGHT}, vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled };

	// Load mesh
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Face> faces;
	faust::loadFromFile(vertices, indices, faces);
	faust::computeNormals(vertices, indices);

	Buffer vertexBuffer{ context, Buffer::Type::AccelInputVertex, sizeof(Vertex) * vertices.size(), vertices.data() };
	Buffer indexBuffer{ context, Buffer::Type::AccelInputIndex, sizeof(uint32_t) * indices.size(), indices.data() };
	Buffer faceBuffer{ context, Buffer::Type::AccelInput, sizeof(Face) * faces.size(), faces.data() };

	// Create bottom level accel struct
	vk::AccelerationStructureGeometryTrianglesDataKHR triangleData;
	triangleData.setVertexFormat(vk::Format::eR32G32B32Sfloat);
	triangleData.setVertexData(vertexBuffer.deviceAddress);
	triangleData.setVertexStride(sizeof(Vertex));
	triangleData.setMaxVertex(static_cast<uint32_t>(vertices.size()));
	triangleData.setIndexType(vk::IndexType::eUint32);
	triangleData.setIndexData(indexBuffer.deviceAddress);

	vk::AccelerationStructureGeometryKHR triangleGeometry;
	triangleGeometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
	triangleGeometry.setGeometry({ triangleData });
	triangleGeometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

	const auto primitiveCount = static_cast<uint32_t>(indices.size() / 3);

	Accel bottomAccel{ context, triangleGeometry, primitiveCount, vk::AccelerationStructureTypeKHR::eBottomLevel };

	// Create top level accel struct
	vk::TransformMatrixKHR transformMatrix = std::array{
		std::array{1.0f, 0.0f, 0.0f, 0.0f},
		std::array{0.0f, 1.0f, 0.0f, 0.0f},
		std::array{0.0f, 0.0f, 1.0f, 0.0f},
	};

	vk::AccelerationStructureInstanceKHR accelInstance;
	accelInstance.setTransform(transformMatrix);
	accelInstance.setMask(0xFF);
	accelInstance.setAccelerationStructureReference(bottomAccel.buffer.deviceAddress);
	accelInstance.setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable);

	Buffer instancesBuffer{ context, Buffer::Type::AccelInput, sizeof(vk::AccelerationStructureInstanceKHR), &accelInstance };

	vk::AccelerationStructureGeometryInstancesDataKHR instancesData;
	instancesData.setArrayOfPointers(false);
	instancesData.setData(instancesBuffer.deviceAddress);

	vk::AccelerationStructureGeometryKHR instanceGeometry;
	instanceGeometry.setGeometryType(vk::GeometryTypeKHR::eInstances);
	instanceGeometry.setGeometry({ instancesData });
	instanceGeometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

	Accel topAccel{ context, instanceGeometry, 1, vk::AccelerationStructureTypeKHR::eTopLevel };

	// Load shaders
	const std::vector<char> raygenCode = faust::readFile("C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\shaders\\raygen.rgen.spv"
	);
	const std::vector<char> missCode = faust::readFile("C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\shaders\\miss.rmiss.spv");
	const std::vector<char> chitCode = faust::readFile("C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\shaders\\closesthit.rchit.spv");

	std::vector<vk::UniqueShaderModule> shaderModules(3);
	shaderModules[0] = context.device->createShaderModuleUnique({ {}, raygenCode.size(), reinterpret_cast<const uint32_t*>(raygenCode.data()) });
	shaderModules[1] = context.device->createShaderModuleUnique({ {}, missCode.size(), reinterpret_cast<const uint32_t*>(missCode.data()) });
	shaderModules[2] = context.device->createShaderModuleUnique({ {}, chitCode.size(), reinterpret_cast<const uint32_t*>(chitCode.data()) });

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(3);
	shaderStages[0] = { {}, vk::ShaderStageFlagBits::eRaygenKHR, *shaderModules[0], "main" };
	shaderStages[1] = { {}, vk::ShaderStageFlagBits::eMissKHR, *shaderModules[1], "main" };
	shaderStages[2] = { {}, vk::ShaderStageFlagBits::eClosestHitKHR, *shaderModules[2], "main" };

	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups(3);
	shaderGroups[0] = { vk::RayTracingShaderGroupTypeKHR::eGeneral, 0, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR };
	shaderGroups[1] = { vk::RayTracingShaderGroupTypeKHR::eGeneral, 1, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR };
	shaderGroups[2] = { vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup, VK_SHADER_UNUSED_KHR, 2, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR };


	// create ray tracing pipeline
	std::vector<vk::DescriptorSetLayoutBinding> bindings{
		{0, vk::DescriptorType::eAccelerationStructureKHR, 1, vk::ShaderStageFlagBits::eRaygenKHR},  // Binding = 0 : TLAS
		{1, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eRaygenKHR},              // Binding = 1 : Storage image
		{2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},         // Binding = 2 : Vertices
		{3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},         // Binding = 3 : Indices
		{4, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},         // Binding = 4 : Faces
		{5, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR}
	};

	// Create desc set layout
	vk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
	descSetLayoutInfo.setBindings(bindings);
	vk::UniqueDescriptorSetLayout descSetLayout = context.device->createDescriptorSetLayoutUnique(descSetLayoutInfo);

	// Create pipeline layout
	vk::PushConstantRange pushRange;
	pushRange.setOffset(0);
	pushRange.setSize(sizeof(int));
	pushRange.setStageFlags(vk::ShaderStageFlagBits::eRaygenKHR);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setSetLayouts(*descSetLayout);
	pipelineLayoutInfo.setPushConstantRanges(pushRange);
	vk::UniquePipelineLayout pipelineLayout = context.device->createPipelineLayoutUnique(pipelineLayoutInfo);

	// Create pipeline
	vk::RayTracingPipelineCreateInfoKHR rtPipelineInfo;
	rtPipelineInfo.setStages(shaderStages);
	rtPipelineInfo.setGroups(shaderGroups);
	rtPipelineInfo.setMaxPipelineRayRecursionDepth(4);
	rtPipelineInfo.setLayout(*pipelineLayout);

	auto result = context.device->createRayTracingPipelineKHRUnique(nullptr, nullptr, rtPipelineInfo);
	if (result.result != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create ray tracing pipeline.");
	}

	vk::UniquePipeline pipeline = std::move(result.value);

	// Get ray tracing properties
	auto properties = context.physicalDevice.getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
	auto rtProperties = properties.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();

	// Calculate shader binding table (SBT) size
	uint32_t handleSize = rtProperties.shaderGroupHandleSize;
	uint32_t handleSizeAligned = rtProperties.shaderGroupHandleAlignment;
	uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
	uint32_t sbtSize = groupCount * handleSizeAligned;

	// Get shader group handles
	std::vector<uint8_t> handleStorage(sbtSize);
	if (context.device->getRayTracingShaderGroupHandlesKHR(*pipeline, 0, groupCount, sbtSize, handleStorage.data()) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to get ray tracing shader group handles.");
	}

	// Create SBT
	Buffer raygenSBT{ context, Buffer::Type::ShaderBindingTable, handleSize, handleStorage.data() + 0 * handleSizeAligned };
	Buffer missSBT{ context, Buffer::Type::ShaderBindingTable, handleSize, handleStorage.data() + 1 * handleSizeAligned };
	Buffer hitSBT{ context, Buffer::Type::ShaderBindingTable, handleSize, handleStorage.data() + 2 * handleSizeAligned };

	uint32_t stride = rtProperties.shaderGroupHandleAlignment;
	uint32_t size = rtProperties.shaderGroupHandleAlignment;

	vk::StridedDeviceAddressRegionKHR raygenRegion{ raygenSBT.deviceAddress, stride, size };
	vk::StridedDeviceAddressRegionKHR missRegion{ missSBT.deviceAddress, stride, size };
	vk::StridedDeviceAddressRegionKHR hitRegion{ hitSBT.deviceAddress, stride, size };

	// Create RAY TRACING desc set
	vk::UniqueDescriptorSet descSet = context.allocateDescriptorSet(*descSetLayout);
	std::vector<vk::WriteDescriptorSet> writes(bindings.size());
	for (int i = 0; i < bindings.size(); i++) {
		writes[i].setDstSet(*descSet);
		writes[i].setDescriptorType(bindings[i].descriptorType);
		writes[i].setDescriptorCount(bindings[i].descriptorCount);
		writes[i].setDstBinding(bindings[i].binding);
	}
	writes[0].setPNext(&topAccel.descAccelInfo);
	writes[1].setImageInfo(rtOutputImage.descImageInfo);
	writes[2].setBufferInfo(vertexBuffer.descBufferInfo);
	writes[3].setBufferInfo(indexBuffer.descBufferInfo);
	writes[4].setBufferInfo(faceBuffer.descBufferInfo);
	writes[5].setBufferInfo(cameraBuffer.descBufferInfo);
	context.device->updateDescriptorSets(writes, nullptr);

	// rasterization stuff
	std::array<Image, 2> normalImages = {
		Image{ context, { faust::WIDTH, faust::HEIGHT }, vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eColorAttachmentOptimal },
		Image{ context, { faust::WIDTH, faust::HEIGHT }, vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eColorAttachmentOptimal }
	};
	std::array<Image, 2> depthImages = {
	Image{ context, { faust::WIDTH, faust::HEIGHT }, vk::Format::eD32Sfloat,
		   vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eDepth, vk::ImageLayout::eDepthStencilAttachmentOptimal },
	Image{ context, { faust::WIDTH, faust::HEIGHT }, vk::Format::eD32Sfloat,
		   vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eDepth, vk::ImageLayout::eDepthStencilAttachmentOptimal }
	};
	normalImages[0].descImageInfo.setImageLayout(vk::ImageLayout::eGeneral);
	normalImages[1].descImageInfo.setImageLayout(vk::ImageLayout::eGeneral);
	depthImages[0].descImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	depthImages[1].descImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	Image gbufferMotionImage{ context, { faust::WIDTH, faust::HEIGHT }, vk::Format::eR16G16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eColorAttachmentOptimal };

	std::array<Image, 2> meshIdImages = {
		Image { context, { faust::WIDTH, faust::HEIGHT }, vk::Format::eR32Uint, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eColorAttachmentOptimal },
		Image { context, { faust::WIDTH, faust::HEIGHT }, vk::Format::eR32Uint, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eColorAttachmentOptimal }
	};

	depthImages[0].createSampler(*context.device);
	depthImages[1].createSampler(*context.device);

	std::array<Image, 2> filterOutputImages = { Image { context, {faust::WIDTH, faust::HEIGHT}, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,  vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eGeneral },
		Image{ context, {faust::WIDTH, faust::HEIGHT}, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,  vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eGeneral } };

	std::array<Image, 2> atrousFilterImages = { Image { context, {faust::WIDTH, faust::HEIGHT},vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,  vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eGeneral },
		Image{ context, {faust::WIDTH, faust::HEIGHT}, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,  vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eGeneral } };
	atrousFilterImages[1].createSampler(*context.device);

	std::array<Image, 2> momentImages = { Image { context, {faust::WIDTH, faust::HEIGHT},  vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,  vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eGeneral },
		Image{ context, {faust::WIDTH, faust::HEIGHT},vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,  vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eGeneral } };

	std::array<Image, 2> varianceImages = { Image{ context, {faust::WIDTH, faust::HEIGHT}, vk::Format::eR32Sfloat, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,  vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eGeneral }, Image{ context, {faust::WIDTH, faust::HEIGHT}, vk::Format::eR32Sfloat, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,  vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eGeneral } };

	Image finalOutputImage = Image{ context, {faust::WIDTH, faust::HEIGHT}, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,  vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eGeneral };

	std::vector<vk::ImageView> attachments = { *normalImages[0].view, *gbufferMotionImage.view, *meshIdImages[0].view, *depthImages[0].view };

	RasterPipeline gbufferPipeline;
	gbufferPipeline.create(context, attachments, cameraBuffer);
	gbufferPipeline.updateDescriptorSet(*context.device, cameraBuffer, gbufferMotionImage);
	vk::RenderPass gbufferRenderPass = gbufferPipeline.renderPass;
	vk::Framebuffer gbufferFramebuffer = gbufferPipeline.framebuffer;

	FullscreenPresentPipeline presentPipeline;
	presentPipeline.create(context, swapchainImageViews);
	presentPipeline.updateDescriptorSet(*context.device, atrousFilterImages[1]);
	vk::RenderPass presentRenderPass = gbufferPipeline.renderPass;
	vk::Framebuffer presentFramebuffer = gbufferPipeline.framebuffer;

	const std::string temporalShaderFilename = "C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\shaders\\filter.comp.spv";

	const std::string spatialShaderFilename = "C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\shaders\\atrous.comp.spv";
	ComputePipeline temporalPipeline{ context, temporalShaderFilename, temporalAccumulationBindings };

	ComputePipeline spatialPipeline{ context, spatialShaderFilename, spatialFilterBindings, 2 };

	// Main loop
	uint32_t imageIndex = 0;
	int frame = 0;
	vk::UniqueSemaphore imageAcquiredSemaphore = context.device->createSemaphoreUnique(vk::SemaphoreCreateInfo());
	float factor = 1.0f;
	int totalFrames = 0;

	while (!glfwWindowShouldClose(context.window)) {
		glfwPollEvents();

		Image& currentNormalWrite = (totalFrames % 2 == 0) ? normalImages[0] : normalImages[1];
		Image& currentDepthWrite = (totalFrames % 2 == 0) ? depthImages[0] : depthImages[1];
		Image& currentMeshIdWrite = (totalFrames % 2 == 0) ? meshIdImages[0] : meshIdImages[1];
		Image& prevNormalImage = (totalFrames % 2 == 0) ? normalImages[1] : normalImages[0];
		Image& prevDepthImage = (totalFrames % 2 == 0) ? depthImages[1] : depthImages[0];
		Image& prevMeshIdImage = (totalFrames % 2 == 0) ? meshIdImages[1] : meshIdImages[0];

		Image& currentFilteredImage = (totalFrames % 2 == 0) ? filterOutputImages[0] : filterOutputImages[1];
		Image& prevFilteredImage = (totalFrames % 2 == 0) ? filterOutputImages[1] : filterOutputImages[0];
		currentFilteredImage.updateDescriptor();
		prevFilteredImage.updateDescriptor();

		Image& currMoment = (totalFrames % 2 == 0) ? momentImages[0] : momentImages[1];
		Image& prevMoment = (totalFrames % 2 == 0) ? momentImages[1] : momentImages[0];

		// TODO: clean this up
		std::vector<vk::DescriptorImageInfo> spatialFilterInputs = {
			currentNormalWrite.descImageInfo,
			currentDepthWrite.descImageInfo,
			atrousFilterImages[0].descImageInfo,
			varianceImages[0].descImageInfo,
			varianceImages[1].descImageInfo,
			atrousFilterImages[1].descImageInfo
		};
		spatialPipeline.updateDescriptorSet(*context.device, spatialFilterInputs, 0);
		std::vector<vk::DescriptorImageInfo> spatialFilterInputs2 = {
			currentNormalWrite.descImageInfo,
			currentDepthWrite.descImageInfo,
			atrousFilterImages[1].descImageInfo,
			varianceImages[1].descImageInfo,
			varianceImages[0].descImageInfo,
			atrousFilterImages[0].descImageInfo
		};
		spatialPipeline.updateDescriptorSet(*context.device, spatialFilterInputs2, 1);

		std::vector<vk::ImageView> attachments = {
			*currentNormalWrite.view,
			*gbufferMotionImage.view,
			*currentMeshIdWrite.view,
			*currentDepthWrite.view };
		gbufferPipeline.createFramebuffer(*context.device, attachments);

		//camera.rotate(-0.1f, 0.0f);
		if (totalFrames % 20 == 0) factor *= -1.f;
		CameraData cameraubo{};
		cameraubo.prevProj = camera.getProjection();
		cameraubo.prevView = camera.getView();

		if (totalFrames % 5 == 0) {
			camera.translate(glm::vec3{ 0.f, factor * 0.1f, 0.f });
			frame = 0;
		}

		cameraubo.proj = camera.getProjection();
		cameraubo.view = camera.getView();
		memcpy(cameraBuffer.mapped, &cameraubo, sizeof(CameraData));

		// Acquire next image
		imageIndex = context.device->acquireNextImageKHR(*swapchain, UINT64_MAX, *imageAcquiredSemaphore).value;

		// Record commands
		vk::CommandBuffer commandBuffer = *commandBuffers[imageIndex];
		commandBuffer.begin(vk::CommandBufferBeginInfo());

		// rasterization
		vk::RenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.renderPass = gbufferPipeline.renderPass;
		renderPassBeginInfo.framebuffer = gbufferPipeline.framebuffer;
		renderPassBeginInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
		renderPassBeginInfo.renderArea.extent = vk::Extent2D{ faust::WIDTH, faust::HEIGHT };
		renderPassBeginInfo.clearValueCount = (uint32_t)RasterPipeline::clearValues.size();
		renderPassBeginInfo.pClearValues = RasterPipeline::clearValues.data();

		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, gbufferPipeline.pipeline);
		vk::DeviceSize offsets[] = { 0 };
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *(gbufferPipeline.pipelineLayout), 0, *(gbufferPipeline.descriptorSet), nullptr);
		commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer.buffer.get(), offsets);
		commandBuffer.bindIndexBuffer(indexBuffer.buffer.get(), 0, vk::IndexType::eUint32);
		commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);
		commandBuffer.endRenderPass();

		// ray tracing
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, *pipeline);
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, *pipelineLayout, 0, *descSet, nullptr);
		commandBuffer.pushConstants(*pipelineLayout, vk::ShaderStageFlagBits::eRaygenKHR, 0, sizeof(int), &frame);
		commandBuffer.traceRaysKHR(raygenRegion, missRegion, hitRegion, {}, faust::WIDTH, faust::HEIGHT, 1);

		if (totalFrames == 0) {
			prevNormalImage.transitionImageLayout(commandBuffer, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eGeneral, 1, 1);
			prevMeshIdImage.transitionImageLayout(commandBuffer, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eGeneral, 1, 1);
			prevDepthImage.transitionImageLayout(commandBuffer, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 1, 1);
		}

		currentMeshIdWrite.transitionImageLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral, 1, 1);
		currentDepthWrite.transitionImageLayout(commandBuffer, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 1, 1);

		prevNormalImage.setDescImageLayout(vk::ImageLayout::eGeneral);
		prevMeshIdImage.setDescImageLayout(vk::ImageLayout::eGeneral);
		prevDepthImage.setDescImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		gbufferMotionImage.transitionImageLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral, 1, 1);

		// -- Temporal filtering and variance compute shader --
		std::vector<vk::DescriptorImageInfo> filterInputs = {
			currentNormalWrite.descImageInfo,
			currentMeshIdWrite.descImageInfo,
			currentDepthWrite.descImageInfo,
			prevNormalImage.descImageInfo,
			prevMeshIdImage.descImageInfo,
			prevDepthImage.descImageInfo,
			gbufferMotionImage.descImageInfo,
			rtOutputImage.descImageInfo,
			prevFilteredImage.descImageInfo,
			currentFilteredImage.descImageInfo,
			prevMoment.descImageInfo,
			currMoment.descImageInfo,
			varianceImages[0].descImageInfo
		};
		temporalPipeline.updateDescriptorSet(*context.device, filterInputs);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, temporalPipeline.pipeline);
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, temporalPipeline.layout, 0, *(temporalPipeline.descriptorSets[0]), {});
		commandBuffer.pushConstants(temporalPipeline.layout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(int), &totalFrames);
		commandBuffer.dispatch((faust::WIDTH + 7) / 8, (faust::HEIGHT + 7) / 8, 1);

		vk::Image srcTemporalImage = *currentFilteredImage.image;
		Image::setImageLayout(commandBuffer, srcTemporalImage, vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal);
		Image::setImageLayout(commandBuffer, *atrousFilterImages[0].image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

		Image::copyImage(commandBuffer, srcTemporalImage, *atrousFilterImages[0].image);

		Image::setImageLayout(commandBuffer, srcTemporalImage, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eGeneral);
		Image::setImageLayout(commandBuffer, *atrousFilterImages[0].image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eGeneral);

		// -- WAVELET TRANSFORM --
		for (int iteration = 1; iteration <= 5; iteration++) {
			int descSetIndex = (iteration % 2 == 1) ? 0 : 1;
			int stepWidth = 1 << (iteration - 1);
			commandBuffer.pushConstants(spatialPipeline.layout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(int), &stepWidth);
			commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, spatialPipeline.pipeline);
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, spatialPipeline.layout, 0, *(spatialPipeline.descriptorSets[descSetIndex]), {});
			commandBuffer.dispatch((faust::WIDTH + 7) / 8, (faust::HEIGHT + 7) / 8, 1);
			if (iteration == 1) {
				vk::Image colorHistSrc = *atrousFilterImages[0].image;
				vk::Image colorHistDest = *currentFilteredImage.image;
				Image::setImageLayout(commandBuffer, colorHistSrc, vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal);
				Image::setImageLayout(commandBuffer, colorHistDest, vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferDstOptimal);
				Image::copyImage(commandBuffer, colorHistSrc, colorHistDest);
				Image::setImageLayout(commandBuffer, colorHistSrc, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eGeneral);
				Image::setImageLayout(commandBuffer, colorHistDest, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eGeneral);

			}
		}

		vk::RenderPassBeginInfo presentRenderPassBeginInfo;
		presentRenderPassBeginInfo.renderPass = presentPipeline.renderPass;
		presentRenderPassBeginInfo.framebuffer = *presentPipeline.framebuffers[imageIndex];
		presentRenderPassBeginInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
		presentRenderPassBeginInfo.renderArea.extent = vk::Extent2D{ faust::WIDTH, faust::HEIGHT };
		presentRenderPassBeginInfo.clearValueCount = (uint32_t)FullscreenPresentPipeline::clearValues.size();
		presentRenderPassBeginInfo.pClearValues = FullscreenPresentPipeline::clearValues.data();

		commandBuffer.beginRenderPass(presentRenderPassBeginInfo, vk::SubpassContents::eInline);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, presentPipeline.pipeline);
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, presentPipeline.pipelineLayout, 0, *(presentPipeline.descriptorSet), nullptr);
		commandBuffer.draw(3, 1, 0, 0);
		commandBuffer.endRenderPass();

		// Prepare filterOutputImage for copy to swapchain
		vk::Image swapchainImage = swapchainImages[imageIndex];
		Image::setImageLayout(commandBuffer, swapchainImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		Image::setImageLayout(commandBuffer, swapchainImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);

		commandBuffer.end();

		// Submit
		context.queue.submit(vk::SubmitInfo().setCommandBuffers(commandBuffer));

		// Present image
		vk::PresentInfoKHR presentInfo;
		presentInfo.setSwapchains(*swapchain);
		presentInfo.setImageIndices(imageIndex);
		presentInfo.setWaitSemaphores(*imageAcquiredSemaphore);
		auto result = context.queue.presentKHR(presentInfo);
		if (result != vk::Result::eSuccess) {
			throw std::runtime_error("failed to present.");
		}
		context.queue.waitIdle();
		frame++;

		totalFrames++;
	}

	context.device->waitIdle();
	glfwDestroyWindow(context.window);
	glfwTerminate();
}