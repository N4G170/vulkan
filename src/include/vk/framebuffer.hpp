#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <vulkan/vulkan.h>
#include <vector>

namespace vk
{

class VulkanContext;
class Renderpass;
class Swapchain;

class Framebuffer
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        explicit Framebuffer(VulkanContext*);
        /** brief Default destructor */
        virtual ~Framebuffer() noexcept;

        /** brief Copy constructor */
        Framebuffer(const Framebuffer& other);
        /** brief Move constructor */
        Framebuffer(Framebuffer&& other) noexcept;

        /** brief Copy operator */
        Framebuffer& operator= (const Framebuffer& other);
        /** brief Move operator */
        Framebuffer& operator= (Framebuffer&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        void Cleanup();
        void Init(Swapchain*, Renderpass*);
        //</f> /Methods

        //<f> Getters/Setters
        uint32_t FramebuffersCount() const { return m_framebuffers.size(); }
        std::vector<VkFramebuffer>* Framebuffers() { return &m_framebuffers; }
        //</f> /Getters/Setters

    protected:
        VulkanContext* m_context;
        // vars and stuff
        /**
         * \brief framebuffer for the swapchain
         */
        std::vector<VkFramebuffer> m_framebuffers;

    private:
};
}//namespace vk

#endif //FRAMEBUFFER_HPP
