#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <set>
#include <fstream>
#include <iostream>
#include <functional>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "constants.hpp"

namespace faust {
	vk::PhysicalDevice pickSuitablePhysicalDevice(
		const vk::Instance& instance,
		const std::vector<const char*>& requiredExtensions);
};

struct Context {
	Context();
	bool checkDeviceExtensionSupport(const std::vector<const char*>& requiredExtensions) const;
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;
	void oneTimeSubmit(const std::function<void(vk::CommandBuffer)>& func) const;

	inline vk::UniqueDescriptorSet allocateDescriptorSet(vk::DescriptorSetLayout descSetLayout) {

		vk::DescriptorSetAllocateInfo descSetInfo;
		descSetInfo.setDescriptorPool(*descPool);
		descSetInfo.setSetLayouts(descSetLayout);
		return std::move(device->allocateDescriptorSetsUnique(descSetInfo).front());
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
		void* pUserData);

	GLFWwindow* window;
	vk::DynamicLoader dl;
	vk::UniqueInstance instance;
	vk::UniqueDebugUtilsMessengerEXT messenger;
	vk::UniqueSurfaceKHR surface;
	vk::UniqueDevice device;
	vk::PhysicalDevice physicalDevice;
	uint32_t queueFamilyIndex;
	vk::Queue queue;
	vk::UniqueCommandPool commandPool;
	vk::UniqueDescriptorPool descPool;
};


