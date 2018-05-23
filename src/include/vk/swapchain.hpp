#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "vulkan_structs.hpp"

namespace vk
{

class Swapchain;
class VulkanContext;
class CommandBuffer;

class Swapchain
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        Swapchain(VulkanContext*, CommandBuffer*);
        /** brief Default destructor */
        virtual ~Swapchain() noexcept;

        /** brief Copy constructor */
        Swapchain(const Swapchain& other) = delete;
        /** brief Move constructor */
        Swapchain(Swapchain&& other) noexcept;

        /** brief Copy operator */
        Swapchain& operator= (const Swapchain& other) = delete;
        /** brief Move operator */
        Swapchain& operator= (Swapchain&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        void Init();
        void Cleanup();
        //</f> /Methods

        //<f> Getters/Setters
        VkSwapchainKHR* SwapchainKHR() { return &m_swapchain; }
        VkExtent2D* Extent() { return &m_extent; }
        VkFormat* Format() { return &m_format; }
        VkFormat* DepthFormat() { return &m_depth_format; }
        std::vector<VkImageView>* ImageViews() { return &m_image_views; }
        uint32_t ImageViewsCount() const { return m_image_views.size(); }
        VkImageView* DepthImageView() { return &m_depth_image_view; }

        float AspectRatio() { return m_extent.width / (float)m_extent.height; }
        //</f> /Getters/Setters

    protected:
        // vars and stuff
        VulkanContext* m_context;
        CommandBuffer* m_commandbuffer;
        bool m_init_ok{false};

        //<f> SwapChain
        VkSwapchainKHR m_swapchain;

        VkFormat m_format;
        VkExtent2D m_extent;

        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_image_views;

        ImageBuffer m_depth_image;
        VkFormat m_depth_format;
        VkImageView m_depth_image_view;
        //</f> /SwapChain
    private:
        //<f> Private Methods
        void CreateSwapchain();
        void CreateSwapchainImageViews();
        void CreateDepthResources();

        VkSurfaceFormatKHR SelectSwapChainFormat(const SwapchainSupport&);
        VkPresentModeKHR SelectSwapChainPresentationMode(const SwapchainSupport&);
        VkExtent2D SelectSwapChainExtent(const SwapchainSupport&);
        //</f> /Private Methods
};

}//namespace vk
#endif //SWAPCHAIN_HPP
