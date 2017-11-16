#ifndef VULKAN_CONTEXT_HPP
#define VULKAN_CONTEXT_HPP

#include <vulkan/vulkan.h>
#include "SDL.h"
#include <vector>
#include <string>
#include "vulkan_structs.hpp"

class VulkanContext
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        explicit VulkanContext(SDL_Window*);
        /** brief Default destructor */
        virtual ~VulkanContext() noexcept;

        /** brief Copy constructor */
        VulkanContext(const VulkanContext& other);
        /** brief Move constructor */
        VulkanContext(VulkanContext&& other) noexcept;

        /** brief Copy operator */
        VulkanContext& operator= (const VulkanContext& other);
        /** brief Move operator */
        VulkanContext& operator= (VulkanContext&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        void Init();
        void Cleanup();
        void DrawFrame();
        void WaitForIdle();
        void Resize();
        void UpdateUniformBuffer();
        //</f> /Methods

        //<f> Getters/Setters
        void ApplicationInfo(const VkApplicationInfo& info);
        VkApplicationInfo ApplicationInfo() const;
        //</f> /Getters/Setters

    protected:
        // vars and stuff

    private:
        //<f> Private Vars
        /**
         * \brief SDL window to get the extensions from
         */
        SDL_Window* m_window;
        /**
        * \brief entry point for comunication with the vulkan lib
        */
        VkInstance m_vulkan_instance;
        /**
         * \brief Struct that holds information regarding the current application.
         * \n Used to pass more info to the driver
         */
        VkApplicationInfo m_application_info;
        /**
         * \brief
         */
        VkDebugReportCallbackEXT m_debug_callback;

        #ifdef NDEBUG
            const bool m_enable_validation_layers = false;
        #else
            const bool m_enable_validation_layers = true;
        #endif

        const std::vector<const char*> m_required_validation_layers = {"VK_LAYER_LUNARG_standard_validation"};

        //<f> Devices
        VkPhysicalDevice m_physical_device {VK_NULL_HANDLE};
        const std::vector<const char*> m_required_physical_device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkDevice m_logical_device {VK_NULL_HANDLE};
        //</f> /Devices

        //<f> Rendering
        VkSurfaceKHR m_surface {VK_NULL_HANDLE};

        VkRenderPass m_render_pass;
        VkPipelineLayout m_pipeline_layout {VK_NULL_HANDLE};
        VkPipeline m_graphics_pipeline;

        std::vector<VkFramebuffer> m_swap_chain_framebuffers;
        VkBuffer m_vertex_buffer;
        VkDeviceMemory m_vertex_buffer_memory;

        VkBuffer m_index_buffer;
        VkDeviceMemory m_index_buffer_memory;

        VkBuffer m_uniform_buffer;
        VkDeviceMemory m_uniform_buffer_memory;

        VkDescriptorSetLayout m_descriptor_set_layout;
        VkDescriptorPool m_descriptor_pool;
        VkDescriptorSet m_descriptor_set;
        //</f> /Rendering
        //<f> Queues
        VkQueue m_graphics_queue;
        VkQueue m_presentation_queue;
        //</f> /Queues
        //<f> SwapChains
        VkSwapchainKHR m_swap_chain;

        VkFormat m_swap_chain_format;
        VkExtent2D m_swap_chain_extent;

        std::vector<VkImage> m_swap_chain_images;
        std::vector<VkImageView> m_swap_chain_image_views;
        //</f> /SwapChains

        //<f> Commands
        VkCommandPool m_command_pool;
        std::vector<VkCommandBuffer> m_command_buffers;

        VkSemaphore m_image_available_semaphore;
        VkSemaphore m_render_finished_semaphore;
        //</f> /Commands
        //</f> /Private Vars

        //<f> Private Init
        //<f> Instance & Validation

        void DefaultInitApplicationInfo();
        void CreateVulkanInstance();

        void CreateDebugCallback();
        void DestroyDebugCallback(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* allocator);

        bool CheckValidationLayerAvailability();
        std::vector<const char*> GetRequiredExtensions();

        /**
         * \brief We need to load the debug extension function to be able to use it.
         * \nAs it belongs to an extensions it is not automatically loaded.
         */
        VkResult LoadDebugReportCallbackEXTAddr(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* create_info,
            const VkAllocationCallbacks* allocator, VkDebugReportCallbackEXT* callback);

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback( VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
            uint64_t object, size_t location, int32_t message_code, const char* layer_prefix, const char* message, void* user_data);
        //</f> /Instance & Validation

        //<f> Devices
        void SelectPhysicalDevice();
        bool IsPhysicalDeviceCapable(VkPhysicalDevice);
        bool CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice);

        void CreateLogicalDevice();
        //<f> Queue Families
        bool HasNeededQueueFamilies(VkPhysicalDevice);
        QueueFamiliesIndices FindQueueFamiliesIndeces(VkPhysicalDevice);
        //</f> /Queue Families

        //<f> SwapChains
        SwapChainSupportStruct CheckSwapChainSupport(VkPhysicalDevice);

        VkSurfaceFormatKHR SelectSwapChainFormat(const SwapChainSupportStruct&);
        VkPresentModeKHR SelectSwapChainPresentationMode(const SwapChainSupportStruct&);
        VkExtent2D SelectSwapChainExtent(const SwapChainSupportStruct&);

        void CreateSwapChain();
        void RecreateSwapChain();
        void CleanUpSwapChain();
        //</f> /SwapChains
        //</f> /Devices

        //<f> Rendering
        void CreateSurface();

        void CreateImageViews();

        void CreateGraphicsPipeline();

        VkShaderModule CreateShaderModule(const std::vector<char>& shader_code);

        void CreateRenderPass();

        void CreateFramebuffers();
        void CreateBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);
        void CreateVertexBuffer();
        void CreateIndexBuffer();
        void CreateUniformBuffer();
        void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
        uint32_t FindMemoryType(uint32_t filter_type, VkMemoryPropertyFlags flags);

        void CreateDescriptorSetLayout();
        void CreateDescriptorPool();
        void CreateDescriptorSet();
        //</f> /Rendering

        //<f> Commands
        void CreateCommandPool();
        void CreateCommandBuffers();
        void CreateSemaphores();
        //</f> /Commands

        //</f> /Private Init
};

#endif //VULKAN_CONTEXT_HPP
