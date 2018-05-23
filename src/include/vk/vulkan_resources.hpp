#ifndef VULKAN_RESOURCES_HPP
#define VULKAN_RESOURCES_HPP

#include <vulkan/vulkan.h>
#include "vulkan_context.hpp"

namespace vk
{
class VulkanResources
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        explicit VulkanResources(VulkanContext*);
        /** brief Default destructor */
        virtual ~VulkanResources() noexcept;

        /** brief Copy constructor */
        VulkanResources(const VulkanResources& other) = delete;
        /** brief Move constructor */
        VulkanResources(VulkanResources&& other) noexcept;

        /** brief Copy operator */
        VulkanResources& operator= (const VulkanResources& other) = delete;
        /** brief Move operator */
        VulkanResources& operator= (VulkanResources&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        void Cleanup();
        void InitDescriptorLayoutAndPools();
        //</f> /Methods

        //<f> Getters/Setters
        VkDescriptorSetLayout* DescriptorSetUniformLayout();
        VkDescriptorSetLayout* DescriptorSetSamplerLayout();
        VkDescriptorPool* DescriptorPoolUniform();
        VkDescriptorPool* DescriptorPoolSampler();

        void MaxDescriptorCount(uint32_t);
        uint32_t MaxDescriptorCount() const;
        //</f> /Getters/Setters

    protected:
        VulkanContext* m_context;

        uint32_t m_max_descriptor_count;

        //<f> Flags
        bool m_descriptors_init_ok{false};
        //</f> /Flags
        // vars and stuff
        //<f> Descriptors
        VkDescriptorSetLayout m_uniform_descriptor_layout;
        VkDescriptorPool m_uniform_descriptor_pool;

        VkDescriptorSetLayout m_sampler_descriptor_layout;
        VkDescriptorPool m_sampler_descriptor_pool;
        //</f> /Descriptors
    private:
        //<f> Private Methods
        /**
         * \brief Initialize the descriptor layout and pool for the materials
         */
        void InitDescriptorsLayouts();
        void InitDescriptorsPools();
        //</f> /Private Methods
};

}//namespace vk
#endif //VULKAN_RESOURCES_HPP
