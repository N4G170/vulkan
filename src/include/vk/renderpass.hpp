#ifndef RENDERPASS_HPP
#define RENDERPASS_HPP

#include <vulkan/vulkan.h>

namespace vk
{

class VulkanContext;
class Swapchain;

class Renderpass
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        explicit Renderpass(VulkanContext*);
        /** brief Default destructor */
        virtual ~Renderpass() noexcept;

        /** brief Copy constructor */
        Renderpass(const Renderpass& other);
        /** brief Move constructor */
        Renderpass(Renderpass&& other) noexcept;

        /** brief Copy operator */
        Renderpass& operator= (const Renderpass& other);
        /** brief Move operator */
        Renderpass& operator= (Renderpass&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        void Cleanup();
        void Init(Swapchain*);
        //</f> /Methods

        //<f> Getters/Setters
        VkRenderPass* RenderPass() { return &m_renderpass; }
        //</f> /Getters/Setters

    protected:
        // vars and stuff
        VulkanContext* m_context;
        VkRenderPass m_renderpass;
    private:
};

}//namespace vk
#endif //RENDERPASS_HPP
