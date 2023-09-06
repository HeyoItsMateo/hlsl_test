#include "vk.gpu.h"

#include <thread>

namespace vk {
    /* Graphics Processing Unit */
    GPU::GPU()
    {
        pickPhysicalDevice();
        createPhysicalDevice();
    }
    GPU::~GPU()
    {
        vkDestroyDevice(device, nullptr);
    }
    //Public:
    uint32_t GPU::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }
    //Private:
    void GPU::pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;

        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {// Debug failure to find GPU with Vulkan support
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        for (const auto& device : devices)
        {// Pick most optimal GPU out of available devices
            if (isDeviceSuitable(device))
            {// Set device and specifications
                physicalDevice = device;
                getSampleCount();
                getSwapExtent();
                break;
            }
        }
        if (physicalDevice == VK_NULL_HANDLE)
        {// Debug failure to find/set GPU
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }
    void GPU::createPhysicalDevice()
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily.value(), presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo
            { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading feature for the device

        VkDeviceCreateInfo createInfo
        { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, graphicsFamily.value(), 0, &computeQueue);
        vkGetDeviceQueue(device, presentFamily.value(), 0, &presentQueue);

    }

    bool GPU::isDeviceSuitable(VkPhysicalDevice device)
    {
        bool queueFamilySupported = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device);
        bool swapChainAdequate = false;

        if (extensionsSupported) {
            querySwapChainSupport(device);
            swapChainAdequate = !formats.empty() && !presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
        return supportedFeatures.shaderFloat64 && queueFamilySupported && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }
    bool GPU::findQueueFamilies(VkPhysicalDevice device)
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                presentFamily = i;
            }

            if (graphicsFamily.has_value() && presentFamily.has_value()) {
                return true;
            }
            i++;
        }
        return false;
    }
    bool GPU::checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }
    void GPU::querySwapChainSupport(VkPhysicalDevice device)
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());
        }
    }

    void GPU::getSampleCount()
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { msaaSamples = VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { msaaSamples = VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { msaaSamples = VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { msaaSamples = VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { msaaSamples = VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { msaaSamples = VK_SAMPLE_COUNT_2_BIT; }
        else { msaaSamples = VK_SAMPLE_COUNT_1_BIT; }
    }
    void GPU::getSwapExtent()
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            Extent = capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(vk::Window::handle, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            Extent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            Extent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }
    }

    CPU::CPU(uint32_t numBuffers, VkFenceCreateFlags flags)
    {
        std::jthread commandFence([&] { createFence(); });
        createCommandPool();
        buffers.resize(numBuffers);
        createCommandBuffer(buffers);
    }
    CPU::~CPU()
    {
        vkDestroyCommandPool(GPU::device, pool, nullptr);
        vkDestroyFence(GPU::device, fence, nullptr);
    }
    /* Public */
    void CPU::signal()
    {
        vkWaitForFences(GPU::device, 1, &fence, VK_TRUE, UINT64_MAX);
        vkResetFences(GPU::device, 1, &fence);
    }
    /* Private */
    void CPU::createCommandPool()
    {
        VkCommandPoolCreateInfo poolInfo
        { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = GPU::graphicsFamily.value();

        if (vkCreateCommandPool(GPU::device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create CPU command pool!");
        }
    }
    void CPU::createFence(VkFenceCreateFlags flags)
    {
        VkFenceCreateInfo fenceInfo
        { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        fenceInfo.flags = flags;

        if (vkCreateFence(GPU::device, &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create CPU fence!");
        }
    }


    void CPU::createCommandBuffer(std::vector<VkCommandBuffer>& buffers)
    {
        VkCommandBufferAllocateInfo allocInfo
        { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = pool;
        allocInfo.commandBufferCount = buffers.size();

        if (vkAllocateCommandBuffers(GPU::device, &allocInfo, buffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void CPU::begin(VkCommandBuffer& buffer)
    {
        VkCommandBufferBeginInfo beginInfo
        { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(buffer, &beginInfo);
    }

    void CPU::end(VkCommandBuffer& buffer)
    {// TODO: Find out how to submit all commands in a single batch
        vkEndCommandBuffer(buffer);
    }

    void CPU::submit(VkCommandBuffer* buffers, uint32_t numBuffers) {
        VkSubmitInfo submitInfo
        { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = numBuffers; //TODO: find out how to allocate two or more command buffers
        submitInfo.pCommandBuffers = buffers;

        vkQueueSubmit(GPU::graphicsQueue, 1, &submitInfo, fence);
        signal();

        vkFreeCommandBuffers(GPU::device, pool, 1, buffers);
    }
}