#ifndef VULKAN_MEMORY_MANAGER_HPP
#define VULKAN_MEMORY_MANAGER_HPP

#include <vulkan/vulkan.h>
#include <map>
#include <vector>
#include "vulkan_memory_block.hpp"
#include <mutex>
// #include "vulkan_context.hpp"


namespace vk
{
class VulkanContext;

class VulkanMemoryManager
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        explicit VulkanMemoryManager(VulkanContext* vulkan_context);
        /** brief Default destructor */
        virtual ~VulkanMemoryManager() noexcept;

        /** brief Copy constructor */
        VulkanMemoryManager(const VulkanMemoryManager& other);
        /** brief Move constructor */
        VulkanMemoryManager(VulkanMemoryManager&& other) noexcept;

        /** brief Copy operator */
        VulkanMemoryManager& operator= (const VulkanMemoryManager& other);
        /** brief Move operator */
        VulkanMemoryManager& operator= (VulkanMemoryManager&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        //<f> Buffer
        bool RequestBuffer(VkDeviceSize buffer_size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags property_flags, Buffer* buffer);
        void DestroyBuffer(Buffer*);

        void MapMemory(Buffer* buffer, void **ppData, VkMemoryMapFlags flags = 0);
        void UnmapMemory(Buffer* buffer);
        //</f> /Buffer

        //<f> Image
        bool RequestImageBuffer(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, ImageBuffer* buffer);
        bool RequestCubemapBuffer(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, ImageBuffer* buffer);
        void DestroyImageBuffer(ImageBuffer*);
        //</f> /Image

        void Clear();
        //</f> /Methods

        //<f> Getters/Setters
        VulkanMemoryBlock RequestMemoryBlock();
        //</f> /Getters/Setters

    protected:
        // vars and stuff
        VulkanContext* m_vulkan_context;
        /**
         * key std::pair<uint32_t, uint32_t> represents memoryTypeIndex, VkMemoryPropertyFlags
         * value std::map<uint32_t, VulkanMemoryBlock> key = internal id stored in a static var
         */
        std::map<std::pair<uint32_t, uint32_t>, std::map<uint32_t, VulkanMemoryBlock>> m_memory_blocks;
        std::map<std::pair<uint32_t, uint32_t>, std::map<uint32_t, VulkanMemoryBlock>> m_image_memory_blocks;

        std::mutex m_blocks_access_mutex;
    private:
};
}//namespace
#endif //VULKAN_MEMORY_MANAGER_HPP
