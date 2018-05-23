#include "vulkan_memory_manager.hpp"
#include <utility>
#include "vulkan_utils.hpp"
#include "vulkan_context.hpp"
#include <iostream>

namespace vk
{
//<f> Constructors & operator=
VulkanMemoryManager::VulkanMemoryManager(VulkanContext* vulkan_context): m_vulkan_context{vulkan_context}
{

}

VulkanMemoryManager::~VulkanMemoryManager() noexcept
{
    // m_memory_blocks.clear();
    m_vulkan_context = nullptr;
}

VulkanMemoryManager::VulkanMemoryManager(const VulkanMemoryManager& other)
{

}

VulkanMemoryManager::VulkanMemoryManager(VulkanMemoryManager&& other) noexcept
{

}

VulkanMemoryManager& VulkanMemoryManager::operator=(const VulkanMemoryManager& other)
{
    if(this != &other)//not same ref
    {
        auto tmp(other);
        *this = std::move(tmp);
    }

    return *this;
}

VulkanMemoryManager& VulkanMemoryManager::operator=(VulkanMemoryManager&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
//<f> Buffer
bool VulkanMemoryManager::RequestBuffer(VkDeviceSize buffer_size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags property_flags, Buffer* buffer)
{
    std::lock_guard<std::mutex> lock{m_blocks_access_mutex};

    //<f> Create Buffer
    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = nullptr;
    buffer_create_info.flags = 0;

    buffer_create_info.size = buffer_size;
    buffer_create_info.usage = usage_flags;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if( vkCreateBuffer(*m_vulkan_context->LogicalDevice(), &buffer_create_info, nullptr, &buffer->buffer) != VK_SUCCESS )
        throw std::runtime_error("Failed to create vertex buffer");
    //</f> /Create Buffer

    //<f> Request Memory block
    VkMemoryRequirements buffer_memory_requirements;
    vkGetBufferMemoryRequirements(*m_vulkan_context->LogicalDevice(), buffer->buffer, &buffer_memory_requirements);
    auto memory_type_index{m_vulkan_context->FindMemoryTypeIndex(buffer_memory_requirements.memoryTypeBits, property_flags)};

    auto key_pair{ std::make_pair(memory_type_index, property_flags) };
    //find a valid memory block
    bool created{false};
    //run through every block with the given key_pair
    for(auto& block : m_memory_blocks[key_pair])
    {
        if(block.second.CreateBuffer(buffer_memory_requirements, usage_flags, buffer))//was able to create buffer
            created = true;
    }

    if(!created)//failed to create buffer, either by not having space in existing blocks or no block of the needed type exists
    {
        auto block_size{VulkanMemoryBlock::DefaultSize()};
        while(block_size < buffer_size) block_size *= 2;//starting default is 64MB so we keep it as a pow2

        //create new block
        VulkanMemoryBlock block{m_vulkan_context->LogicalDevice(), memory_type_index, block_size};
        block.CreateBuffer(buffer_memory_requirements, usage_flags, buffer);
        //store memory block
        m_memory_blocks[key_pair].insert(std::make_pair(block.ID(), std::move(block)));
    }
    //</f> /Request Memory block

    return true;
}

void VulkanMemoryManager::DestroyBuffer(Buffer* buffer)
{
    std::lock_guard<std::mutex> lock{m_blocks_access_mutex};

    //tmp code
    for(auto& block_list : m_memory_blocks)
    {
        auto itr{block_list.second.find(buffer->block_id)};

        if(itr != std::end(block_list.second))//we found the block owning the buffer
            itr->second.ReleaseBuffer(buffer);
    }
}

void VulkanMemoryManager::MapMemory(Buffer* buffer, void **ppData, VkMemoryMapFlags flags)
{
    std::lock_guard<std::mutex> lock{m_blocks_access_mutex};

    //tmp code
    for(auto& block_list : m_memory_blocks)
    {
        auto itr{block_list.second.find(buffer->block_id)};
        
        if(itr != std::end(block_list.second))//we found the block owning the buffer
            itr->second.MapMemory(buffer, ppData, flags);
    }
}

void VulkanMemoryManager::UnmapMemory(Buffer* buffer)
{
    std::lock_guard<std::mutex> lock{m_blocks_access_mutex};

    //tmp code
    for(auto& block_list : m_memory_blocks)
    {
        auto itr{block_list.second.find(buffer->block_id)};

        if(itr != std::end(block_list.second))//we found the block owning the buffer
            itr->second.UnmapMemory(buffer);
    }
}
//</f> /Buffer

//<f> Image
bool VulkanMemoryManager::RequestImageBuffer(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage_flags, VkMemoryPropertyFlags property_flags, ImageBuffer* buffer)
{
    std::lock_guard<std::mutex> lock{m_blocks_access_mutex};

    //<f> Create Image
    VkImageCreateInfo create_info{vk::ImageCreateInfo()};

    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.extent.width = width;
    create_info.extent.height = height;
    create_info.extent.depth = 1;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;

    create_info.format = format;
    create_info.tiling = tiling;

    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.usage = usage_flags;
    // create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;//we will copy to it(transfer) and use it in shaders to colour the mesh(sampled)
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//one queue at a time
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;

    if( vkCreateImage(*m_vulkan_context->LogicalDevice(), &create_info, nullptr, &buffer->image) != VK_SUCCESS )
        throw std::runtime_error("Failed to create image buffer");
    //</f> /Create Image

    //<f> Request Memory block
    VkMemoryRequirements image_memory_requirements;
    vkGetImageMemoryRequirements(*m_vulkan_context->LogicalDevice(), buffer->image, &image_memory_requirements);
    auto memory_type_index{m_vulkan_context->FindMemoryTypeIndex(image_memory_requirements.memoryTypeBits, property_flags)};

    auto image_size{image_memory_requirements.size};
    auto key_pair{ std::make_pair(memory_type_index, property_flags) };
    //find a valid memory block
    bool created{false};
    //run through every block with the given key_pair
    for(auto& block : m_image_memory_blocks[key_pair])
    {
        if(block.second.CreateImageBuffer(image_memory_requirements, usage_flags, buffer))//was able to create buffer
            created = true;
    }

    if(!created)//failed to create buffer, either by not having space in existing blocks or no block of the needed type exists
    {
        auto block_size{VulkanMemoryBlock::DefaultSize()};
        while(block_size < image_size) block_size *= 2;//starting default is 64MB so we keep it as a pow2

        //create new block
        VulkanMemoryBlock block{m_vulkan_context->LogicalDevice(), memory_type_index, block_size};
        block.CreateImageBuffer(image_memory_requirements, usage_flags, buffer);
        //store memory block
        m_image_memory_blocks[key_pair].insert(std::make_pair(block.ID(), std::move(block)));
    }
    //</f> /Request Memory block

    return true;
}

bool VulkanMemoryManager::RequestCubemapBuffer(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage_flags, VkMemoryPropertyFlags property_flags, ImageBuffer* buffer)
{
    std::lock_guard<std::mutex> lock{m_blocks_access_mutex};

    //<f> Create Image
    VkImageCreateInfo create_info{vk::ImageCreateInfo()};

    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    create_info.extent.width = width;
    create_info.extent.height = height;
    create_info.extent.depth = 1;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 6;//6 faces of the cubemap

    create_info.format = format;
    create_info.tiling = tiling;

    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.usage = usage_flags;
    // create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;//we will copy to it(transfer) and use it in shaders to colour the mesh(sampled)
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//one queue at a time
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;

    if( vkCreateImage(*m_vulkan_context->LogicalDevice(), &create_info, nullptr, &buffer->image) != VK_SUCCESS )
        throw std::runtime_error("Failed to create image buffer");
    //</f> /Create Image

    //<f> Request Memory block
    VkMemoryRequirements image_memory_requirements;
    vkGetImageMemoryRequirements(*m_vulkan_context->LogicalDevice(), buffer->image, &image_memory_requirements);
    auto memory_type_index{m_vulkan_context->FindMemoryTypeIndex(image_memory_requirements.memoryTypeBits, property_flags)};

    auto image_size{image_memory_requirements.size};
    auto key_pair{ std::make_pair(memory_type_index, property_flags) };
    //find a valid memory block
    bool created{false};
    //run through every block with the given key_pair
    for(auto& block : m_image_memory_blocks[key_pair])
    {
        if(block.second.CreateImageBuffer(image_memory_requirements, usage_flags, buffer))//was able to create buffer
            created = true;
    }

    if(!created)//failed to create buffer, either by not having space in existing blocks or no block of the needed type exists
    {
        auto block_size{VulkanMemoryBlock::DefaultSize()};
        while(block_size < image_size) block_size *= 2;//starting default is 64MB so we keep it as a pow2

        //create new block
        VulkanMemoryBlock block{m_vulkan_context->LogicalDevice(), memory_type_index, block_size};
        block.CreateImageBuffer(image_memory_requirements, usage_flags, buffer);
        //store memory block
        m_image_memory_blocks[key_pair].insert(std::make_pair(block.ID(), std::move(block)));
    }
    //</f> /Request Memory block

    return true;
}

void VulkanMemoryManager::DestroyImageBuffer(ImageBuffer* buffer)
{
    std::lock_guard<std::mutex> lock{m_blocks_access_mutex};

    //tmp code
    for(auto& block_list : m_image_memory_blocks)
    {
        auto itr{block_list.second.find(buffer->block_id)};

        if(itr != std::end(block_list.second))//we found the block owning the buffer
            itr->second.ReleaseImageBuffer(buffer);
    }
}
//</f> /Image

void VulkanMemoryManager::Clear()
{
    //memory block destructor will destroy the memory allocation
    m_memory_blocks.clear();
    m_image_memory_blocks.clear();
}
//</f> /Methods
}//namespace
