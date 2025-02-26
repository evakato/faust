#pragma once 

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <optional>
#include <vector>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "window.hpp"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class FaustDevice {
public:
	FaustDevice(FaustWindow& window);
	~FaustDevice();
	VkInstance getInstance() { return instance; }
	VkSurfaceKHR getSurface() { return surface; }
	VkCommandPool getCommandPool() { return commandPool; }
	VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
	VkDevice getDevice() { return device; }
	uint32_t getGraphicsFamily() { return physicalDeviceIndices.graphicsFamily.value(); }
	VkQueue getGraphicsQueue() { return graphicsQueue; }
	VkQueue getPresentQueue() { return presentQueue; }
	QueueFamilyIndices getIndices() { return physicalDeviceIndices; }
	VkSampleCountFlagBits getMsaaSamples() const {
		auto& state = FaustState::getInstance();
		if (!state.useMsaa) {
			return VK_SAMPLE_COUNT_1_BIT;
		}
		state.msaaSamples = msaaSamples;
		return msaaSamples;
	}
	SwapChainSupportDetails querySwapChainSupport() { return querySwapChainSupport(physicalDevice); }
	VkPhysicalDeviceProperties getProperties() const { return properties; }

	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice pd) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(pd, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	VkFormat findDepthFormat();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

private:
	void createInstance();
	bool checkValidationLayerSupport();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	void createSurface(FaustWindow& window);
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice pd);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void createCommandPool();
	void createLogicalDevice();
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkCommandPool commandPool;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	QueueFamilyIndices physicalDeviceIndices;

	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkPhysicalDeviceProperties properties{};
};
