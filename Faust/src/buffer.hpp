#pragma once

#include "context.hpp"

struct Buffer {
	enum class Type {
		Scratch,
		AccelInput,
		Vertex,
		AccelInputIndex,
		AccelStorage,
		ShaderBindingTable,
		Uniform
	};

	Buffer() = default;
	Buffer(const Context& context, Type type, vk::DeviceSize size, const void* data = nullptr);

	vk::UniqueBuffer buffer;
	vk::UniqueDeviceMemory memory;
	vk::DescriptorBufferInfo descBufferInfo;
	void* mapped;
	uint64_t deviceAddress = 0;
};
