#include "renderpass.hpp"
#include <utility>
#include "vulkan_utils.hpp"
#include "vulkan_context.hpp"
#include "swapchain.hpp"

namespace vk
{

//<f> Constructors & operator=
Renderpass::Renderpass(VulkanContext* context): m_context(context)
{

}

Renderpass::~Renderpass() noexcept
{

}

Renderpass::Renderpass(const Renderpass& other)
{

}

Renderpass::Renderpass(Renderpass&& other) noexcept
{

}

Renderpass& Renderpass::operator=(const Renderpass& other)
{
    if(this != &other)//not same ref
    {
        auto tmp(other);
        *this = std::move(tmp);
    }

    return *this;
}

Renderpass& Renderpass::operator=(Renderpass&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
void Renderpass::Cleanup()
{
    vkDestroyRenderPass(*m_context->LogicalDevice(), m_renderpass, nullptr);
}

void Renderpass::Init(Swapchain* swapchain)
{
    //<f> Attachments
    //<f> Colour
    VkAttachmentDescription colour_attachment{};

    colour_attachment.format = *swapchain->Format();
    colour_attachment.samples = VK_SAMPLE_COUNT_1_BIT;//no multisampling

    //what to do with the data BEFORE rendering
    colour_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//clear to constant at start

    //what to do with the data AFTER rendering
    colour_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;//store value for latter use

    //no stencyl or layout, for now
    colour_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colour_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colour_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colour_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    //</f> /Colour

    //<f> Depth
    VkAttachmentDescription depth_attachment{};

    depth_attachment.format = *swapchain->DepthFormat();//same as depth image
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;//no multisampling

    //what to do with the data BEFORE rendering
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//clear to constant at start
    //what to do with the data AFTER rendering
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//value not needed after rendering

    //no stencyl or layout, for now
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //</f> /Depth
    //</f> /Attachments

    //<f> Subpasses
    VkAttachmentReference colour_attachment_reference{};
    colour_attachment_reference.attachment = 0;//this is the first VkAttachmentDescription so we use the index 0
    colour_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//best result for colour buffer

    VkAttachmentReference depth_attachment_reference{};
    depth_attachment_reference.attachment = 1;
    depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


    VkSubpassDescription subpass_description{};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;//this is a graphics subpass

    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &colour_attachment_reference;
    subpass_description.pDepthStencilAttachment = &depth_attachment_reference;//subpas only uses one depth/stencil attachment so we dont need to set is number

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
    std::vector<VkAttachmentDescription> descriptions{colour_attachment, depth_attachment};

    VkRenderPassCreateInfo render_pass_create_info{vk::RenderPassCreateInfo()};

    render_pass_create_info.attachmentCount = static_cast<uint32_t>(descriptions.size());
    render_pass_create_info.pAttachments = descriptions.data();

    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_description;

    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;

    if( vkCreateRenderPass(*m_context->LogicalDevice(), &render_pass_create_info, nullptr, &m_renderpass) != VK_SUCCESS)
        throw std::runtime_error("Failed to create rendre pass");
    //</f> /RenderPass
}
//</f> /Methods

}//namespace vk
