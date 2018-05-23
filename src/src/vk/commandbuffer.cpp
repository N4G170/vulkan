#include "commandbuffer.hpp"
#include <utility>
#include "vulkan_utils.hpp"
#include "vulkan_context.hpp"
#include "framebuffer.hpp"
#include "swapchain.hpp"
#include "renderpass.hpp"
#include "pipelines.hpp"
#include <iostream>

namespace vk
{

//<f> Constructors & operator=
CommandBuffer::CommandBuffer(VulkanContext* context): m_context{context}
{

}

CommandBuffer::~CommandBuffer() noexcept
{

}

CommandBuffer::CommandBuffer(const CommandBuffer& other)
{

}

CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
{

}

CommandBuffer& CommandBuffer::operator=(const CommandBuffer& other)
{
    if(this != &other)//not same ref
    {
        auto tmp(other);
        *this = std::move(tmp);
    }

    return *this;
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
void CommandBuffer::Cleanup()
{
    vkDestroySemaphore(*m_context->LogicalDevice(), m_image_available_semaphore, nullptr);
    vkDestroySemaphore(*m_context->LogicalDevice(), m_render_finished_semaphore, nullptr);

    vkFreeCommandBuffers(*m_context->LogicalDevice(), m_command_pool, m_command_buffers.size(), m_command_buffers.data());
    vkDestroyCommandPool(*m_context->LogicalDevice(), m_command_pool, nullptr);
}

void CommandBuffer::Init()
{
    CreateCommandPool();
    CreateSemaphores();
    CreateQueues();
}

void CommandBuffer::InitMainCommanfBuffers(Framebuffer * framebuffer)
{
    // vkFreeCommandBuffers(*m_context->LogicalDevice(), m_command_pool, m_command_buffers.size(), m_command_buffers.data());
    CreateCommandBuffers(framebuffer);
}

VkCommandBuffer CommandBuffer::BeginCommand()
{
    VkCommandBufferAllocateInfo allocate_info{vk::CommandBufferAllocateInfo()};

    allocate_info.commandPool = m_command_pool;
    allocate_info.commandBufferCount = 1;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer cmd;
    if( vkAllocateCommandBuffers(*m_context->LogicalDevice(), &allocate_info, &cmd) != VK_SUCCESS )
        throw std::runtime_error("Failed to allocate command buffers");

    //begin record
    VkCommandBufferBeginInfo begin_info{vk::CommandBufferBeginInfo()};
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &begin_info);

    return cmd;
}

void CommandBuffer::SubmitCommand(VkCommandBuffer cmd)
{
    if( vkEndCommandBuffer(cmd) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer");

    VkSubmitInfo submit_info = {vk::SubmitInfo()};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd;

    vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphics_queue);

    vkFreeCommandBuffers(*m_context->LogicalDevice(), m_command_pool, 1, &cmd);
}

void CommandBuffer::RegisterModelsData(Swapchain* swapchain, Renderpass* renderpass, Framebuffer* framebuffer, Pipelines* pipelines, std::vector<VulkanModelData> model_data)
{
    for(auto i{0}; i<m_command_buffers.size(); ++i)
    {
        VkCommandBufferBeginInfo command_buffer_begin_info{vk::CommandBufferBeginInfo()};
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;//command buffer can be resubmitted while it is also already pending execution

        vkBeginCommandBuffer(m_command_buffers[i], &command_buffer_begin_info);

        //<f> RenderPass
        VkRenderPassBeginInfo render_pass_begin_info{vk::RenderPassBeginInfo()};

        render_pass_begin_info.renderPass = *renderpass->RenderPass();
        render_pass_begin_info.framebuffer = (*framebuffer->Framebuffers())[i];

        //render area
        render_pass_begin_info.renderArea.offset = {0,0};
        render_pass_begin_info.renderArea.extent = *swapchain->Extent();

        //colour clear value
        std::vector<VkClearValue> clear_values(2);
        clear_values[0].color = { {0.f, 0.f, 0.f, 1.f} };//black 100% opacity
        clear_values[1].depthStencil = {1.f, 0};

        render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_begin_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(m_command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        //vertex buffer input
        for(auto& data : model_data)
        {
            vkCmdBindPipeline(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelines->Pipeline(data.pipeline_type));

            //uniform descriptor set
            vkCmdBindDescriptorSets(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelines->GraphicsLayout(), 0, 1, data.uniform_descriptor_set, 0, nullptr);

            for(auto& mesh : data.meshes_data)
            {
                std::vector<VkBuffer> vertex_buffers{mesh.vertex_buffer->buffer};
                std::vector<VkDeviceSize> offsets{VkDeviceSize{0}};
                //vertex
                vkCmdBindVertexBuffers(m_command_buffers[i], 0, 1, vertex_buffers.data(), offsets.data());
                //index
                vkCmdBindIndexBuffer(m_command_buffers[i], mesh.index_buffer->buffer, 0, VK_INDEX_TYPE_UINT32);//VK_INDEX_TYPE_UINT16 because vertex_indices uses uint16_t
                //sampler descriptor set
                vkCmdBindDescriptorSets(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelines->GraphicsLayout(), 1, 1, mesh.sampler_descriptor_set, 0, nullptr);

                //register draw
                vkCmdDrawIndexed(m_command_buffers[i], mesh.index_vector_size, 1, 0, 0, 0);
            }
        }
        vkCmdEndRenderPass(m_command_buffers[i]);
        //</f> /RenderPass

        if( vkEndCommandBuffer(m_command_buffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to record command buffer");
    }//for loop
}

void CommandBuffer::DrawFrame(Swapchain* swapchain, bool* recreate)
{
    //get image index from swap chain
    uint32_t image_index{0};
    auto acquire_result{vkAcquireNextImageKHR(*m_context->LogicalDevice(), *swapchain->SwapchainKHR(), std::numeric_limits<uint64_t>::max(), m_image_available_semaphore, VK_NULL_HANDLE, &image_index)};

    if(acquire_result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        *recreate = true;
        return;
    }
    else if(acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR)//only these return an image
        throw std::runtime_error("Failed to acquire swap chain image");

    //<f> Submit command
    VkSubmitInfo command_submit_info{vk::SubmitInfo()};

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
    VkPresentInfoKHR presentation_info{vk::PresentInfoKHR()};

    presentation_info.waitSemaphoreCount = 1;
    presentation_info.pWaitSemaphores = signal_semaphores.data();//wait for these semaphores before presentation

    std::vector<VkSwapchainKHR> swap_chains{*swapchain->SwapchainKHR()};
    presentation_info.swapchainCount = 1;
    presentation_info.pSwapchains = swap_chains.data();
    presentation_info.pImageIndices = &image_index;

    auto queue_result{vkQueuePresentKHR(m_graphics_queue, &presentation_info)};

    if (queue_result == VK_ERROR_OUT_OF_DATE_KHR || queue_result == VK_SUBOPTIMAL_KHR)
    {
        *recreate = true;
    }
    else if (queue_result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    vkQueueWaitIdle(m_presentation_queue);
    //</f> /Presentation
}

void CommandBuffer::CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size, VkDeviceSize src_offset, VkDeviceSize dst_offset)
{
    //create command buffer
    VkCommandBuffer cmd_buffer{BeginCommand()};

    //operations
    VkBufferCopy copy_region{};
    copy_region.srcOffset = src_offset;
    copy_region.dstOffset = dst_offset;
    copy_region.size = size;

    vkCmdCopyBuffer(cmd_buffer, src, dst, 1, &copy_region);

    //end and submit
    SubmitCommand(cmd_buffer);
}

void CommandBuffer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    auto cmd{BeginCommand()};

    VkBufferImageCopy copy_region{};
    copy_region.bufferOffset = 0;

    //really bad var name
    copy_region.bufferRowLength = 0;// \see https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkBufferImageCopy.html
    copy_region.bufferImageHeight = 0;// \see https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkBufferImageCopy.html

    //parts of image to copy
    copy_region.imageOffset = {0,0,0};//no offset
    copy_region.imageExtent ={ width, height, 1 };//it is a 3d extent of a 2d extent

    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;//copy colour
    copy_region.imageSubresource.baseArrayLayer = 0;//starting layer to copy from
    copy_region.imageSubresource.mipLevel = 0;//mip level to copy from
    copy_region.imageSubresource.layerCount = 1;//number of layers to copy

    vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    SubmitCommand(cmd);
}

void CommandBuffer::CopyBufferToCubemap(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    auto cmd{BeginCommand()};

    VkBufferImageCopy copy_region{};
    copy_region.bufferOffset = 0;

    //really bad var name
    copy_region.bufferRowLength = 0;// \see https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkBufferImageCopy.html
    copy_region.bufferImageHeight = 0;// \see https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkBufferImageCopy.html

    //parts of image to copy
    copy_region.imageOffset = {0,0,0};//no offset
    copy_region.imageExtent ={ width, height, 1 };//it is a 3d extent of a 2d extent

    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;//copy colour
    copy_region.imageSubresource.baseArrayLayer = 0;//starting layer to copy from
    copy_region.imageSubresource.mipLevel = 0;//mip level to copy from
    copy_region.imageSubresource.layerCount = 6;//number of layers to copy

    vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    SubmitCommand(cmd);
}

void CommandBuffer::ChangeImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
{
    auto cmd{BeginCommand()};

    VkImageMemoryBarrier barrier{vk::ImageMemoryBarrier()};

    barrier.image = image;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    //set if we are changing queue families ownership (ex: if using VK_SHARING_MODE_EXCLUSIVE)
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (DepthFormatHasStencil(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    // barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    //<f> AccessMasks and Stages
    //Move this to external function?
    VkPipelineStageFlags src_stage;
    VkPipelineStageFlags dst_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("Unsupported layout transition!");
    }
    //</f> /AccessMasks and Stages

    vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    SubmitCommand(cmd);
}

void CommandBuffer::ChangeCubemapLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
{
    auto cmd{BeginCommand()};

    VkImageMemoryBarrier barrier{vk::ImageMemoryBarrier()};

    barrier.image = image;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    //set if we are changing queue families ownership (ex: if using VK_SHARING_MODE_EXCLUSIVE)
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (DepthFormatHasStencil(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    // barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 6;

    //<f> AccessMasks and Stages
    //Move this to external function?
    VkPipelineStageFlags src_stage;
    VkPipelineStageFlags dst_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("Unsupported layout transition!");
    }
    //</f> /AccessMasks and Stages

    vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    SubmitCommand(cmd);
}

//</f> /Methods

//<f> Private Methods
void CommandBuffer::CreateCommandPool()
{
    QueueFamiliesIndices indices{m_context->QueueFamiliesIndicesStruct()};

    //<f> Graphics
    VkCommandPoolCreateInfo command_pool_create_info{vk::CommandPoolCreateInfo()};

    command_pool_create_info.queueFamilyIndex = indices.graphics_family;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;//this allows the creation of a new commandbuffer render on top of the old

    if( vkCreateCommandPool(*m_context->LogicalDevice(), &command_pool_create_info, nullptr, &m_command_pool) != VK_SUCCESS )
        throw std::runtime_error("Failed to create command pool");
    //</f> /Graphics
}

void CommandBuffer::CreateCommandBuffers(Framebuffer* framebuffer)
{
    auto framebuffers_count{framebuffer->FramebuffersCount()};
    m_command_buffers.resize(framebuffers_count);

    VkCommandBufferAllocateInfo allocate_info{vk::CommandBufferAllocateInfo()};

    allocate_info.commandPool = m_command_pool;
    allocate_info.commandBufferCount = framebuffers_count;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if( vkAllocateCommandBuffers(*m_context->LogicalDevice(), &allocate_info, m_command_buffers.data()) != VK_SUCCESS )
        throw std::runtime_error("Failed to allocate command buffers");
}

void CommandBuffer::CreateSemaphores()
{
    VkSemaphoreCreateInfo semaphore_create_info{vk::SemaphoreCreateInfo()};

    if( vkCreateSemaphore(*m_context->LogicalDevice(), &semaphore_create_info, nullptr, &m_image_available_semaphore) != VK_SUCCESS ||
        vkCreateSemaphore(*m_context->LogicalDevice(), &semaphore_create_info, nullptr, &m_render_finished_semaphore) != VK_SUCCESS )
        throw std::runtime_error("Failed to create semaphores");
}

void CommandBuffer::CreateQueues()
{
    auto queue_families_indices{ m_context->QueueFamiliesIndicesStruct() };
    vkGetDeviceQueue(*m_context->LogicalDevice(), queue_families_indices.graphics_family, 0, &m_graphics_queue);
    vkGetDeviceQueue(*m_context->LogicalDevice(), queue_families_indices.presentation_family, 0, &m_presentation_queue);
}
//</f> /Private Methods

}//namespace vk
