#pragma once
#include "vk.gpu.h"
//#include "vk.swapchain.h"

#include <execution>

#include <variant>
#include <format>

namespace vk {
    struct Shader {
        VkShaderModule shaderModule;
        VkPipelineShaderStageCreateInfo stageInfo
        { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        Shader(const std::string& filename, VkShaderStageFlagBits shaderStage) {
            try {
                auto shaderCode = readFile(".\\shaders\\" + filename + ".spv");
                std::cout << "File read successful\n";
                createShaderModule(shaderCode, filename);
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
            stageInfo.stage = shaderStage;
            stageInfo.module = shaderModule;
            stageInfo.pName = "main";
        }
        ~Shader() {
            vkDestroyShaderModule(GPU::device, shaderModule, nullptr);
        }
    private:
        void createShaderModule(const std::vector<char>& code, const std::string& filename) {
            VkShaderModuleCreateInfo createInfo
            { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            createInfo.codeSize = code.size() * sizeof(uint32_t);
            createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
            uint32_t magic = createInfo.pCode[0];
            if (vkCreateShaderModule(GPU::device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                std::cout << "failed to generate module!\n";
                throw std::runtime_error("failed to create " + filename + " module!");
            }
            std::cout << "Successfully created module!\n";
        }
        // File Reader
        static std::vector<char> readFile(const std::string& filename) {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            if (!file.is_open()) {
                throw std::runtime_error(std::format("failed to open {}!", filename));
            }

            size_t fileSize = (size_t)file.tellg();
            std::vector<char> buffer(fileSize);

            file.seekg(0);
            file.read(buffer.data(), fileSize);

            file.close();

            return buffer;
        }
    };
}



