#include "buffer.hpp"

Buffer::Buffer(const Context& context, Type type, vk::DeviceSize size, const void* data) {
	vk::BufferUsageFlags usage;
	vk::MemoryPropertyFlags memoryProps;
	using Usage = vk::BufferUsageFlagBits;
	using Memory = vk::MemoryPropertyFlagBits;

	switch (type) {
	case Type::AccelInput:
		usage = Usage::eAccelerationStructureBuildInputReadOnlyKHR | Usage::eStorageBuffer | Usage::eShaderDeviceAddress;
		memoryProps = Memory::eHostVisible | Memory::eHostCoherent;
		break;

	case Type::Vertex:
		usage = Usage::eVertexBuffer | Usage::eShaderDeviceAddress;
		memoryProps = Memory::eHostVisible | Memory::eHostCoherent;
		break;

	case Type::AccelInputIndex:
		usage = Usage::eIndexBuffer | Usage::eStorageBuffer | Usage::eShaderDeviceAddress | Usage::eAccelerationStructureBuildInputReadOnlyKHR;
		memoryProps = Memory::eHostVisible | Memory::eHostCoherent;
		break;

	case Type::Scratch:
		usage = Usage::eStorageBuffer | Usage::eShaderDeviceAddress;
		memoryProps = Memory::eDeviceLocal;
		break;

	case Type::AccelStorage:
		usage = Usage::eAccelerationStructureStorageKHR | Usage::eShaderDeviceAddress;
		memoryProps = Memory::eDeviceLocal;
		break;

	case Type::ShaderBindingTable:
		usage = Usage::eShaderBindingTableKHR | Usage::eShaderDeviceAddress;
		memoryProps = Memory::eHostVisible | Memory::eHostCoherent;
		break;

	case Type::Uniform:
		usage = Usage::eUniformBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;
		memoryProps = Memory::eHostVisible | Memory::eHostCoherent;
		break;

	default:
		throw std::runtime_error("Unknown buffer type!");
	}

	buffer = context.device->createBufferUnique({ {}, size, usage });

	// Allocate memory
	vk::MemoryRequirements requirements = context.device->getBufferMemoryRequirements(*buffer);
	uint32_t memoryTypeIndex = context.findMemoryType(requirements.memoryTypeBits, memoryProps);

	vk::MemoryAllocateFlagsInfo flagsInfo{ vk::MemoryAllocateFlagBits::eDeviceAddress };

	vk::MemoryAllocateInfo memoryInfo;
	memoryInfo.setAllocationSize(requirements.size);
	memoryInfo.setMemoryTypeIndex(memoryTypeIndex);
	memoryInfo.setPNext(&flagsInfo);
	memory = context.device->allocateMemoryUnique(memoryInfo);

	context.device->bindBufferMemory(*buffer, *memory, 0);

	// Get device address
	vk::BufferDeviceAddressInfoKHR bufferDeviceAI{ *buffer };
	deviceAddress = context.device->getBufferAddressKHR(&bufferDeviceAI);

	descBufferInfo.setBuffer(*buffer);
	descBufferInfo.setOffset(0);
	descBufferInfo.setRange(size);

	if (data) {
		mapped = context.device->mapMemory(*memory, 0, size);
		memcpy(mapped, data, size);
		context.device->unmapMemory(*memory);
	}
}
