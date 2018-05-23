#ifndef VULKAN_CONTEXT_HPP
#define VULKAN_CONTEXT_HPP

#include <vulkan/vulkan.h>
#include "SDL.h"
#include <vector>
#include <string>
#include "vulkan_structs.hpp"
#include "vulkan_memory_manager.hpp"

namespace vk
{
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
        void RegisterModelsData(std::vector<VulkanModelData>& models_data);
        void Finalize();
        void Cleanup();
        void WaitForIdle();
        //</f> /Methods

        //<f> Public util methods

        uint32_t FindMemoryTypeIndex(uint32_t filter_type, VkMemoryPropertyFlags flags);
        VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags);
        VkImageView CreateCubemapView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags);
        /**
         * \brief Finds the first supported format from the input vector
         * @param wanted_formats vector of formats we want, ordered by preference, with index 0 the highest preference
         * @param tiling        tiling we need
         * @param features       format features we need
         * \return  [description]
         */
        VkFormat FindCapableFormat(const std::vector<VkFormat>& wanted_formats, VkImageTiling tiling, VkFormatFeatureFlags features);

        void CreateQueues(VkQueue*, VkQueue*);
        //</f> /Public util methods

        //<f> Getters/Setters
        void ApplicationInfo(const VkApplicationInfo& info);
        VkApplicationInfo ApplicationInfo() const;

        VkDevice* LogicalDevice() { return &m_logical_device; }

        VulkanMemoryManager* MemoryManager() { return &m_memory_manager; }

        SDL_Window* Window() const { return m_window; }
        VkSurfaceKHR* Surface() { return &m_surface; }

        SwapchainSupport SwapchainSupportStruct() const { return m_swapchain_support; }
        QueueFamiliesIndices QueueFamiliesIndicesStruct() const { return m_queue_families_indices; }

        bool InitOk() const { return m_init_ok; }
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

        VulkanMemoryManager m_memory_manager;

        //<f> Flags
        bool m_init_ok{false};
        //</f> /Flags
        // #ifndef NDEBUG
        //     #define NDEBUG
        // #endif

        #ifdef NDEBUG
            const bool m_enable_validation_layers{false};
        #else
            const bool m_enable_validation_layers{true};
        #endif

        // const std::vector<const char*> m_required_validation_layers = {"VK_LAYER_LUNARG_standard_validation", "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_monitor"};
        const std::vector<const char*> m_required_validation_layers = {"VK_LAYER_LUNARG_standard_validation"};

        //<f> Devices
        VkPhysicalDevice m_physical_device {VK_NULL_HANDLE};
        const std::vector<const char*> m_required_physical_device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkDevice m_logical_device {VK_NULL_HANDLE};
        //</f> /Devices

        //<f> Rendering
        VkSurfaceKHR m_surface {VK_NULL_HANDLE};
        SwapchainSupport m_swapchain_support;
        QueueFamiliesIndices m_queue_families_indices;
        //</f> /Rendering

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
        QueueFamiliesIndices FindQueueFamiliesIndeces(VkPhysicalDevice);

        //<f> SwapChains
        // SwapChainSupportStruct CheckSwapChainSupport();
        SwapchainSupport CheckSwapchainSupport(VkPhysicalDevice);
        //</f> /SwapChains

        //</f> /Devices

        //<f> Rendering
        void CreateSurface();
        //</f> /Rendering

        //<f> Commands
        void CreateCommandPool();
        void CreateCommandBuffers();
        void CreateSemaphores();
        //</f> /Commands

        //</f> /Private Init
};
}//namespace vk
#endif //VULKAN_CONTEXT_HPP
