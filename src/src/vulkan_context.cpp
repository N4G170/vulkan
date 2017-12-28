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

#include "texture.hpp"


//<f> Constructors & operator=
VulkanContext::VulkanContext(SDL_Window* window):m_texture{this}, m_window{window}, m_vulkan_instance{}
{

}

VulkanContext::~VulkanContext() noexcept
{

}

VulkanContext::VulkanContext(const VulkanContext& other): m_texture{other.m_texture}
{

}

VulkanContext::VulkanContext(VulkanContext&& other) noexcept:m_texture{other.m_texture}
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

    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();

    CreateCommandPool();
    CreateDepthResources();
    CreateFramebuffers();

    m_texture.LoadTextureFile("data/img.png");
    m_texture.CreateImageView();
    m_texture.CreateSampler();

    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffer();
    CreateDescriptorPool();
    CreateDescriptorSet();//needs uniform buffer
    CreateCommandBuffers();
    CreateSemaphores();

}

void VulkanContext::Cleanup()
{
    //these 3 are inside clenupswapchain
    // vkDestroyImageView(m_logical_device, m_depth_image_view, nullptr);
    // vkDestroyImage(m_logical_device, m_depth_image, nullptr);
    // vkFreeMemory(m_logical_device, m_depth_image_memory, nullptr);

    CleanUpSwapChain();//needs the command_pool to exist

    m_texture.Cleanup();

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

#include <cmath>
void VulkanContext::UpdateUniformBuffer(float delta_time)
{
    static auto start_time{std::chrono::high_resolution_clock::now()};

    auto current_time{std::chrono::high_resolution_clock::now()};

    float time{ std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count() / 1000.f };

    UniformBufferObject obj{};

    static glm::vec3 mpos{0,0,0};
    static float st{1};
    mpos.z += st * delta_time * 5;
    // std::cout<<mpos.z<<std::endl;
    if(mpos.z > 8.5)
        st = -1;
    if(mpos.z < -0.5)
        st = 1;


    //rotate model 90 degrees * time on glm::vec3{1.f, 0.f, 0.f} axis
    glm::mat4 model = glm::rotate(glm::mat4{1.0f}, glm::radians(45.f) * time, glm::vec3{0.f, 1.f, 0.f});
    // model = glm::translate(model, mpos);

    obj.model = model;

    glm::vec3 front{0.f,0.f,3.f};
    glm::vec3 pos{0.f, 0.f, -3.f};
    // static float angle = 0.f;
    // static float px{0.f};
    // static float py{0.f};
    // static float xm{.1f};
    // static float ym{.1f};
    // if(px > 2)
    //     xm = -0.1f;
    // if(px < -2)
    //     xm = 0.1f;
    // if(py > 2)
    //     ym = -0.1f;
    // if(py < -2)
    //     ym = 0.1f;
    //
    // px += xm;
    // py += ym;
    // std::cout<<px*delta_time<<std::endl;
    // pos.x = px;
    // pos.y = py;
    // angle += 1;
    //camera position, point looking at, up vector(what side is up)
    // obj.view = glm::lookAt(glm::vec3{2.f, 2.f, 2.f}, glm::vec3{0.f, 0.f, 0.f}, glm::vec3{0.f, 1.f, 0.f});
    // obj.view = glm::lookAt(glm::vec3{0.f, 0.f, -2.f}, glm::vec3{0.5f, 0.f, 0.f}, glm::vec3{std::cos(glm::radians(angle)), -std::sin(glm::radians(angle)), 0.f});
    // obj.view = glm::lookAt(pos, pos + front, glm::vec3{0.f, -1.f, 0.f});
    obj.view = glm::lookAt(glm::vec3{0.f,0.f,-2.f}, glm::vec3{0.f,0.f,0.f}, glm::vec3{0.f, -1.f, 0.f});

    obj.projection = glm::perspective(glm::radians(45.f), m_swap_chain_extent.width / (float) m_swap_chain_extent.height, 0.1f, 10.0f);//TODO::Study this

    obj.projection[1][1] *= -1;//so we have y going up

    void* data;
    vkMapMemory(m_logical_device, m_uniform_buffer_memory, 0, sizeof(obj), 0, &data);
    memcpy(data, &obj, sizeof(obj));
    vkUnmapMemory(m_logical_device, m_uniform_buffer_memory);
}
//</f> /Methods

//<f> Public util methods
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

void VulkanContext::CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size, VkDeviceSize src_offset = 0, VkDeviceSize dst_offset = 0)
{
    //create command buffer
    VkCommandBuffer cmd_buffer{BeginSingleCommand()};

    //operations
    VkBufferCopy copy_region{};
    copy_region.srcOffset = src_offset;
    copy_region.dstOffset = dst_offset;
    copy_region.size = size;

    vkCmdCopyBuffer(cmd_buffer, src, dst, 1, &copy_region);

    //end and submit
    EndSubmitSingleCommand(cmd_buffer);
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

void VulkanContext::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory)
{
    //<f> Image Object
    VkImageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;

    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.extent.width = width;
    create_info.extent.height = height;
    create_info.extent.depth = 1;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;

    create_info.format = format;
    create_info.tiling = tiling;

    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.usage = usage;
    // create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;//we will copy to it(transfer) and use it in shaders to colour the mesh(sampled)
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//one queue at a time
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;

    if( vkCreateImage(m_logical_device, &create_info, nullptr, &image) != VK_SUCCESS )
        throw std::runtime_error("Failed to create texture image");
    //</f> /Image Object

    //<f> Memory
    VkMemoryRequirements memory_requirements{};
    vkGetImageMemoryRequirements(m_logical_device, image, &memory_requirements);

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = FindMemoryType(memory_requirements.memoryTypeBits, properties);

    if( vkAllocateMemory(m_logical_device, &allocate_info, nullptr, &image_memory) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate memory for texture image");
    //</f> /Memory

    vkBindImageMemory(m_logical_device, image, image_memory, 0);
}

void VulkanContext::ChangeImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
{
    auto cmd{BeginSingleCommand()};

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;

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

    EndSubmitSingleCommand(cmd);
}

void VulkanContext::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    auto cmd{BeginSingleCommand()};

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

    EndSubmitSingleCommand(cmd);
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

VkSampler VulkanContext::CreateSampler()
{
    VkSampler sampler{};
    VkSamplerCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;

    //magnify and minify interpolation type
    create_info.magFilter = VK_FILTER_LINEAR;
    create_info.minFilter = VK_FILTER_LINEAR;

    //address mode, ex: repeat, clamp ...
    create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;//X axis
    create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;//Y axis
    create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;//Z axis

    //anisotropic filter
    create_info.anisotropyEnable = VK_FALSE;
    create_info.maxAnisotropy = 1;//1 = disabled, to enable we need to request the feature during logical device creation, if the physical has the capability

    create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;//colour for the areas outside the image content

    //False -> texel coordinates normalized to [0,1)
    //True -> texel coordinates as [0, width) and [0, height)
    create_info.unnormalizedCoordinates = VK_FALSE;

    //comapre options
    create_info.compareEnable = VK_FALSE;//true, mainly used with shadow maps
    create_info.compareOp = VK_COMPARE_OP_ALWAYS;

    //mipmaps
    create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    create_info.mipLodBias = 0.f;
    create_info.minLod = 0.f;
    create_info.maxLod = 0.f;

    if( vkCreateSampler(m_logical_device, &create_info, nullptr, &sampler) != VK_SUCCESS )
        throw std::runtime_error("Failed to create texture sampler");

    return std::move(sampler);
}

//<f> Commands
VkCommandBuffer VulkanContext::BeginSingleCommand()
{
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;

    allocate_info.commandPool = m_command_pool;
    allocate_info.commandBufferCount = 1;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer cmd;
    if( vkAllocateCommandBuffers(m_logical_device, &allocate_info, &cmd) != VK_SUCCESS )
        throw std::runtime_error("Failed to allocate command buffers");

    //begin record
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &begin_info);

    return cmd;
}

void VulkanContext::EndSubmitSingleCommand(VkCommandBuffer cmd)
{
    if( vkEndCommandBuffer(cmd) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer");

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd;

    vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphics_queue);

    vkFreeCommandBuffers(m_logical_device, m_command_pool, 1, &cmd);
}
//</f> /Commands
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
    CreateDepthResources();
    CreateFramebuffers();
    CreateCommandBuffers();
}

void VulkanContext::CleanUpSwapChain()
{
    vkDestroyImageView(m_logical_device, m_depth_image_view, nullptr);
    vkDestroyImage(m_logical_device, m_depth_image, nullptr);
    vkFreeMemory(m_logical_device, m_depth_image_memory, nullptr);

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

void VulkanContext::CreateDepthResources()
{
    VkFormat depth_format{FindDepthCapableFormat()};

    CreateImage(m_swap_chain_extent.width, m_swap_chain_extent.height, depth_format, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depth_image, m_depth_image_memory);

    m_depth_image_view = CreateImageView(m_depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);

    ChangeImageLayout(m_depth_image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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

VkFormat VulkanContext::FindDepthCapableFormat()
{
    //valid depth formats VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
    return FindCapableFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
    VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool VulkanContext::DepthFormatHasStencil(VkFormat format)
{
    return format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT;
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
        m_swap_chain_image_views[i] = CreateImageView(m_swap_chain_images[i], m_swap_chain_format, VK_IMAGE_ASPECT_COLOR_BIT);
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
    // rasterizer_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;//vertex order for faces to be considered front-facing (counter-clock because of y flip)
    rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;//vertex order for faces to be considered front-facing

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

    //<f> Depth and stencil
    VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
    depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_create_info.pNext = nullptr;
    depth_stencil_create_info.flags = 0;

    depth_stencil_create_info.depthTestEnable = VK_TRUE;
    depth_stencil_create_info.depthWriteEnable = VK_TRUE;

    depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;//smaller depth = closer

    depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_create_info.stencilTestEnable = VK_FALSE;
    //</f> /Depth and stencil

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
    graphics_pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
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
    //<f> Attachments
    //<f> Colour
    VkAttachmentDescription colour_attachment{};

    colour_attachment.format = m_swap_chain_format;
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

    depth_attachment.format = FindDepthCapableFormat();//same as depth image
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

    VkRenderPassCreateInfo render_pass_Create_info{};
    render_pass_Create_info.sType =VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    render_pass_Create_info.pNext = nullptr;
    render_pass_Create_info.flags = 0;

    render_pass_Create_info.attachmentCount = static_cast<uint32_t>(descriptions.size());
    render_pass_Create_info.pAttachments = descriptions.data();

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
        std::vector<VkImageView> attachements{m_swap_chain_image_views[i], m_depth_image_view};

        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.pNext = nullptr;
        framebuffer_create_info.flags = 0;

        framebuffer_create_info.attachmentCount = static_cast<uint32_t>(attachements.size());
        framebuffer_create_info.pAttachments = attachements.data();

        framebuffer_create_info.renderPass = m_render_pass;
        framebuffer_create_info.layers = 1;
        framebuffer_create_info.width = m_swap_chain_extent.width;
        framebuffer_create_info.height = m_swap_chain_extent.height;

        if( vkCreateFramebuffer(m_logical_device, &framebuffer_create_info, nullptr, &m_swap_chain_framebuffers[i]) != VK_SUCCESS )
            throw std::runtime_error("Failed to create framebuffer");
    }
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

void VulkanContext::CreateDescriptorSetLayout()
{
    //<f> Vertex descriptor
    VkDescriptorSetLayoutBinding vertex_descriptor_binding{};
    vertex_descriptor_binding.binding = 0;//binding id in the shader
    vertex_descriptor_binding.descriptorCount = 1;
    vertex_descriptor_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;//we are working with uniform shader field
    vertex_descriptor_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;//we are working in the vertex shader

    vertex_descriptor_binding.pImmutableSamplers = nullptr;//for now
    //</f> /Vertex descriptor

    //<f> Sampler descriptor
    VkDescriptorSetLayoutBinding sampler_descriptor_binding{};
    sampler_descriptor_binding.binding = 1;
    sampler_descriptor_binding.descriptorCount = 1;
    sampler_descriptor_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_descriptor_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    sampler_descriptor_binding.pImmutableSamplers = nullptr;
    //</f> /Sampler descriptor

    std::vector<VkDescriptorSetLayoutBinding> bindings{vertex_descriptor_binding, sampler_descriptor_binding};

    //create set layout
    VkDescriptorSetLayoutCreateInfo descriptor_set_creat_info{};
    descriptor_set_creat_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_creat_info.pNext = nullptr;
    descriptor_set_creat_info.flags = 0;

    descriptor_set_creat_info.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptor_set_creat_info.pBindings = bindings.data();

    if( vkCreateDescriptorSetLayout(m_logical_device, &descriptor_set_creat_info, nullptr, &m_descriptor_set_layout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor set layout");
}

void VulkanContext::CreateDescriptorPool()
{
    //pool size
    std::vector<VkDescriptorPoolSize> pool_sizes(2);
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = 1;//UniformBufferObject

    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = 1;//UniformBufferObject

    //
    VkDescriptorPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.pNext = nullptr;
    pool_create_info.flags = 0;

    pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_create_info.pPoolSizes = pool_sizes.data();
    pool_create_info.maxSets = 1;//max allocated sets

    if( vkCreateDescriptorPool(m_logical_device, &pool_create_info, nullptr, &m_descriptor_pool) != VK_SUCCESS )
        throw std::runtime_error("Failed to create descriptor pool");
}

void VulkanContext::CreateDescriptorSet()
{
    //<f> Create Descriptor set
    std::vector<VkDescriptorSetLayout> layouts{m_descriptor_set_layout};

    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;

    allocate_info.descriptorPool = m_descriptor_pool;
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = layouts.data();

    if( vkAllocateDescriptorSets(m_logical_device, &allocate_info, &m_descriptor_set) != VK_SUCCESS )
        throw std::runtime_error("Failed to allocate descriptor set");
    //</f> /Create Descriptor set

    //<f> Descriptor data update

    std::vector<VkWriteDescriptorSet> write_descriptor(2);

    //<f> Uniform buffer
    //config buffer descriptor
    VkDescriptorBufferInfo buffer_config{};
    buffer_config.buffer = m_uniform_buffer;
    buffer_config.offset = 0;
    buffer_config.range = sizeof(UniformBufferObject);

    //update set
    write_descriptor[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor[0].pNext = nullptr;

    write_descriptor[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor[0].dstBinding = 0;//shader binding index
    write_descriptor[0].dstSet = m_descriptor_set;//set to write to
    write_descriptor[0].dstArrayElement = 0;//no array == index 0

    write_descriptor[0].descriptorCount = 1;
    write_descriptor[0].pBufferInfo = &buffer_config;
    //</f> /Uniform buffer

    //<f> Shader sampler
    VkDescriptorImageInfo sampler_config{};
    sampler_config.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    sampler_config.imageView = m_image_view;
    sampler_config.sampler = m_image_sampler;

    write_descriptor[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor[1].pNext = nullptr;

    write_descriptor[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor[1].dstBinding = 1;
    write_descriptor[1].dstSet = m_descriptor_set;//set to write to
    write_descriptor[1].dstArrayElement = 0;
    write_descriptor[1].descriptorCount = 1;
    write_descriptor[1].pImageInfo = &sampler_config;
    //</f> /Shader sampler

    vkUpdateDescriptorSets(m_logical_device, static_cast<uint32_t>(write_descriptor.size()), write_descriptor.data(), 0, nullptr);//write to set or copy to set

    //</f> /Descriptor data update
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
        std::vector<VkClearValue> clear_values(2);
        clear_values[0].color = { {0.f, 0.f, 0.f, 1.f} };//black 100% opacity
        clear_values[1].depthStencil = {1.f, 0};

        render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_begin_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(m_command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

        //vertex buffer input
        std::vector<VkBuffer> vertex_buffers{m_vertex_buffer};
        std::vector<VkDeviceSize> offsets{VkDeviceSize{0}};

        vkCmdBindVertexBuffers(m_command_buffers[i], 0, 1, vertex_buffers.data(), offsets.data());
        vkCmdBindIndexBuffer(m_command_buffers[i], m_index_buffer, 0, VK_INDEX_TYPE_UINT32);
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
