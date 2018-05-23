#ifndef COMMANDBUFFER_HPP
#define COMMANDBUFFER_HPP

#include <vulkan/vulkan.h>
#include <vector>
#include "vulkan_structs.hpp"

namespace vk
{
//forward declaration
class VulkanContext;
class Framebuffer;
class Renderpass;
class Pipelines;
class Swapchain;

class CommandBuffer
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        explicit CommandBuffer(VulkanContext*);
        /** brief Default destructor */
        virtual ~CommandBuffer() noexcept;

        /** brief Copy constructor */
        CommandBuffer(const CommandBuffer& other);
        /** brief Move constructor */
        CommandBuffer(CommandBuffer&& other) noexcept;

        /** brief Copy operator */
        CommandBuffer& operator= (const CommandBuffer& other);
        /** brief Move operator */
        CommandBuffer& operator= (CommandBuffer&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        void Cleanup();
        void Init();
        void InitMainCommanfBuffers(Framebuffer*);
        VkCommandBuffer BeginCommand();
        void SubmitCommand(VkCommandBuffer);

        void RegisterModelsData(Swapchain*, Renderpass*, Framebuffer*, Pipelines*, std::vector<VulkanModelData>);

        void DrawFrame(Swapchain*, bool*);

        /**
         * \brief Copy the content of one buffer to another
         * @param src        source buffer
         * @param dst        destination buffer
         * @param size       size of area to copy
         * @param src_offset source buffer read offset (Default = 0)
         * @param dst_offset destination buffer write offset (Default = 0)
         */
        void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size, VkDeviceSize src_offset = 0, VkDeviceSize dst_offset = 0);
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void CopyBufferToCubemap(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void ChangeImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
        void ChangeCubemapLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
        //</f> /Methods

        //<f> Getters/Setters

        //</f> /Getters/Setters

    protected:
        VulkanContext* m_context;
        // vars and stuff

        //<f> Commands
        VkCommandPool m_command_pool;
        std::vector<VkCommandBuffer> m_command_buffers;

        VkSemaphore m_image_available_semaphore;
        VkSemaphore m_render_finished_semaphore;
        //</f> /Commands

        //<f> Queues
        VkQueue m_graphics_queue;
        VkQueue m_presentation_queue;
        //</f> /Queues

    private:
        //<f> Private Methods
        void CreateCommandPool();
        void CreateCommandBuffers(Framebuffer*);
        void CreateSemaphores();
        void CreateQueues();
        //</f> /Private Methods
};
}//namespace vk

#endif //COMMANDBUFFER_HPP
