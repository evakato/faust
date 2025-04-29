#pragma once

#include "buffer.hpp"
#include "context.hpp"

struct Accel {
	Accel() = default;
	Accel(const Context& context, vk::AccelerationStructureGeometryKHR geometry, uint32_t primitiveCount, vk::AccelerationStructureTypeKHR type);

	Buffer buffer;
	vk::UniqueAccelerationStructureKHR accel;
	vk::WriteDescriptorSetAccelerationStructureKHR descAccelInfo;
};

