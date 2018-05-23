#ifndef VULKAN_POINTERS_HPP
#define VULKAN_POINTERS_HPP

#include "vulkan_context.hpp"
#include "vulkan_resources.hpp"
#include "commandbuffer.hpp"

namespace vk
{

struct VulkanPointers
{
    VulkanContext* context;
    VkDevice* logical_device;
    VulkanResources* resources;
    VulkanMemoryManager* memory_manager;
    CommandBuffer* commandbuffer;

    VulkanPointers(): context{nullptr}, logical_device{nullptr}, resources{nullptr}, memory_manager{nullptr}, commandbuffer{nullptr} {}
};

}//namespace vk
#endif //VULKAN_POINTERS_HPP
