#include "framebuffer.hpp"
#include <utility>
#include "vulkan_utils.hpp"
#include "vulkan_context.hpp"
#include "renderpass.hpp"
#include "swapchain.hpp"

namespace vk
{

//<f> Constructors & operator=
Framebuffer::Framebuffer(VulkanContext* context): m_context{context}
{

}

Framebuffer::~Framebuffer() noexcept
{

}

Framebuffer::Framebuffer(const Framebuffer& other)
{

}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
{

}

Framebuffer& Framebuffer::operator=(const Framebuffer& other)
{
    if(this != &other)//not same ref
    {
        auto tmp(other);
        *this = std::move(tmp);
    }

    return *this;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
void Framebuffer::Cleanup()
{
    for(auto i{0}; i < m_framebuffers.size(); ++i)
        vkDestroyFramebuffer(*m_context->LogicalDevice(), m_framebuffers[i], nullptr);
}

void Framebuffer::Init(Swapchain* swapchain, Renderpass* renderpass)
{
    auto vector_size{ swapchain->ImageViewsCount() };
    m_framebuffers.resize(vector_size);

    //create a framebuffer for every image view
    for(auto i{0}; i<vector_size; ++i)
    {
        std::vector<VkImageView> attachements{(*swapchain->ImageViews())[i], *swapchain->DepthImageView()};

        VkFramebufferCreateInfo framebuffer_create_info{vk::FramebufferCreateInfo()};

        framebuffer_create_info.attachmentCount = static_cast<uint32_t>(attachements.size());
        framebuffer_create_info.pAttachments = attachements.data();

        framebuffer_create_info.renderPass = *renderpass->RenderPass();
        framebuffer_create_info.layers = 1;
        framebuffer_create_info.width = swapchain->Extent()->width;
        framebuffer_create_info.height = swapchain->Extent()->height;

        if( vkCreateFramebuffer(*m_context->LogicalDevice(), &framebuffer_create_info, nullptr, &m_framebuffers[i]) != VK_SUCCESS )
            throw std::runtime_error("Failed to create framebuffer");
    }
}
//</f> /Methods

}//namespace vk
