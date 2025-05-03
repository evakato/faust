#include "context.hpp"

namespace faust {
	vk::PhysicalDevice pickSuitablePhysicalDevice(
		const vk::Instance& instance,
		const std::vector<const char*>& requiredExtensions)
	{
		auto physicalDevices = instance.enumeratePhysicalDevices();

		for (const auto& device : physicalDevices) {
			// Get supported extensions for this device
			auto availableExtensions = device.enumerateDeviceExtensionProperties();
			std::set<std::string> available;

			for (const auto& ext : availableExtensions) {
				available.insert(ext.extensionName);
			}

			bool allSupported = true;
			for (const char* required : requiredExtensions) {
				if (!available.contains(required)) {
					allSupported = false;
					break;
				}
			}

			if (allSupported) {
				vk::PhysicalDeviceProperties props = device.getProperties();
				std::cout << "Selected device: " << props.deviceName << std::endl;
				return device;
			}
		}

		throw std::runtime_error("No suitable physical device found with required extensions.");
	}
}

Context::Context() {
	// Create window
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(faust::WIDTH, faust::HEIGHT, "Vulkan Pathtracing", nullptr, nullptr);

	// Prepare extensions and layers
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	std::vector<const char*> layers{ "VK_LAYER_KHRONOS_validation" };

	auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

	// Create instance
	vk::ApplicationInfo appInfo;
	appInfo.setPApplicationName("My Vulkan App");
	appInfo.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
	appInfo.setPEngineName("No Engine");
	appInfo.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
	appInfo.setApiVersion(VK_API_VERSION_1_3);

	vk::InstanceCreateInfo instanceInfo;
	instanceInfo.setPApplicationInfo(&appInfo);
	instanceInfo.setPEnabledLayerNames(layers);
	instanceInfo.setPEnabledExtensionNames(extensions);
	instance = vk::createInstanceUnique(instanceInfo);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);

	// Pick first gpu
	std::vector<const char*> requiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
		VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
		VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		VK_KHR_SPIRV_1_4_EXTENSION_NAME,
		VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
	};
	physicalDevice = faust::pickSuitablePhysicalDevice(*instance, requiredExtensions);

	// Create debug messenger
	vk::DebugUtilsMessengerCreateInfoEXT messengerInfo;
	messengerInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
	messengerInfo.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
	messengerInfo.setPfnUserCallback(&debugUtilsMessengerCallback);
	messenger = instance->createDebugUtilsMessengerEXTUnique(messengerInfo);

	// Create surface
	VkSurfaceKHR _surface;
	VkResult res = glfwCreateWindowSurface(VkInstance(*instance), window, nullptr, &_surface);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
	surface = vk::UniqueSurfaceKHR(vk::SurfaceKHR(_surface), { *instance });

	// Find queue family
	std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
	for (int i = 0; i < queueFamilies.size(); i++) {
		auto supportGraphics = queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics;
		auto supportCompute = queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute;
		auto supportPresent = physicalDevice.getSurfaceSupportKHR(i, *surface);
		if (supportCompute && supportPresent && supportGraphics) {
			queueFamilyIndex = i;
		}
	}

	// Create device
	const float queuePriority = 1.0f;
	vk::DeviceQueueCreateInfo queueCreateInfo;
	queueCreateInfo.setQueueFamilyIndex(queueFamilyIndex);
	queueCreateInfo.setQueuePriorities(queuePriority);

	const std::vector<const char*> deviceExtensions{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
		VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
		VK_KHR_MAINTENANCE3_EXTENSION_NAME,
		VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
		VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	};

	// Check device extension support
	if (!checkDeviceExtensionSupport(deviceExtensions)) {
		throw std::runtime_error("Some required extensions are not supported");
	}

	// Enable required features for ray tracing
	vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures{};
	bufferDeviceAddressFeatures.setBufferDeviceAddress(true);

	vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
	rayTracingPipelineFeatures.setRayTracingPipeline(true);

	vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
	accelerationStructureFeatures.setAccelerationStructure(true);

	// Physical device feature chain
	vk::PhysicalDeviceFeatures2 features2;
	features2.setPNext(&bufferDeviceAddressFeatures);
	bufferDeviceAddressFeatures.setPNext(&rayTracingPipelineFeatures);
	rayTracingPipelineFeatures.setPNext(&accelerationStructureFeatures);

	// Create device info with features enabled
	vk::DeviceCreateInfo deviceInfo;
	deviceInfo.setQueueCreateInfos(queueCreateInfo);
	deviceInfo.setPEnabledExtensionNames(deviceExtensions);
	deviceInfo.setPNext(&features2);

	device = physicalDevice.createDeviceUnique(deviceInfo);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);

	queue = device->getQueue(queueFamilyIndex, 0);

	// Create command pool
	vk::CommandPoolCreateInfo commandPoolInfo;
	commandPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
	commandPoolInfo.setQueueFamilyIndex(queueFamilyIndex);
	commandPool = device->createCommandPoolUnique(commandPoolInfo);

	// Create descriptor pool
	std::vector<vk::DescriptorPoolSize> poolSizes{
		{vk::DescriptorType::eAccelerationStructureKHR, 1},
		{vk::DescriptorType::eStorageImage, 5},
		{vk::DescriptorType::eStorageBuffer, 3},
		{vk::DescriptorType::eUniformBuffer, 2},
	};

	vk::DescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.setPoolSizes(poolSizes);
	descPoolInfo.setMaxSets(faust::MAX_DESC_SETS);
	descPoolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
	descPool = device->createDescriptorPoolUnique(descPoolInfo);
}


bool Context::checkDeviceExtensionSupport(const std::vector<const char*>& requiredExtensions) const {
	std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
	std::vector<std::string> requiredExtensionNames(requiredExtensions.begin(), requiredExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensionNames.erase(std::remove(requiredExtensionNames.begin(), requiredExtensionNames.end(), extension.extensionName),
			requiredExtensionNames.end());
	}

	if (requiredExtensionNames.empty()) {
		std::cout << "All required extensions are supported by the device." << std::endl;
		return true;
	}
	else {
		std::cout << "The following required extensions are not supported by the device:" << std::endl;
		for (const auto& name : requiredExtensionNames) {
			std::cout << "\t" << name << std::endl;
		}
		return false;
	}
}

uint32_t Context::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const {
	vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
	for (uint32_t i = 0; i != memProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error("failed to find suitable memory type");
}

void Context::oneTimeSubmit(const std::function<void(vk::CommandBuffer)>& func) const {
	vk::CommandBufferAllocateInfo commandBufferInfo;
	commandBufferInfo.setCommandPool(*commandPool);
	commandBufferInfo.setCommandBufferCount(1);

	vk::UniqueCommandBuffer commandBuffer = std::move(device->allocateCommandBuffersUnique(commandBufferInfo).front());
	commandBuffer->begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	func(*commandBuffer);
	commandBuffer->end();

	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBuffers(*commandBuffer);
	queue.submit(submitInfo);
	queue.waitIdle();
}

/*
vk::UniqueDescriptorSet Context::allocateDescriptorSet(vk::DescriptorSetLayout descSetLayout) {
}
*/

VKAPI_ATTR VkBool32 VKAPI_CALL Context::debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
	void* pUserData) {
	std::cerr << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}
