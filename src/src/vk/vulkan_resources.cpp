#include "vulkan_resources.hpp"
#include <utility>
#include "vulkan_utils.hpp"
#include <iostream>

namespace vk
{

//<f> Constructors & operator=
VulkanResources::VulkanResources(VulkanContext* context): m_context{context}
{

}

VulkanResources::~VulkanResources() noexcept
{

}

VulkanResources::VulkanResources(VulkanResources&& other) noexcept
{

}

VulkanResources& VulkanResources::operator=(VulkanResources&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
void VulkanResources::Cleanup()
{
    vkDestroyDescriptorPool(*m_context->LogicalDevice(), m_uniform_descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(*m_context->LogicalDevice(), m_uniform_descriptor_layout, nullptr);

    vkDestroyDescriptorPool(*m_context->LogicalDevice(), m_sampler_descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(*m_context->LogicalDevice(), m_sampler_descriptor_layout, nullptr);
}

void VulkanResources::InitDescriptorLayoutAndPools()
{
    InitDescriptorsLayouts();
    InitDescriptorsPools();
}
//</f> /Methods

//<f> Getters/Setters
VkDescriptorSetLayout* VulkanResources::DescriptorSetSamplerLayout()
{
    return &m_sampler_descriptor_layout;
}

VkDescriptorSetLayout* VulkanResources::DescriptorSetUniformLayout()
{
    return &m_uniform_descriptor_layout;
}

VkDescriptorPool* VulkanResources::DescriptorPoolSampler()
{
    return &m_sampler_descriptor_pool;
}

VkDescriptorPool* VulkanResources::DescriptorPoolUniform()
{
    return &m_uniform_descriptor_pool;
}

void VulkanResources::MaxDescriptorCount(uint32_t count)
{
    m_max_descriptor_count = count;
}

uint32_t VulkanResources::MaxDescriptorCount() const
{
    return m_max_descriptor_count;
}
//</f> /Getters/Setters

//<f> Private Methods
void VulkanResources::InitDescriptorsLayouts()
{
    std::vector<VkDescriptorSetLayoutBinding> bindings{VkDescriptorSetLayoutBinding{}};
    VkDescriptorSetLayoutCreateInfo descriptor_set_create_info{vk::DescriptorSetLayoutCreateInfo()};

    //<f> Materials
    //<f> Sampler descriptor
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[0].pImmutableSamplers = nullptr;
    //</f> /Sampler descriptor

    descriptor_set_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptor_set_create_info.pBindings = bindings.data();

    if( vkCreateDescriptorSetLayout(*m_context->LogicalDevice(), &descriptor_set_create_info, nullptr, &m_sampler_descriptor_layout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create materials descriptor set layout");
    //</f> /Materials

    //<f> Uniform Buffer

    //<f> Uniform descriptor
    bindings[0].binding = 0;//binding id in the shader
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;//we are working with uniform shader field
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;//we are working in the vertex shader

    bindings[0].pImmutableSamplers = nullptr;//for now
    //</f> /Uniform descriptor

    descriptor_set_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptor_set_create_info.pBindings = bindings.data();

    if( vkCreateDescriptorSetLayout(*m_context->LogicalDevice(), &descriptor_set_create_info, nullptr, &m_uniform_descriptor_layout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create uniform buffer descriptor set layout");
    //</f> /Uniform Buffer
}

void VulkanResources::InitDescriptorsPools()
{
    std::vector<VkDescriptorPoolSize> pool_sizes(1);
    VkDescriptorPoolCreateInfo pool_create_info{vk::DescriptorPoolCreateInfo()};
    //<f> Materials
    //pool size
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[0].descriptorCount = m_max_descriptor_count;//UniformBufferObject
    //
    pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_create_info.pPoolSizes = pool_sizes.data();
    pool_create_info.maxSets = m_max_descriptor_count;//max allocated sets

    if( vkCreateDescriptorPool(*m_context->LogicalDevice(), &pool_create_info, nullptr, &m_sampler_descriptor_pool) != VK_SUCCESS )
        throw std::runtime_error("Failed to create descriptor pool");
    //</f> /Materials

    //<f> Uniform Buffers
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = m_max_descriptor_count;//UniformBufferObject
    //
    pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_create_info.pPoolSizes = pool_sizes.data();
    pool_create_info.maxSets = m_max_descriptor_count;//max allocated sets

    if( vkCreateDescriptorPool(*m_context->LogicalDevice(), &pool_create_info, nullptr, &m_uniform_descriptor_pool) != VK_SUCCESS )
        throw std::runtime_error("Failed to create descriptor pool");
    //</f> /Uniform Buffers
}
//</f> /Private Methods
}//namespace vk
