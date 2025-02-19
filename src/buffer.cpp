#include "buffer.hpp"

Buffer::Buffer(FaustDevice& device) : device{ device } {}

Buffer::Buffer(FaustDevice& device, VkDeviceSize bufferSize, void* data, VkBufferUsageFlagBits usageFlags) : device{ device } {
	setupBuffer(bufferSize, data, usageFlags);
}

Buffer::~Buffer() {
	vkDestroyBuffer(device.getDevice(), buffer, nullptr);
	vkFreeMemory(device.getDevice(), bufferMemory, nullptr);
}

void Buffer::setupBuffer(VkDeviceSize bufferSize, void* newData, VkBufferUsageFlagBits usageFlags) {
	if (buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device.getDevice(), buffer, nullptr);
		buffer = VK_NULL_HANDLE;
	}
	if (bufferMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device.getDevice(), bufferMemory, nullptr);
		bufferMemory = VK_NULL_HANDLE;
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device.getDevice(), device.getPhysicalDevice()); // src flag = buffer is source in a memory transfer

	void* data;
	vkMapMemory(device.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, newData, (size_t)bufferSize);
	vkUnmapMemory(device.getDevice(), stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory, device.getDevice(), device.getPhysicalDevice()); // dst flag = buffer is destination in a memory transfer

	copyBuffer(stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(device.getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device.getDevice(), stagingBufferMemory, nullptr);
}

void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDevice device, VkPhysicalDevice pd) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FaustDevice::findMemoryType(memRequirements.memoryTypeBits, properties, pd);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device.getDevice(), buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FaustDevice::findMemoryType(memRequirements.memoryTypeBits, properties, device.getPhysicalDevice());

	if (vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(device.getDevice(), buffer, bufferMemory, 0);
}

VkCommandBuffer Buffer::beginSingleTimeCommands(FaustDevice& device) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = device.getCommandPool();
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device.getDevice(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Buffer::endSingleTimeCommands(FaustDevice& device, VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device.getGraphicsQueue());

	vkFreeCommandBuffers(device.getDevice(), device.getCommandPool(), 1, &commandBuffer);
}

void Buffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(device, commandBuffer);
}
