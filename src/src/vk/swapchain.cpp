#include "swapchain.hpp"
#include <utility>
#include <algorithm>
#include "vulkan_utils.hpp"
#include "vulkan_context.hpp"
#include "commandbuffer.hpp"

namespace vk
{

//<f> Constructors & operator=
Swapchain::Swapchain(VulkanContext* context, CommandBuffer* commandbuffer): m_context{context}, m_commandbuffer{commandbuffer}
{

}

Swapchain::~Swapchain() noexcept
{

}

Swapchain::Swapchain(Swapchain&& other) noexcept
{

}

Swapchain& Swapchain::operator=(Swapchain&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
void Swapchain::Init()
{
    CreateSwapchain();
    CreateSwapchainImageViews();
    CreateDepthResources();

    m_init_ok = true;
}

void Swapchain::Cleanup()
{
    if(!m_init_ok)
        throw std::runtime_error("Cannot cleanup swapchain has it was not initialized!");

    vkDestroyImageView(*m_context->LogicalDevice(), m_depth_image_view, nullptr);
    m_context->MemoryManager()->DestroyImageBuffer(&m_depth_image);

    for(auto i{0}; i<m_image_views.size(); ++i)
        vkDestroyImageView(*m_context->LogicalDevice(), m_image_views[i], nullptr);

    m_image_views.clear();
    m_images.clear();
    vkDestroySwapchainKHR(*m_context->LogicalDevice(), m_swapchain, nullptr);
}
//</f> /Methods

//<f> Private Methods
void Swapchain::CreateSwapchain()
{
    auto swapchain_struct{m_context->SwapchainSupportStruct()};

    VkSurfaceFormatKHR format = SelectSwapChainFormat(swapchain_struct);
    auto presentation_mode{SelectSwapChainPresentationMode(swapchain_struct)};
    VkExtent2D extent = SelectSwapChainExtent(swapchain_struct);
    m_format = format.format;
    m_extent = extent;

    //set min number of images in the swap chain
    uint32_t min_image_count{ swapchain_struct.capabilities.minImageCount + 1 };

    if(swapchain_struct.capabilities.maxImageCount > 0)//0 = infinite (memory limits apply)
        min_image_count = std::min(min_image_count, swapchain_struct.capabilities.maxImageCount);

    //create said swap chain
    VkSwapchainCreateInfoKHR swapchain_create_info = vk::SwapchainCreateInfoKHR();
    swapchain_create_info.surface = *m_context->Surface();

    swapchain_create_info.imageArrayLayers = 1;//default
    swapchain_create_info.imageExtent = extent;
    swapchain_create_info.imageFormat = format.format;
    swapchain_create_info.imageColorSpace = format.colorSpace;
    swapchain_create_info.minImageCount = min_image_count;
    swapchain_create_info.presentMode = presentation_mode;
    swapchain_create_info.clipped = VK_TRUE;//clip obscured pixel
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;//we render directly on it

    //set how queues will handle the swap chain images
    auto queue_indices{m_context->QueueFamiliesIndicesStruct()};
    auto uint32_indices{queue_indices.ToVectorUInt32()};

    if(queue_indices.graphics_family != queue_indices.presentation_family)//not the same queue
    {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//can be used by multiple families at he same time (need at least 2 families)
        swapchain_create_info.queueFamilyIndexCount = static_cast<uint32_t>(uint32_indices.size());
        swapchain_create_info.pQueueFamilyIndices = uint32_indices.data();//who will share usage
    }
    else//same queue
    {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//owner by one family at a type, need explicit ownership change
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
    }

    swapchain_create_info.preTransform = swapchain_struct.capabilities.currentTransform;//transformation to apply in the chain. Current mean no transformation
    // swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;//alph blend with other windows
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//alph blend with other windows
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;//used when we need to recreate the chain

    //finally we create the damn swap chain
    if(vkCreateSwapchainKHR(*m_context->LogicalDevice(), &swapchain_create_info, nullptr, &m_swapchain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swap chain");

    //get swap chain images
    uint32_t swapchain_image_count{0};
    //vulkan can create more than the number we want, so we need to ask for how many are there
    vkGetSwapchainImagesKHR(*m_context->LogicalDevice(), m_swapchain, &swapchain_image_count, nullptr);

    m_images.resize(swapchain_image_count);
    vkGetSwapchainImagesKHR(*m_context->LogicalDevice(), m_swapchain, &swapchain_image_count, m_images.data());
}

void Swapchain::CreateSwapchainImageViews()
{
    m_image_views.resize(m_images.size());

    //create each image view, based on an image
    for(auto i{0}; i<m_images.size(); ++i)
    {
        m_image_views[i] = m_context->CreateImageView(m_images[i], m_format, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void Swapchain::CreateDepthResources()
{
    //valid depth formats VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
    m_depth_format =  m_context->FindCapableFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
    VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    m_context->MemoryManager()->RequestImageBuffer(m_extent.width, m_extent.height, m_depth_format, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_depth_image);

    m_depth_image_view = m_context->CreateImageView(m_depth_image.image, m_depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);

    m_commandbuffer->ChangeImageLayout(m_depth_image.image, m_depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkSurfaceFormatKHR Swapchain::SelectSwapChainFormat(const SwapchainSupport& swap_chain_struct)
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

VkPresentModeKHR Swapchain::SelectSwapChainPresentationMode(const SwapchainSupport& swap_chain_struct)
{
    for(const auto& mode : swap_chain_struct.presentation_modes)
    {
        if(mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return mode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;//guaranteed to be available
}

VkExtent2D Swapchain::SelectSwapChainExtent(const SwapchainSupport& swap_chain_struct)
{
    auto capabilities = swap_chain_struct.capabilities;

    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;

    int w{0}, h{0};
    SDL_GetWindowSize(m_context->Window(), &w,&h);
    VkExtent2D extent{static_cast<uint32_t>(w), static_cast<uint32_t>(h)};

    //clamp width and height to min and max allowed size
    extent.width = std::max( capabilities.minImageExtent.width, std::min( capabilities.maxImageExtent.width, extent.width ) );
    extent.height = std::max( capabilities.minImageExtent.height, std::min( capabilities.maxImageExtent.height, extent.height ) );

    return extent;
}
//</f> /Private Methods

}//namespace vk
