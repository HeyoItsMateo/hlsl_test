#include "vk.graphics.h"
#include <iostream>

vk::Window window("Vulkan");
vk::GPU vulkan;
vk::Shader hlsl_test("vertex_vert", VK_SHADER_STAGE_VERTEX_BIT);
int main() {
	std::cout << "Success!";
	return 0;
}