#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <vulkan/vulkan.h>
#include <map>
#include <string>
#include "vulkan_structs.hpp"
#include "vulkan_resources.hpp"

namespace vk
{

class VulkanContext;
class Swapchain;
class Renderpass;

class Pipelines
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        explicit Pipelines(VulkanContext*);
        /** brief Default destructor */
        virtual ~Pipelines() noexcept;

        /** brief Copy constructor */
        Pipelines(const Pipelines& other) = delete;
        /** brief Move constructor */
        Pipelines(Pipelines&& other) noexcept;

        /** brief Copy operator */
        Pipelines& operator= (const Pipelines& other) = delete;
        /** brief Move operator */
        Pipelines& operator= (Pipelines&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        void Cleanup();
        void Init(VulkanResources*, Swapchain*, Renderpass*);
        //</f> /Methods

        //<f> Getters/Setters
        VkPipeline* Pipeline(PipelineType);
        // void SetShaderPath(PipelinesType type, const std::string& path);
        VkPipelineLayout* GraphicsLayout() { return &m_graphics_layout; }
        //</f> /Getters/Setters

    protected:
        VulkanContext* m_context;
        bool m_init_ok{false};

        // vars and stuff
        PipelineCreateInfos m_create_infos;
        VkGraphicsPipelineCreateInfo m_graphics_pipeline_create_info;

        VkPipelineLayout m_graphics_layout {VK_NULL_HANDLE};

        std::map<PipelineType, VkPipeline> m_graphics_pipelines;
        /**
         * \brief Used to store all loaded shader modules.
         * \nModules will only be destroyed during cleanup (to avoid recreating the modules when we rebuild the pipeline)
         */
        std::map<std::string, VkShaderModule> m_shader_modules;
    private:
        //<f> Private Methods
        void CreateGraphicsLayout(VulkanResources*);
        void CreateGraphicsCreateInfo(Swapchain*, Renderpass*);
        void CreateGraphicsMain();
        void CreateGraphicsDerivatives();
        //</f> /Private Methods
};

}//namespace vk
#endif //PIPELINE_HPP
