#include "vulkan_context.hpp"
#include <utility>
#include "SDL_vulkan.h"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <cstring>
#include <limits>
#include <algorithm>
#include "utils.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE//because vulkan depth goes from 0 to 1 and gl used -1 to 1
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include "material.hpp"
#include "mesh.hpp"
#include "vulkan_utils.hpp"

namespace vk
{

//<f> Constructors & operator=
VulkanContext::VulkanContext(SDL_Window* window): m_window{window}, m_vulkan_instance{}, m_memory_manager{this}
{

}

VulkanContext::~VulkanContext() noexcept
{

}

VulkanContext::VulkanContext(const VulkanContext& other): m_memory_manager{other.m_memory_manager}
{

}

VulkanContext::VulkanContext(VulkanContext&& other) noexcept: m_memory_manager{std::move(other.m_memory_manager)}
{

}

VulkanContext& VulkanContext::operator=(const VulkanContext& other)
{
    if(this != &other)//not same ref
    {
        auto tmp(other);
        *this = std::move(tmp);
    }

    return *this;
}

VulkanContext& VulkanContext::operator=(VulkanContext&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
void VulkanContext::Init()
{
    //prepare context
    DefaultInitApplicationInfo();
    CreateVulkanInstance();
    CreateSurface();
    CreateDebugCallback();

    SelectPhysicalDevice();
    CreateLogicalDevice();

    m_init_ok = true;
}

void VulkanContext::Cleanup()
{
    m_memory_manager.Clear();

    vkDestroyDevice(m_logical_device, nullptr);

    DestroyDebugCallback(m_vulkan_instance, m_debug_callback, nullptr);

    vkDestroySurfaceKHR(m_vulkan_instance, m_surface, nullptr);
    vkDestroyInstance(m_vulkan_instance, nullptr);
}

void VulkanContext::WaitForIdle()
{
    vkDeviceWaitIdle(m_logical_device);
}
//</f> /Methods

//<f> Public util methods
uint32_t VulkanContext::FindMemoryTypeIndex(uint32_t type_filter, VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties memory_properties{};
    vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memory_properties);

    for(auto i{0}; i < memory_properties.memoryTypeCount; ++i)
    {
        if(type_filter & (1 << i))//check if bit at i position in type_filter is 1
            if((memory_properties.memoryTypes[i].propertyFlags & flags) == flags)//flags exist
                return i;
    }

    throw std::runtime_error("Failed to find suitable memory type");
}

VkImageView VulkanContext::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags)
{
    VkImageView view;
    VkImageViewCreateInfo view_create_info;
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    //need these two or driver screams
    view_create_info.pNext = nullptr;
    view_create_info.flags = 0;

    view_create_info.image = image;
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = format;

    //default component colour chanels values
    view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    //what will the image be used for (mipmaps and stuff)
    view_create_info.subresourceRange.aspectMask = aspect_flags;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.layerCount = 1;
    view_create_info.subresourceRange.baseArrayLayer = 0;

    //create view
    if(vkCreateImageView(m_logical_device, &view_create_info, nullptr, &view) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image view");

    return view;
}

VkImageView VulkanContext::CreateCubemapView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags)
{
    VkImageView view;
    VkImageViewCreateInfo view_create_info;
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    //need these two or driver screams
    view_create_info.pNext = nullptr;
    view_create_info.flags = 0;

    view_create_info.image = image;
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    view_create_info.format = format;

    //default component colour chanels values
    view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
    view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
    view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
    view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;

    //what will the image be used for (mipmaps and stuff)
    view_create_info.subresourceRange.aspectMask = aspect_flags;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.layerCount = 6;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    //
    // // 6 array layers (faces)
	// view_create_info.subresourceRange.layerCount = 6;
	// 	// Set number of mip levels
	// view_create_info.subresourceRange.levelCount = 1;

    //create view
    if(vkCreateImageView(m_logical_device, &view_create_info, nullptr, &view) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image view");

    return view;
}

VkFormat VulkanContext::FindCapableFormat(const std::vector<VkFormat>& wanted_formats, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for(auto& format : wanted_formats)
    {
        //check if format is supported
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(m_physical_device, format, &properties);

        if(tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
            return format;
        if(tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
            return format;
    }

    throw std::runtime_error("Failed to find requested image format");
}



//</f> /Public util methods

//<f> Getters/Setters
void VulkanContext::ApplicationInfo(const VkApplicationInfo& info) { m_application_info = info; }
VkApplicationInfo VulkanContext::ApplicationInfo() const { return m_application_info; }
//</f> /Getters/Setters

//<f> Private Init
//<f> Instance & Validation
void VulkanContext::DefaultInitApplicationInfo()
{
    m_application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    m_application_info.pNext = nullptr;
    m_application_info.pApplicationName = "Vulkan demo";
    m_application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    m_application_info.pEngineName = "No Engine";
    m_application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    m_application_info.apiVersion = VK_API_VERSION_1_0;
    // m_application_info.apiVersion = VK_API_VERSION_1_1;
}

void VulkanContext::CreateVulkanInstance()
{
    VkInstanceCreateInfo info{vk::InstanceCreateInfo()};
    info.pApplicationInfo = &m_application_info;

    //<f> Validation Layers
    if(m_enable_validation_layers)
    {
        if(CheckValidationLayerAvailability())
        {
            info.enabledLayerCount = static_cast<uint32_t>(m_required_validation_layers.size());
            info.ppEnabledLayerNames = m_required_validation_layers.data();
        }
        else
            throw std::runtime_error("Requested Validation Layers are not available.");
    }
    else
    {
        info.enabledLayerCount = 0;//disable validation layer
    }
    //</f> /Validation Layers

    //<f> Extensions
    auto extensions{ GetRequiredExtensions() };

    info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    info.ppEnabledExtensionNames = extensions.data();
    //</f> /Extensions

    //finally create the instance
    VkResult result{ vkCreateInstance(&info, nullptr, &m_vulkan_instance) };
    if(result != VK_SUCCESS)
        throw std::runtime_error("Failed to create vulkan instance");
}

void VulkanContext::CreateDebugCallback()
{
    if(!m_enable_validation_layers)
        return;

    VkDebugReportCallbackCreateInfoEXT info{vk::DebugReportCallbackCreateInfoEXT()};
    info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

    info.pfnCallback = DebugReportCallback;//static function

    if (LoadDebugReportCallbackEXTAddr(m_vulkan_instance, &info, nullptr, &m_debug_callback) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create debug callback!");
    }
}

void VulkanContext::DestroyDebugCallback(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* allocator)
{
    auto function_addr = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

    if (function_addr != nullptr)//found function address
    {
        function_addr(instance, callback, allocator);
    }
}

bool VulkanContext::CheckValidationLayerAvailability()
{
    uint32_t available_layers_count{0};
    vkEnumerateInstanceLayerProperties(&available_layers_count, nullptr);//get total layer count

    std::vector<VkLayerProperties> available_layers{available_layers_count};
    vkEnumerateInstanceLayerProperties(&available_layers_count, available_layers.data());

    //check if needed layer exists
    for(auto& needed_layer : m_required_validation_layers)
    {
        bool layer_found{false};

        for(auto& available_layer : available_layers)//check avilable names for current needed layer
        {
            if(std::strcmp(available_layer.layerName, needed_layer) == 0)
            {
                layer_found = true;
                break;
            }
        }

        if(!layer_found)//current needed layer was not found on the available pool
            return false;
    }
    //all layers were found
    return true;
}

std::vector<const char*> VulkanContext::GetRequiredExtensions()
{

    //get extensions from sdl window
    unsigned int extensions_count{0};
    std::vector<const char*> extensions{};
    // const char** extensions_names = nullptr;

    //get total extensions count
    if( !SDL_Vulkan_GetInstanceExtensions(m_window, &extensions_count , NULL) )//exit if it fails to get extensions count
        throw std::runtime_error("Failed to get sdl extensions count.");

    extensions.resize(extensions_count);
    if( !SDL_Vulkan_GetInstanceExtensions(m_window, &extensions_count , extensions.data()) )//exit if it fails to get extensions count
        throw std::runtime_error("Failed to get sdl extensions names.");

    if(m_enable_validation_layers)
    {
        //if we use validation layers we need to load the following extensions
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return extensions;
}

VkResult VulkanContext::LoadDebugReportCallbackEXTAddr(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator, VkDebugReportCallbackEXT* callback)
{
    auto function_addr = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

    if (function_addr != nullptr)//found function address
    {
        return function_addr(instance, create_info, allocator, callback);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugReportCallback( VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
    uint64_t object, size_t location, int32_t message_code, const char* layer_prefix, const char* message, void* user_data)
{
    std::cerr << message << std::endl;
    return VK_FALSE;
}
//</f> /Instance & Validation

//<f> Devices
void VulkanContext::SelectPhysicalDevice()
{
    //get device count
    uint32_t device_count{0};
    vkEnumeratePhysicalDevices(m_vulkan_instance, &device_count, nullptr);
    if (device_count == 0) throw std::runtime_error("Unable to find a GPU with Vulkan support!");

    //get device list
    std::vector<VkPhysicalDevice> devices{device_count};
    vkEnumeratePhysicalDevices(m_vulkan_instance, &device_count, devices.data());

    //check if we have a capable device
    for(const auto& device : devices)
    {
        if(IsPhysicalDeviceCapable(device))
        {
            m_physical_device = device;
            break;
        }
    }
    //failed to find a capable device
    if (m_physical_device == VK_NULL_HANDLE) throw std::runtime_error("Failed to find a capable GPU");

}

bool VulkanContext::IsPhysicalDeviceCapable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(device, &device_properties);

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(device, &device_features);

    bool has_required_features_and_properties{device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && device_features.geometryShader};

    bool has_required_extensions{CheckPhysicalDeviceExtensionSupport(device)};

    m_queue_families_indices = FindQueueFamiliesIndeces(device);
    bool has_required_queue_families{m_queue_families_indices.AllFound()};

    m_swapchain_support = CheckSwapchainSupport(device);
    bool has_capable_swap_chain{m_swapchain_support.SwapchainIsCapable()};

    return  has_required_features_and_properties && has_required_extensions && has_required_queue_families && has_capable_swap_chain && device_features.samplerAnisotropy;
}

bool VulkanContext::CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensions_count{0};
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensions_count, nullptr);

    std::vector<VkExtensionProperties> extensions_properties{extensions_count};
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensions_count, extensions_properties.data());

    //check if needed layer exists
    for(auto& needed_extension : m_required_physical_device_extensions)
    {
        bool extension_found{false};

        for(auto& available_extensions : extensions_properties)//check avilable names for current needed layer
        {
            if(std::strcmp(available_extensions.extensionName, needed_extension) == 0)
            {
                extension_found = true;
                break;
            }
        }

        if(!extension_found)//current needed extension was not found on the available pool
            return false;
    }

    return true;
}

void VulkanContext::CreateLogicalDevice()
{
    auto queue_families_indices{FindQueueFamiliesIndeces(m_physical_device)};
    auto indices_vector{queue_families_indices.ToVector()};
    std::vector<VkDeviceQueueCreateInfo> queues_infos{};

    float queue_priority {1.f};//outside of loop because of scope
    for(auto index : indices_vector)
    {
        VkDeviceQueueCreateInfo queue_info;
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        //radv needs these two lines
        queue_info.pNext = nullptr;
        queue_info.flags = 0;

        queue_info.queueFamilyIndex = index;
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = &queue_priority;

        queues_infos.push_back(queue_info);
    }

    VkPhysicalDeviceFeatures physical_device_features{};
    physical_device_features.samplerAnisotropy = VK_TRUE;
    physical_device_features.fillModeNonSolid = VK_TRUE;
    // physical_device_features.geometryShader = VK_TRUE;

    //<f> Device Create Info
    VkDeviceCreateInfo device_info{vk::DeviceCreateInfo()};
    device_info.queueCreateInfoCount = static_cast<uint32_t>(queues_infos.size());
    device_info.pQueueCreateInfos = queues_infos.data();
    device_info.pEnabledFeatures = &physical_device_features;

    device_info.enabledExtensionCount = static_cast<uint32_t>(m_required_physical_device_extensions.size());
    device_info.ppEnabledExtensionNames = m_required_physical_device_extensions.data();
    //</f> /Device Create Info

    //set validation layers
    if(m_enable_validation_layers)
    {std::cout<<"Validation Layers - ON"<<std::endl;
        device_info.enabledLayerCount = static_cast<uint32_t>(m_required_validation_layers.size());
        device_info.ppEnabledLayerNames = m_required_validation_layers.data();
    }
    else
    {
        std::cout<<"Validation Layers - OFF"<<std::endl;
        device_info.enabledLayerCount = 0;//disable validation layer
    }

    if(vkCreateDevice(m_physical_device, &device_info, nullptr, &m_logical_device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device");
}

QueueFamiliesIndices VulkanContext::FindQueueFamiliesIndeces(VkPhysicalDevice device)
{
    QueueFamiliesIndices indices;

    uint32_t queues_count{0};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queues_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families{queues_count};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queues_count, queue_families.data());

    for(auto i{0}; i < queue_families.size(); ++i)
    {
        auto family{queue_families[i]};

        //<f> Graphics
        //can use more than 0 queues && is graphical
        if(family.queueCount > 0 && family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_family = i;
        }
        //</f> /Graphics

        //<f> Presentation
        //check if queue supports presentation
        VkBool32 presentation_supported{false};
        //check is current family supports presentation (will set presentation_supported true/false)
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentation_supported);

        if(family.queueCount > 0 && presentation_supported)
        {
            indices.presentation_family = i;
        }
        //</f> /Presentation
    }
    //if we reach this function we know that all we need exist
    return indices;
}

//<f> SwapChains
SwapchainSupport VulkanContext::CheckSwapchainSupport(VkPhysicalDevice device)
{
    SwapchainSupport swapchain_struct{};

    //get physical device swap chain capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &swapchain_struct.capabilities);

    //get physical device swap chain formats
    uint32_t formats_count{0};
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formats_count, nullptr);

    swapchain_struct.formats.resize(formats_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formats_count, swapchain_struct.formats.data());

    //get physical device swap chain presentation_modes
    uint32_t presentation_modes_count{0};
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentation_modes_count, nullptr);

    swapchain_struct.presentation_modes.resize(presentation_modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentation_modes_count, swapchain_struct.presentation_modes.data());

    return swapchain_struct;
}
//</f> /SwapChains
//</f> /Devices

//<f> Rendering
void VulkanContext::CreateSurface()
{
    if( !SDL_Vulkan_CreateSurface(m_window, m_vulkan_instance, &m_surface) )
        throw std::runtime_error("Failed to create surface");
}
//</f> /Rendering

//</f> /Private Init

}//namespace vk
