#pragma once

#include <vulkan/vulkan.h>

#include "device.hpp"

class Buffer {
public:
	Buffer(FaustDevice& device);
	Buffer(FaustDevice& device, VkDeviceSize bufferSize, void* data, VkBufferUsageFlagBits usageFlags);
	~Buffer();

	VkBuffer getBuffer() const { return buffer; }
	void setupBuffer(VkDeviceSize bufferSize, void* newData, VkBufferUsageFlagBits usageFlags);

	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDevice device, VkPhysicalDevice pd);
	static VkCommandBuffer beginSingleTimeCommands(FaustDevice& device);
	static void endSingleTimeCommands(FaustDevice& device, VkCommandBuffer commandBuffer);

private:
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	FaustDevice& device;

	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
};