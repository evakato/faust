#pragma once

#include <array>

#include "device.hpp"
#include "texture.hpp"

class Cubemap : public Texture {
public:
	Cubemap(FaustDevice& device, const std::string& imagePath);
	~Cubemap();

private:
	void createCubemapImage(const std::string& imagePath);
	void copyBufferToImages(VkBuffer buffer, uint32_t width, uint32_t height, VkDeviceSize imageSize);
};