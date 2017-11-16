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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

//<f> Constructors & operator=
VulkanContext::VulkanContext(SDL_Window* window): m_window{window}, m_vulkan_instance{}
{

}

VulkanContext::~VulkanContext() noexcept
{

}

VulkanContext::VulkanContext(const VulkanContext& other)
{

}

VulkanContext::VulkanContext(VulkanContext&& other) noexcept
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
    DefaultInitApplicationInfo();
    CreateVulkanInstance();
    CreateSurface();

    CreateDebugCallback();

    SelectPhysicalDevice();
    CreateLogicalDevice();

    CreateSwapChain();
    CreateImageViews();

    CreateDescriptorSetLayout();
    CreateDescriptorPool();

    CreateRenderPass();
    CreateGraphicsPipeline();

    CreateFramebuffers();

    CreateCommandPool();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffer();
    CreateDescriptorSet();//needs uniform buffer
    CreateCommandBuffers();
    CreateSemaphores();
}

void VulkanContext::Cleanup()
{
    CleanUpSwapChain();//needs the command_pool to exist

    vkDestroySemaphore(m_logical_device, m_image_available_semaphore, nullptr);
    vkDestroySemaphore(m_logical_device, m_render_finished_semaphore, nullptr);

    // vkFreeCommandBuffers(m_logical_device, m_command_pool, m_command_buffers.size(), m_command_buffers.data());

    // vkFreeDescriptorSets(m_logical_device, m_descriptor_pool, 1, &m_descriptor_set);//no need the pool destruction takes care of it
    vkDestroyDescriptorPool(m_logical_device, m_descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(m_logical_device, m_descriptor_set_layout, nullptr);

    vkDestroyBuffer(m_logical_device, m_uniform_buffer, nullptr);
    vkFreeMemory(m_logical_device, m_uniform_buffer_memory, nullptr);
    vkDestroyBuffer(m_logical_device, m_index_buffer, nullptr);
    vkFreeMemory(m_logical_device, m_index_buffer_memory, nullptr);
    vkDestroyBuffer(m_logical_device, m_vertex_buffer, nullptr);
    vkFreeMemory(m_logical_device, m_vertex_buffer_memory, nullptr);//after destroying the buffer we no longer need the allocated memory

    vkDestroyCommandPool(m_logical_device, m_command_pool, nullptr);

    // for(auto i{0}; i < m_swap_chain_framebuffers.size(); ++i)
    //     vkDestroyFramebuffer(m_logical_device, m_swap_chain_framebuffers[i], nullptr);
    //
    // vkDestroyPipeline(m_logical_device, m_graphics_pipeline, nullptr);
    // vkDestroyPipelineLayout(m_logical_device, m_pipeline_layout, nullptr);//the pipeline uses this so it is destroyed after the pipeline
    // vkDestroyRenderPass(m_logical_device, m_render_pass, nullptr);
    //
    // for(auto i{0}; i<m_swap_chain_image_views.size(); ++i)
    //     vkDestroyImageView(m_logical_device, m_swap_chain_image_views[i], nullptr);
    //
    // vkDestroySwapchainKHR(m_logical_device, m_swap_chain, nullptr);

    vkDestroyDevice(m_logical_device, nullptr);

    DestroyDebugCallback(m_vulkan_instance, m_debug_callback, nullptr);

    vkDestroySurfaceKHR(m_vulkan_instance, m_surface, nullptr);
    vkDestroyInstance(m_vulkan_instance, nullptr);
}

void VulkanContext::DrawFrame()
{
    //get image index from swap chain
    uint32_t image_index{0};
    auto acquire_result{vkAcquireNextImageKHR(m_logical_device, m_swap_chain, std::numeric_limits<uint64_t>::max(), m_image_available_semaphore, VK_NULL_HANDLE, &image_index)};

    if(acquire_result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
        return;
    }
    else if(acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR)//only these return an image
        throw std::runtime_error("Failed to acquire swap chain image");

    //<f> Submit command
    VkSubmitInfo command_submit_info{};
    command_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    std::vector<VkSemaphore> wait_semaphores{m_image_available_semaphore};
    std::vector<VkPipelineStageFlags> wait_stages{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    command_submit_info.waitSemaphoreCount = 1;
    command_submit_info.pWaitSemaphores = wait_semaphores.data();//semaphores to wait for
    command_submit_info.pWaitDstStageMask = wait_stages.data();

    command_submit_info.commandBufferCount = 1;
    command_submit_info.pCommandBuffers = &m_command_buffers[image_index];

    std::vector<VkSemaphore> signal_semaphores{m_render_finished_semaphore};
    command_submit_info.signalSemaphoreCount = 1;
    command_submit_info.pSignalSemaphores = signal_semaphores.data();//semaphores to unlock after finishing

    if( vkQueueSubmit(m_graphics_queue, 1, &command_submit_info, VK_NULL_HANDLE) != VK_SUCCESS )
        throw std::runtime_error("Failed to submit draw commands to buffer");
    //</f> /Submit command

    //<f> Presentation
    VkPresentInfoKHR presentation_info{};
    presentation_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentation_info.pNext = nullptr;

    presentation_info.waitSemaphoreCount = 1;
    presentation_info.pWaitSemaphores = signal_semaphores.data();//wait for these semaphores before presentation

    std::vector<VkSwapchainKHR> swap_chains{m_swap_chain};
    presentation_info.swapchainCount = 1;
    presentation_info.pSwapchains = swap_chains.data();
    presentation_info.pImageIndices = &image_index;

    auto queue_result{vkQueuePresentKHR(m_graphics_queue, &presentation_info)};

    if (queue_result == VK_ERROR_OUT_OF_DATE_KHR || queue_result == VK_SUBOPTIMAL_KHR)
    {
        RecreateSwapChain();
    }
    else if (queue_result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    vkQueueWaitIdle(m_presentation_queue);
    //</f> /Presentation
}

void VulkanContext::WaitForIdle()
{
    vkDeviceWaitIdle(m_logical_device);
}

void VulkanContext::Resize()
{
    RecreateSwapChain();
}

void VulkanContext::UpdateUniformBuffer()
{
    static auto start_time{std::chrono::high_resolution_clock::now()};

    auto current_time{std::chrono::high_resolution_clock::now()};

    float time{ std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count() / 1000.f };

    UniformBufferObject obj{};

    //rotate model 90 degrees * time on z axis
    obj.model = glm::rotate(glm::mat4{1.0f}, glm::radians(90.f) * time, glm::vec3{0.f, 0.f, 1.f});

    //camera position, point looking at, up vector(what side is up)
    obj.view = glm::lookAt(glm::vec3{2.f, 2.f, 2.f}, glm::vec3{0.f, 0.f, 0.f}, glm::vec3{0.f, 0.f, 1.f});

    obj.projection = glm::perspective(glm::radians(45.0f), m_swap_chain_extent.width / (float) m_swap_chain_extent.height, 0.1f, 10.0f);//TODO::Study this

    obj.projection[1][1] *= -1;//because opengl inverted clip coordinates

    void* data;
    vkMapMemory(m_logical_device, m_uniform_buffer_memory, 0, sizeof(obj), 0, &data);
    memcpy(data, &obj, sizeof(obj));
    vkUnmapMemory(m_logical_device, m_uniform_buffer_memory);
}
//</f> /Methods

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
}

void VulkanContext::CreateVulkanInstance()
{
    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &m_application_info;
    info.pNext  = nullptr;//need to be set to nullptr or vkCreateInstance seg faults

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

    VkDebugReportCallbackCreateInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
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
    bool has_required_queue_families{HasNeededQueueFamilies(device)};

    bool has_capable_swap_chain{CheckSwapChainSupport(device).SwapChainIsCapable()};

    return  has_required_features_and_properties && has_required_extensions && has_required_queue_families && has_capable_swap_chain;
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

    //<f> Device Create Info
    VkDeviceCreateInfo device_info{};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = static_cast<uint32_t>(queues_infos.size());
    device_info.pQueueCreateInfos = queues_infos.data();
    device_info.pEnabledFeatures = &physical_device_features;

    device_info.enabledExtensionCount = static_cast<uint32_t>(m_required_physical_device_extensions.size());
    device_info.ppEnabledExtensionNames = m_required_physical_device_extensions.data();
    //</f> /Device Create Info

    //set validation layers
    if(m_enable_validation_layers)
    {
        device_info.enabledLayerCount = static_cast<uint32_t>(m_required_validation_layers.size());
        device_info.ppEnabledLayerNames = m_required_validation_layers.data();
    }
    else
    {
        device_info.enabledLayerCount = 0;//disable validation layer
    }

    if(vkCreateDevice(m_physical_device, &device_info, nullptr, &m_logical_device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device");

    //get queues from logical device
    vkGetDeviceQueue(m_logical_device, queue_families_indices.graphics_family, 0, &m_graphics_queue);
    vkGetDeviceQueue(m_logical_device, queue_families_indices.presentation_family, 0, &m_presentation_queue);
}

//<f> Queue Families
bool VulkanContext::HasNeededQueueFamilies(VkPhysicalDevice device)
{
    QueueFamiliesIndices indices{};

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
            indices.graphics_family = i;
        //</f> /Graphics

        //<f> Presentation
        //check if queue supports presentation
        VkBool32 presentation_supported{false};
        //check is current family supports presentation (will set presentation_supported true/false)
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentation_supported);

        if(family.queueCount > 0 && presentation_supported)
            indices.presentation_family = i;
        //</f> /Presentation

        if(indices.AllFound())//found all needed queue_families
            return true;
    }

    return false;
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
//</f> /Queue Families

//<f> SwapChains
SwapChainSupportStruct VulkanContext::CheckSwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportStruct swap_chain_struct{};

    //get physical device swap chain capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &swap_chain_struct.capabilities);

    //get physical device swap chain formats
    uint32_t formats_count{0};
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formats_count, nullptr);

    swap_chain_struct.formats.resize(formats_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formats_count, swap_chain_struct.formats.data());

    //get physical device swap chain presentation_modes
    uint32_t presentation_modes_count{0};
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentation_modes_count, nullptr);

    swap_chain_struct.presentation_modes.resize(presentation_modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentation_modes_count, swap_chain_struct.presentation_modes.data());

    return swap_chain_struct;
}

VkSurfaceFormatKHR VulkanContext::SelectSwapChainFormat(const SwapChainSupportStruct& swap_chain_struct)
{
    //if we only have one format available
    if(swap_chain_struct.formats.size() == 1 && swap_chain_struct.formats[0].format == VK_FORMAT_UNDEFINED )
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    //else we find the one we want
    for(const auto& format : swap_chain_struct.formats)
    {
        if(format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }

    //otherwise just return the first one
    return swap_chain_struct.formats[0];
}

VkPresentModeKHR VulkanContext::SelectSwapChainPresentationMode(const SwapChainSupportStruct& swap_chain_struct)
{
    for(const auto& mode : swap_chain_struct.presentation_modes)
    {
        if(mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return mode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;//guaranteed to be available
}

VkExtent2D VulkanContext::SelectSwapChainExtent(const SwapChainSupportStruct& swap_chain_struct)
{
    auto capabilities{swap_chain_struct.capabilities};

    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;

    int w{0}, h{0};
    SDL_GetWindowSize(m_window, &w,&h);
    VkExtent2D extent{static_cast<uint32_t>(w), static_cast<uint32_t>(h)};

    //clamp width and height to min and max allowed size
    extent.width = std::max( capabilities.minImageExtent.width, std::min( capabilities.maxImageExtent.width, extent.width ) );
    extent.height = std::max( capabilities.minImageExtent.height, std::min( capabilities.maxImageExtent.height, extent.height ) );

    return extent;
}

void VulkanContext::CreateSwapChain()
{
    auto swap_chain_struct{CheckSwapChainSupport(m_physical_device)};

    auto format{SelectSwapChainFormat(swap_chain_struct)};
    auto presentation_mode{SelectSwapChainPresentationMode(swap_chain_struct)};
    auto extent{SelectSwapChainExtent(swap_chain_struct)};

    m_swap_chain_format = format.format;
    m_swap_chain_extent = extent;

    //set min number of images in the swap chain
    uint32_t min_image_count{ swap_chain_struct.capabilities.minImageCount + 1 };

    if(swap_chain_struct.capabilities.maxImageCount > 0)//0 = infinite (memory limits apply)
        min_image_count = std::min(min_image_count, swap_chain_struct.capabilities.maxImageCount);

    //create said swap chain
    VkSwapchainCreateInfoKHR swap_chain_create_info{};

    swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_chain_create_info.surface = m_surface;
    //need these or the driver screams at me (seg fault)
    swap_chain_create_info.pNext = nullptr;
    swap_chain_create_info.flags = 0;

    swap_chain_create_info.imageArrayLayers = 1;//default
    swap_chain_create_info.imageExtent = extent;
    swap_chain_create_info.imageFormat = format.format;
    swap_chain_create_info.imageColorSpace = format.colorSpace;
    swap_chain_create_info.minImageCount = min_image_count;
    swap_chain_create_info.presentMode = presentation_mode;
    swap_chain_create_info.clipped = VK_TRUE;//clip obscured pixel
    swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;//we render directly on it

    //set how queues will handle the swap chain images
    auto queue_indices{FindQueueFamiliesIndeces(m_physical_device)};
    auto uint32_indices{queue_indices.ToVectorUInt32()};

    if(queue_indices.graphics_family != queue_indices.presentation_family)//not the same queue
    {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//can be used by multiple families at he same time (need at least 2 families)
        swap_chain_create_info.queueFamilyIndexCount = static_cast<uint32_t>(uint32_indices.size());
        swap_chain_create_info.pQueueFamilyIndices = uint32_indices.data();//who will share usage
    }
    else//same queue
    {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//owner by one family at a type, need explicit ownership change
        swap_chain_create_info.queueFamilyIndexCount = 0;
        swap_chain_create_info.pQueueFamilyIndices = nullptr;
    }

    swap_chain_create_info.preTransform = swap_chain_struct.capabilities.currentTransform;//transformation to apply in the chain. Current mean no transformation
    swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//alph blend with other windows
    swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;//used when we need to recreate the chain

    //finally we create the damn swap chain
    if(vkCreateSwapchainKHR(m_logical_device, &swap_chain_create_info, nullptr, &m_swap_chain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swap chain");

    //get swap chain images
    uint32_t swap_chain_image_count{0};
    //vulkan can create more than the number we want, so we need to ask for how many are there
    vkGetSwapchainImagesKHR(m_logical_device, m_swap_chain, &swap_chain_image_count, nullptr);

    m_swap_chain_images.resize(swap_chain_image_count);
    vkGetSwapchainImagesKHR(m_logical_device, m_swap_chain, &swap_chain_image_count, m_swap_chain_images.data());
}

void VulkanContext::RecreateSwapChain()
{
    //wait for the device to finish what it is doing
    vkDeviceWaitIdle(m_logical_device);

    //destroy previous swap chain elements
    CleanUpSwapChain();

    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandBuffers();
}

void VulkanContext::CleanUpSwapChain()
{
    vkFreeCommandBuffers(m_logical_device, m_command_pool, m_command_buffers.size(), m_command_buffers.data());

    for(auto i{0}; i < m_swap_chain_framebuffers.size(); ++i)
        vkDestroyFramebuffer(m_logical_device, m_swap_chain_framebuffers[i], nullptr);

    vkDestroyPipeline(m_logical_device, m_graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(m_logical_device, m_pipeline_layout, nullptr);//the pipeline uses this so it is destroyed after the pipeline
    vkDestroyRenderPass(m_logical_device, m_render_pass, nullptr);

    for(auto i{0}; i<m_swap_chain_image_views.size(); ++i)
        vkDestroyImageView(m_logical_device, m_swap_chain_image_views[i], nullptr);

    vkDestroySwapchainKHR(m_logical_device, m_swap_chain, nullptr);
}
//</f> /SwapChains
//</f> /Devices

//<f> Rendering
void VulkanContext::CreateSurface()
{
    if( !SDL_Vulkan_CreateSurface(m_window, m_vulkan_instance, &m_surface) )
        throw std::runtime_error("Failed to create surface");
}

void VulkanContext::CreateImageViews()
{
    m_swap_chain_image_views.resize(m_swap_chain_images.size());

    //create each image view, based on an image
    for(auto i{0}; i<m_swap_chain_images.size(); ++i)
    {
        VkImageViewCreateInfo view_create_info;
        view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        //need these two or driver screams
        view_create_info.pNext = nullptr;
        view_create_info.flags = 0;

        view_create_info.image = m_swap_chain_images[i];
        view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_create_info.format = m_swap_chain_format;

        //default component colour chanels values
        view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        //what will the image be used for (mipmaps and stuff)
        view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_create_info.subresourceRange.levelCount = 1;
        view_create_info.subresourceRange.baseMipLevel = 0;
        view_create_info.subresourceRange.layerCount = 1;
        view_create_info.subresourceRange.baseArrayLayer = 0;

        //create view
        if(vkCreateImageView(m_logical_device, &view_create_info, nullptr, &m_swap_chain_image_views[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create image view at index "+std::to_string(i));
    }
}

void VulkanContext::CreateGraphicsPipeline()
{
    //<f> Shader Stages
    //<f> Vertex Shaders
    //Get code
    auto vertex_shader{ReadFile("data/shaders/vert.spv")};

    //create Shader module
    VkShaderModule vertex_module{CreateShaderModule(vertex_shader)};

    //create pipeline shader
    VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info{};
    vertex_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    //needed or driver screams at me
    vertex_shader_stage_create_info.pNext = nullptr;
    vertex_shader_stage_create_info.flags = 0;

    vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_create_info.module = vertex_module;
    vertex_shader_stage_create_info.pName = "main";//name entry point for the shader stage
    //</f> /Vertex Shaders

    //<f> Fragment Shaders
    //get code
    auto fragment_shader{ReadFile("data/shaders/frag.spv")};

    //create shader module
    VkShaderModule fragment_module{CreateShaderModule(fragment_shader)};

    //create shader pipeline
    VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info{};
    fragment_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    //needed or driver screams at me
    fragment_shader_stage_create_info.pNext = nullptr;
    fragment_shader_stage_create_info.flags = 0;

    fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_stage_create_info.module = fragment_module;
    fragment_shader_stage_create_info.pName = "main";//name entry point for the shader stage
    //</f> /Fragment Shaders

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages{vertex_shader_stage_create_info, fragment_shader_stage_create_info};
    //</f> /Shader Stages

    //<f> Vertex Input
    VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
    vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_create_info.pNext = nullptr;
    vertex_input_create_info.flags = 0;

    auto attribute_description{Vertex::CreateAttributeDescription()};
    auto binding_description{Vertex::CreateBindingDescrioption()};

    vertex_input_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_description.size());
    vertex_input_create_info.pVertexAttributeDescriptions = attribute_description.data();

    vertex_input_create_info.vertexBindingDescriptionCount = 1;
    vertex_input_create_info.pVertexBindingDescriptions = &binding_description;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
    input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.pNext = nullptr;
    input_assembly_create_info.flags = 0;

    // input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;//TODO:study more of this
    // input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;//TODO:study more of this
    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;//TODO:study more of this
    input_assembly_create_info.primitiveRestartEnable = VK_FALSE;
    //</f> /Vertex Input

    //<f> Viewport & scissors
    VkViewport viewport{};//like srouce rect
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = m_swap_chain_extent.width;
    viewport.height = m_swap_chain_extent.height;
    viewport.minDepth = 0.f;//default
    viewport.maxDepth = 1.f;//default

    VkRect2D scissors{};//like destination rect
    scissors.offset = {0,0};
    scissors.extent = m_swap_chain_extent;

    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.pNext = nullptr;
    viewport_state_create_info.flags = 0;

    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissors;
    //</f> /Viewport & scissors

    //<f> Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer_create_info{};
    rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_create_info.pNext = nullptr;
    rasterizer_create_info.flags = 0;
    rasterizer_create_info.depthClampEnable = VK_FALSE;//True needs enabling gpu feature
    rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;//if true all geometry is discarded, basically disables any output to the framebuffer

    rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;//fill, line(wireframe), point(vectex only). Other than fill needs enabling gpu feature
    rasterizer_create_info.lineWidth = 1.0f;//bigger than 1 needs enabling gpu feature (wideLines)

    rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;//vertex order for faces to be considered front-facing (counter-clock because of y flip)
    // rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;//vertex order for faces to be considered front-facing

    //alter the depth values by adding a constant value or biasing them based on a fragment's slope
    //can be used for shadow mapping
    rasterizer_create_info.depthBiasEnable = VK_FALSE;
    rasterizer_create_info.depthBiasConstantFactor = 0;
    rasterizer_create_info.depthBiasClamp = 0;
    rasterizer_create_info.depthBiasSlopeFactor = 0;
    //</f> /Rasterizer

    //<f> Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling_create_info{};//not in use, for now
    multisampling_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

    multisampling_create_info.sampleShadingEnable = VK_FALSE;
    multisampling_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling_create_info.minSampleShading = 1.0f; // Optional
    multisampling_create_info.pSampleMask = nullptr; // Optional
    multisampling_create_info.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling_create_info.alphaToOneEnable = VK_FALSE; // Optional
    //</f> /Multisampling

    //<f> Colour Blending
    //need more studying
    VkPipelineColorBlendAttachmentState colour_blend_attachment = {};
    colour_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colour_blend_attachment.blendEnable = VK_FALSE;
    colour_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colour_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colour_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colour_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colour_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colour_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    //need more studying
    VkPipelineColorBlendStateCreateInfo colour_blend_create_info{};
    colour_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colour_blend_create_info.pNext = nullptr;
    colour_blend_create_info.flags = 0;

    colour_blend_create_info.logicOpEnable = VK_FALSE;
    colour_blend_create_info.logicOp = VK_LOGIC_OP_COPY; // Optional
    colour_blend_create_info.attachmentCount = 1;
    colour_blend_create_info.pAttachments = &colour_blend_attachment;
    colour_blend_create_info.blendConstants[0] = 0.0f; // Optional
    colour_blend_create_info.blendConstants[1] = 0.0f; // Optional
    colour_blend_create_info.blendConstants[2] = 0.0f; // Optional
    colour_blend_create_info.blendConstants[3] = 0.0f; // Optional
    //</f> /Colour Blending

    //<f> Pipeline Layout
    //empty, for now
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    pipeline_layout_create_info.pNext = nullptr;
    pipeline_layout_create_info.flags = 0;

    pipeline_layout_create_info.setLayoutCount = 1;
    pipeline_layout_create_info.pSetLayouts = &m_descriptor_set_layout;

    if(vkCreatePipelineLayout(m_logical_device, &pipeline_layout_create_info, nullptr, &m_pipeline_layout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout");
    //</f> /Pipeline Layout

    //<f> Pipeline proper
    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
    graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.pNext = nullptr;
    graphics_pipeline_create_info.flags = 0;

    graphics_pipeline_create_info.stageCount = 2;//shader stages
    graphics_pipeline_create_info.pStages = shader_stages.data();

    graphics_pipeline_create_info.pVertexInputState = &vertex_input_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
    graphics_pipeline_create_info.pViewportState = &viewport_state_create_info;
    graphics_pipeline_create_info.pRasterizationState = &rasterizer_create_info;
    graphics_pipeline_create_info.pMultisampleState = &multisampling_create_info;
    graphics_pipeline_create_info.pDepthStencilState = nullptr; // not in use
    graphics_pipeline_create_info.pColorBlendState = &colour_blend_create_info;
    graphics_pipeline_create_info.pDynamicState = nullptr; // not in use

    graphics_pipeline_create_info.layout = m_pipeline_layout;

    graphics_pipeline_create_info.renderPass = m_render_pass;
    graphics_pipeline_create_info.subpass = 0;//subpass index

    //for use when we need to create another pipeline similar to the one being referenced
    //VK_PIPELINE_CREATE_DERIVATIVE_BIT must be set in 'flags' struct field
    graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    graphics_pipeline_create_info.basePipelineIndex = -1;

    if( vkCreateGraphicsPipelines(m_logical_device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &m_graphics_pipeline) != VK_SUCCESS )
        throw std::runtime_error("Failed to create graphics pipeline");
    //</f> /Pipeline proper

    //<f> Cleanup
    //as the modules are only needed inside the function we can destroy them at the end
    vkDestroyShaderModule(m_logical_device, vertex_module, nullptr);
    vkDestroyShaderModule(m_logical_device, fragment_module, nullptr);
    //</f> /Cleanup
}

VkShaderModule VulkanContext::CreateShaderModule(const std::vector<char>& shader_code)
{
    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    //needed or driver screams at me
    shader_module_create_info.pNext = nullptr;
    shader_module_create_info.flags = 0;

    shader_module_create_info.codeSize = shader_code.size();
    //why the hell is it a uint32_t* and not a char*
    shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(shader_code.data());

    VkShaderModule module{};

    if( vkCreateShaderModule(m_logical_device, &shader_module_create_info, nullptr, &module) != VK_SUCCESS)
        throw std::runtime_error("Failed to create shader module");

    return module;
}

void VulkanContext::CreateRenderPass()
{
    //<f> Attachment
    VkAttachmentDescription attachment_description{};

    attachment_description.format = m_swap_chain_format;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;//no multisampling

    //what to do with the data BEFORE rendering
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//clear to constant at start

    //what to do with the data AFTER rendering
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;//store value for latter use

    //no stencyl or layout, for now
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    //</f> /Attachment

    //<f> Subpasses
    VkAttachmentReference attachment_reference{};
    attachment_reference.attachment = 0;//for now, we only have 1 VkAttachmentDescription so we use the index 0
    attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//best result for colour buffer

    VkSubpassDescription subpass_description{};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;//this is a graphics subpass

    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &attachment_reference;

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;//the implicit initial subpass
    subpass_dependency.dstSubpass = 0;//first explicit subpass (index 0). MUST BE HIGHER THAN srcSubpass, ALWAYS

    //wait for bit read and write at output bit stage
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //</f> /Subpasses

    //<f> RenderPass
    VkRenderPassCreateInfo render_pass_Create_info{};
    render_pass_Create_info.sType =VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    render_pass_Create_info.pNext = nullptr;
    render_pass_Create_info.flags = 0;

    render_pass_Create_info.attachmentCount = 1;
    render_pass_Create_info.pAttachments = &attachment_description;

    render_pass_Create_info.subpassCount = 1;
    render_pass_Create_info.pSubpasses = &subpass_description;

    render_pass_Create_info.dependencyCount = 1;
    render_pass_Create_info.pDependencies = &subpass_dependency;

    if( vkCreateRenderPass(m_logical_device, &render_pass_Create_info, nullptr, &m_render_pass) != VK_SUCCESS)
        throw std::runtime_error("Failed to create rendre pass");
    //</f> /RenderPass

}

void VulkanContext::CreateFramebuffers()
{
    auto vector_size{m_swap_chain_image_views.size()};
    m_swap_chain_framebuffers.resize(vector_size);

    //create a framebuffer for every image view
    for(auto i{0}; i<vector_size; ++i)
    {
        auto image_view{m_swap_chain_image_views[i]};

        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.pNext = nullptr;
        framebuffer_create_info.flags = 0;

        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments = &image_view;

        framebuffer_create_info.renderPass = m_render_pass;
        framebuffer_create_info.layers = 1;
        framebuffer_create_info.width = m_swap_chain_extent.width;
        framebuffer_create_info.height = m_swap_chain_extent.height;

        if( vkCreateFramebuffer(m_logical_device, &framebuffer_create_info, nullptr, &m_swap_chain_framebuffers[i]) != VK_SUCCESS )
            throw std::runtime_error("Failed to create framebuffer");
    }
}

void VulkanContext::CreateBuffer(VkDeviceSize buffer_size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags property_flags, VkBuffer& buffer, VkDeviceMemory& buffer_memory)
{
    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = nullptr;
    buffer_create_info.flags = 0;

    buffer_create_info.size = buffer_size;
    buffer_create_info.usage = usage_flags;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if( vkCreateBuffer(m_logical_device, &buffer_create_info, nullptr, &buffer) != VK_SUCCESS )
        throw std::runtime_error("Failed to create vertex buffer");

    //<f> Memory allocation
    VkMemoryRequirements buffer_memory_requirements;
    vkGetBufferMemoryRequirements(m_logical_device, buffer, &buffer_memory_requirements);

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;

    allocate_info.allocationSize = buffer_memory_requirements.size;
    allocate_info.memoryTypeIndex = FindMemoryType(buffer_memory_requirements.memoryTypeBits, property_flags);

    if( vkAllocateMemory(m_logical_device, &allocate_info, nullptr, &buffer_memory) != VK_SUCCESS )
        throw std::runtime_error("Failed to allocate vertex buffer memory");

    vkBindBufferMemory(m_logical_device, buffer, buffer_memory, 0);
    //</f> /Memory allocation
}

void VulkanContext::CreateVertexBuffer()
{
    VkDeviceSize buffer_size {sizeof(vertices[0]) * vertices.size()};

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    //Source of memory transfer
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                staging_buffer, staging_buffer_memory);

    //<f> Mapping
    void* data;
    vkMapMemory(m_logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
    vkUnmapMemory(m_logical_device, staging_buffer_memory);
    //</f> /Mapping

    //destination of memory transfer and vertex buffer and with local device memory
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_vertex_buffer, m_vertex_buffer_memory);

    CopyBuffer(staging_buffer, m_vertex_buffer, buffer_size);

    //free staging vars
    vkDestroyBuffer(m_logical_device, staging_buffer, nullptr);
    vkFreeMemory(m_logical_device, staging_buffer_memory, nullptr);
}

void VulkanContext::CreateIndexBuffer()
{
    VkDeviceSize buffer_size{sizeof(vertex_indices[0]) * vertex_indices.size()};

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    //Source of memory transfer
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                staging_buffer, staging_buffer_memory);

    //<f> Mapping
    void* data;
    vkMapMemory(m_logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertex_indices.data(), static_cast<size_t>(buffer_size));
    vkUnmapMemory(m_logical_device, staging_buffer_memory);
    //</f> /Mapping

    //destination of memory transfer and vertex buffer and with local device memory
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_index_buffer, m_index_buffer_memory);

    CopyBuffer(staging_buffer, m_index_buffer, buffer_size);

    //free staging vars
    vkDestroyBuffer(m_logical_device, staging_buffer, nullptr);
    vkFreeMemory(m_logical_device, staging_buffer_memory, nullptr);
}

void VulkanContext::CreateUniformBuffer()
{
    VkDeviceSize buffer_size{sizeof(UniformBufferObject)};

    //Source of memory transfer
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_uniform_buffer, m_uniform_buffer_memory);
}

void VulkanContext::CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
    //memory tranfer operations are made using command buffers
    VkCommandBufferAllocateInfo allocate_cmd_info{};
    allocate_cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_cmd_info.pNext = nullptr;

    allocate_cmd_info.commandPool = m_command_pool;//use "main" command pool
    allocate_cmd_info.commandBufferCount = 1;//one set of commands

    //create command buffer
    VkCommandBuffer cmd_buffer;
    vkAllocateCommandBuffers(m_logical_device, &allocate_cmd_info, &cmd_buffer);//create cmd buffer

    //record command buffer
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd_buffer, &begin_info);

    //operations
    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;

    vkCmdCopyBuffer(cmd_buffer, src, dst, 1, &copy_region);

    //end record
    vkEndCommandBuffer(cmd_buffer);

    //submit cmd_buffer
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buffer;

    vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);//graphics queue can perform transfer operations
    vkQueueWaitIdle(m_graphics_queue);

    vkFreeCommandBuffers(m_logical_device, m_command_pool, 1, &cmd_buffer);
}

uint32_t VulkanContext::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags flags)
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

void VulkanContext::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding descriptor_binding{};

    descriptor_binding.binding = 0;//binding id in the shader
    descriptor_binding.descriptorCount = 1;
    descriptor_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;//we are working with uniform shader field
    descriptor_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;//we are working in the vertex shader

    descriptor_binding.pImmutableSamplers = nullptr;//for now

    //create set layout
    VkDescriptorSetLayoutCreateInfo descriptor_set_creat_info{};
    descriptor_set_creat_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_creat_info.pNext = nullptr;
    descriptor_set_creat_info.flags = 0;

    descriptor_set_creat_info.bindingCount = 1;
    descriptor_set_creat_info.pBindings = &descriptor_binding;

    if( vkCreateDescriptorSetLayout(m_logical_device, &descriptor_set_creat_info, nullptr, &m_descriptor_set_layout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor set layout");
}

void VulkanContext::CreateDescriptorPool()
{
    //pool size
    VkDescriptorPoolSize pool_size{};
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = 1;//UniformBufferObject

    //
    VkDescriptorPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.pNext = nullptr;
    pool_create_info.flags = 0;

    pool_create_info.poolSizeCount = 1;
    pool_create_info.pPoolSizes = &pool_size;
    pool_create_info.maxSets = 1;//max allocated sets

    if( vkCreateDescriptorPool(m_logical_device, &pool_create_info, nullptr, &m_descriptor_pool) != VK_SUCCESS )
        throw std::runtime_error("Failed to create descriptor pool");
}

void VulkanContext::CreateDescriptorSet()
{
    std::vector<VkDescriptorSetLayout> layouts{m_descriptor_set_layout};

    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;

    allocate_info.descriptorPool = m_descriptor_pool;
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = layouts.data();

    if( vkAllocateDescriptorSets(m_logical_device, &allocate_info, &m_descriptor_set) != VK_SUCCESS )
        throw std::runtime_error("Failed to allocate descriptor set");

    //config descriptor
    VkDescriptorBufferInfo config{};
    config.buffer = m_uniform_buffer;
    config.offset = 0;
    config.range = sizeof(UniformBufferObject);

    //update set
    VkWriteDescriptorSet write_descriptor{};
    write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor.pNext = nullptr;

    write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor.dstBinding = 0;//shader binding index
    write_descriptor.dstSet = m_descriptor_set;//set to write to
    write_descriptor.dstArrayElement = 0;//no array == index 0

    write_descriptor.descriptorCount = 1;
    write_descriptor.pBufferInfo = &config;

    vkUpdateDescriptorSets(m_logical_device, 1, &write_descriptor, 0, nullptr);//write to set or copy to set

}
//</f> /Rendering

//<f> Commands
void VulkanContext::CreateCommandPool()
{
    QueueFamiliesIndices indices{FindQueueFamiliesIndeces(m_physical_device)};

    //<f> Graphics
    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = nullptr;
    command_pool_create_info.flags = 0;

    command_pool_create_info.queueFamilyIndex = indices.graphics_family;

    if( vkCreateCommandPool(m_logical_device, &command_pool_create_info, nullptr, &m_command_pool) != VK_SUCCESS )
        throw std::runtime_error("Failed to create command pool");
    //</f> /Graphics
}

void VulkanContext::CreateCommandBuffers()
{
    auto vector_size{m_swap_chain_framebuffers.size()};
    m_command_buffers.resize(vector_size);

    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;

    allocate_info.commandPool = m_command_pool;
    allocate_info.commandBufferCount = static_cast<uint32_t>(vector_size);
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if( vkAllocateCommandBuffers(m_logical_device, &allocate_info, m_command_buffers.data()) != VK_SUCCESS )
        throw std::runtime_error("Failed to allocate command buffers");

    for(auto i{0}; i<vector_size; ++i)
    {
        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.pNext = nullptr;
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;//command buffer can be resubmitted while it is also already pending execution

        vkBeginCommandBuffer(m_command_buffers[i], &command_buffer_begin_info);

        //<f> RenderPass
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.pNext = nullptr;

        render_pass_begin_info.renderPass = m_render_pass;
        render_pass_begin_info.framebuffer = m_swap_chain_framebuffers[i];

        //render area
        render_pass_begin_info.renderArea.offset = {0,0};
        render_pass_begin_info.renderArea.extent = m_swap_chain_extent;

        //colour clear value
        VkClearValue clear_value { 0.f, 0.f, 0.f, 1.f };//black 100% opacity
        render_pass_begin_info.clearValueCount = 1;
        render_pass_begin_info.pClearValues = &clear_value;

        vkCmdBeginRenderPass(m_command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

        //vertex buffer input
        std::vector<VkBuffer> vertex_buffers{m_vertex_buffer};
        std::vector<VkDeviceSize> offsets{VkDeviceSize{0}};

        vkCmdBindVertexBuffers(m_command_buffers[i], 0, 1, vertex_buffers.data(), offsets.data());
        vkCmdBindIndexBuffer(m_command_buffers[i], m_index_buffer, 0, VK_INDEX_TYPE_UINT16);//VK_INDEX_TYPE_UINT16 because vertex_indices uses uint16_t
        vkCmdBindDescriptorSets(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, &m_descriptor_set, 0, nullptr);

        // vkCmdDraw(m_command_buffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
        vkCmdDrawIndexed(m_command_buffers[i], static_cast<uint32_t>(vertex_indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(m_command_buffers[i]);
        //</f> /RenderPass

        if( vkEndCommandBuffer(m_command_buffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to record command buffer");
    }//for loop
}

void VulkanContext::CreateSemaphores()
{
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = nullptr;
    semaphore_create_info.flags = 0;

    if( vkCreateSemaphore(m_logical_device, &semaphore_create_info, nullptr, &m_image_available_semaphore) != VK_SUCCESS ||
        vkCreateSemaphore(m_logical_device, &semaphore_create_info, nullptr, &m_render_finished_semaphore) != VK_SUCCESS )
        throw std::runtime_error("Failed to create semaphores");
}
//</f> /Commands
//</f> /Private Init
