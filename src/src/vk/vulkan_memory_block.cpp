#include "vulkan_memory_block.hpp"
#include <utility>
// #include <iostream>
#include "vulkan_utils.hpp"

namespace vk
{
VkDeviceSize VulkanMemoryBlock::s_buffer_default_size{268435456};//256MB
// VkDeviceSize VulkanMemoryBlock::s_texture_buffer_default_size{268435456};//256MB
// VkDeviceSize VulkanMemoryBlock::s_buffer_default_size{134217728};//128MB
// VkDeviceSize VulkanMemoryBlock::s_buffer_default_size{67108864};//64MB
uint32_t VulkanMemoryBlock::s_block_id{0};

//<f> Constructors & operator=
VulkanMemoryBlock::VulkanMemoryBlock(VkDevice* logical_device, uint32_t memory_type_index): m_logical_device{logical_device}, m_memory_type_index{memory_type_index},
    m_allocated_size{s_buffer_default_size}, m_mapped_memory{false}
{
    m_block_id = s_block_id++;
    AllocateMemory();
}

VulkanMemoryBlock::VulkanMemoryBlock(VkDevice* logical_device, uint32_t memory_type_index, VkDeviceSize size): m_logical_device{logical_device}, m_memory_type_index{memory_type_index},
    m_allocated_size{size}, m_mapped_memory{false}
{
    m_block_id = s_block_id++;
    AllocateMemory();
}

VulkanMemoryBlock::~VulkanMemoryBlock() noexcept
{
    if(m_allocated_memory != VK_NULL_HANDLE)
        vkFreeMemory(*m_logical_device, m_allocated_memory, nullptr);
}

VulkanMemoryBlock::VulkanMemoryBlock(VulkanMemoryBlock&& other) noexcept : m_block_id{std::move(other.m_block_id)}, m_logical_device{std::move(other.m_logical_device)},
    m_memory_type_index{std::move(other.m_memory_type_index)}, m_allocated_size{std::move(other.m_allocated_size)}, m_allocated_memory{std::move(other.m_allocated_memory)},
    m_mapped_memory{std::move(other.m_mapped_memory)}, m_offsets{std::move(other.m_offsets)}
{
    other.m_allocated_memory = VK_NULL_HANDLE;
}

VulkanMemoryBlock& VulkanMemoryBlock::operator=(VulkanMemoryBlock&& other) noexcept
{
    if(this != &other)//not same ref
    {
        m_block_id = std::move(other.m_block_id);
        m_logical_device = std::move(other.m_logical_device);
        m_memory_type_index = std::move(other.m_memory_type_index);
        m_allocated_size = std::move(other.m_allocated_size);
        m_allocated_memory = std::move(other.m_allocated_memory);
        m_mapped_memory = std::move(other.m_mapped_memory);
        m_offsets = std::move(other.m_offsets);

        other.m_allocated_memory = VK_NULL_HANDLE;
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
bool VulkanMemoryBlock::CreateBuffer(VkMemoryRequirements memory_requirements, VkBufferUsageFlags usage_flags, Buffer* buffer)
{
    std::lock_guard<std::mutex> lock(m_buffer_mutex);

    if(buffer->buffer == VK_NULL_HANDLE)
        throw std::runtime_error("Buffer handle is not NULL");

    buffer->block_id = m_block_id;
    buffer->size = memory_requirements.size;
    buffer->usage_flags = usage_flags;

    //<f> Bind to memory
    //find offset
    VkDeviceSize offset{0};

    if(BlockHasSpace(memory_requirements.size, offset, memory_requirements.alignment))
    {
        buffer->offset = offset;
        vkBindBufferMemory(*m_logical_device, buffer->buffer, m_allocated_memory, offset);
        //store offset
        m_offsets[offset] = memory_requirements.size;
        return true;
    }
    else
        return false;
    //</f> /Bind to memory
}

void VulkanMemoryBlock::ReleaseBuffer(Buffer* buffer)
{
    std::lock_guard<std::mutex> lock(m_buffer_mutex);

    if(buffer->buffer != VK_NULL_HANDLE)
    {
        m_offsets.erase(buffer->offset);
        vkDestroyBuffer(*m_logical_device, buffer->buffer, nullptr);

        buffer->buffer = VK_NULL_HANDLE;
    }
}

bool VulkanMemoryBlock::CreateImageBuffer(VkMemoryRequirements memory_requirements, VkBufferUsageFlags usage_flags, ImageBuffer* buffer)
{
    std::lock_guard<std::mutex> lock(m_buffer_mutex);

    if(buffer->image == VK_NULL_HANDLE)
        throw std::runtime_error("Buffer image handle is not NULL");

    buffer->block_id = m_block_id;
    buffer->size = memory_requirements.size;
    buffer->usage_flags = usage_flags;

    //<f> Bind to memory
    //find offset
    VkDeviceSize offset{0};

    if(BlockHasSpace(memory_requirements.size, offset, memory_requirements.alignment))
    {
        buffer->offset = offset;
        vkBindImageMemory(*m_logical_device, buffer->image, m_allocated_memory, offset);
        //store offset
        m_offsets[offset] = memory_requirements.size;

        return true;
    }
    else
    {
        return false;
    }
    //</f> /Bind to memory
}

void VulkanMemoryBlock::ReleaseImageBuffer(ImageBuffer* buffer)
{
    std::lock_guard<std::mutex> lock(m_buffer_mutex);

    if(buffer->image != VK_NULL_HANDLE)
    {
        // std::cout<<buffer->offset<<std::endl;
        m_offsets.erase(buffer->offset);
        // std::cout<<" - "<<buffer->offset<<std::endl;
        vkDestroyImage(*m_logical_device, buffer->image, nullptr);

        buffer->image = VK_NULL_HANDLE;
    }
}

void VulkanMemoryBlock::MapMemory(Buffer* buffer, void **ppData, VkMemoryMapFlags flags)
{
    if(!m_mapped_memory)
    {
        vkMapMemory(*m_logical_device, m_allocated_memory, buffer->offset, buffer->size, flags, ppData);
        m_mapped_memory = true;
    }
    else
        throw std::runtime_error("Buffer is already mapped. Unmap it before trying again,");
}

void VulkanMemoryBlock::UnmapMemory(Buffer* buffer)
{
    if(m_mapped_memory)
    {
        vkUnmapMemory(*m_logical_device, m_allocated_memory);
        m_mapped_memory = false;
    }
}
//</f> /Methods

//<f> Private Methods
bool VulkanMemoryBlock::BlockHasSpace(VkDeviceSize needed_space, VkDeviceSize& valid_offset, VkDeviceSize alignment, VkDeviceSize starting_offset)
{
    if(needed_space > m_allocated_size)
        return false;

    //correct alignment to avoid errors
    if(alignment == 0)
        alignment = 1;

    for(auto itr{starting_offset}; itr < m_allocated_size; )
    {
        if(m_offsets.find(itr) != std::end(m_offsets))//valid offset
        {
            itr = AlignOffset(itr, m_offsets[itr], alignment);
        }
        else
        {
            // m_offsets.erase(itr);//clean up as [] creates a new entry
            //check if itr is at an offset big enough
            if(!Overlap(itr, needed_space))
            {
                valid_offset = itr;
                return true;
            }
            itr += alignment;
        }
    }
    return false;
}

VkDeviceSize VulkanMemoryBlock::AlignOffset(VkDeviceSize current_offset, VkDeviceSize chunk_size, VkDeviceSize alignment)
{
    VkDeviceSize tmp_pos{current_offset + chunk_size};

    if(tmp_pos%alignment == 0)//already aligned
        return tmp_pos;

    //example:
    //alignment = 10 (every 10)
    //tmp_pos = 45; (next valid alignment = 50)
    //ratio = 45/10 = 4;
    //result = (4 + 1) * 10 =  50
    return ((tmp_pos / alignment) + 1) * alignment;
}

bool VulkanMemoryBlock::Overlap(VkDeviceSize itr, VkDeviceSize needed_space)
{
    auto end{itr + needed_space - 1};

    if(end >= m_allocated_size)
        return true;

    for(auto pair : m_offsets)
    {
        if(itr >= pair.first && itr <= pair.second-1)
        {
            return true;
        }
        if(end >= pair.first && end <= pair.second-1)
        {
            return true;
        }
        if(itr <= pair.first && end >= pair.second -1)
            return true;
    }

    return false;
}

void VulkanMemoryBlock::AllocateMemory()
{
    //<f> Memory allocation
    VkMemoryAllocateInfo allocate_info{MemoryAllocateInfo()};

    allocate_info.allocationSize = m_allocated_size;
    allocate_info.memoryTypeIndex = m_memory_type_index;

    if( vkAllocateMemory(*m_logical_device, &allocate_info, nullptr, &m_allocated_memory) != VK_SUCCESS )
        throw std::runtime_error("Failed to allocate memory block");
    //</f> /Memory allocation
}
//</f> /Private Methods
}//namespace
