#ifndef VULKAN_STRUCTS_HPP
#define VULKAN_STRUCTS_HPP

#include <vulkan/vulkan.h>
#include <vector>

struct QueueFamiliesIndices
{
    int graphics_family{-1};
    int presentation_family{-1};

    /**
     * \brief Checks is all queue families indices are set(>=0)
     */
    bool AllFound(){ return graphics_family >= 0 && presentation_family >= 0; }

    std::vector<int> ToVector(){ return {graphics_family, presentation_family}; }
    std::vector<uint32_t> ToVectorUInt32(){ return {static_cast<uint32_t>(graphics_family), static_cast<uint32_t>(presentation_family)}; }
};

struct SwapChainSupportStruct
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats{};
    std::vector<VkPresentModeKHR> presentation_modes{};

    bool SwapChainIsCapable(){ return !formats.empty() && !presentation_modes.empty(); }
};

#endif//VULKAN_STRUCTS_HPP
