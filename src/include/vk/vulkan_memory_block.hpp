#ifndef VULKAN_MEMORY_BLOCK_HPP
#define VULKAN_MEMORY_BLOCK_HPP

#include <vulkan/vulkan.h>
#include "vulkan_structs.hpp"
#include <map>
#include <mutex>

namespace vk
{

class VulkanMemoryBlock
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        VulkanMemoryBlock(VkDevice* logical_device, uint32_t memory_type_index);
        VulkanMemoryBlock(VkDevice* logical_device, uint32_t memory_type_index, VkDeviceSize size);
        /** brief Default destructor */
        virtual ~VulkanMemoryBlock() noexcept;

        /** brief Copy constructor */
        VulkanMemoryBlock(const VulkanMemoryBlock& other) = delete;
        /** brief Move constructor */
        VulkanMemoryBlock(VulkanMemoryBlock&& other) noexcept;

        /** brief Copy operator */
        VulkanMemoryBlock& operator= (const VulkanMemoryBlock& other) = delete;
        /** brief Move operator */
        VulkanMemoryBlock& operator= (VulkanMemoryBlock&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        bool CreateBuffer(VkMemoryRequirements memory_requirements, VkBufferUsageFlags usage_flags, Buffer* buffer);
        // bool CreateBuffer(VkDeviceSize size, VkDeviceSize alignment, VkBufferUsageFlags usage_flags, Buffer* buffer);
        void ReleaseBuffer(Buffer* buffer);
        bool CreateImageBuffer(VkMemoryRequirements memory_requirements, VkBufferUsageFlags usage_flags, ImageBuffer* buffer);
        // bool CreateImageBuffer(VkDeviceSize size, VkDeviceSize alignment, VkBufferUsageFlags usage_flags, ImageBuffer* buffer);
        void ReleaseImageBuffer(ImageBuffer* buffer);
        //</f> /Methods

        //<f> Getter/Setter
        uint32_t ID() const {return m_block_id; }
        static VkDeviceSize DefaultSize() { return s_buffer_default_size; }
        void MapMemory(Buffer* buffer, void **ppData, VkMemoryMapFlags flags = 0);
        void UnmapMemory(Buffer* buffer);
        //</f> /Getter/Setter

    protected:
        uint32_t m_block_id;

        static VkDeviceSize s_buffer_default_size;
        static uint32_t s_block_id;

        VkDevice* m_logical_device;

        // vars and stuff
        uint32_t m_memory_type_index;
        VkDeviceSize m_allocated_size;
        VkDeviceMemory m_allocated_memory;
        bool m_mapped_memory;

        /**
         * \brief Stores and manages the offsets of the binded buffers
         */
        std::map<VkDeviceSize, VkDeviceSize> m_offsets;
        /**
         * \brief mutex to control the access to containers that manipulate the buffer structs
         */
        std::mutex m_buffer_mutex;

    private:
        //<f> Private Methods
        /**
         * \brief Check if this memory block has space for the needed buffer.
         * @param valid_offset Reference to store the offset to be used, if return == true
         */
        bool BlockHasSpace(VkDeviceSize needed_space, VkDeviceSize& valid_offset, VkDeviceSize alignment = 1, VkDeviceSize starting_offset = 0);

        /**
         * \brief align the result of current_offset + chunk_size to the required alignment
         */
        VkDeviceSize AlignOffset(VkDeviceSize current_offset, VkDeviceSize chunk_size, VkDeviceSize alignment);

        /**
         * \brief Check if the new chunk overlaps with an existing one
         */
        bool Overlap(VkDeviceSize itr, VkDeviceSize needed_space);

        void AllocateMemory();
        //</f> /Private Methods

};
}//namespace vk
#endif //VULKAN_MEMORY_BLOCK_HPP
