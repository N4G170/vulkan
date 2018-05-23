#include "texture.hpp"
#include <utility>
#include <SDL_image.h>
#include <stdexcept>
#include <vulkan_utils.hpp>
#include "vulkan_context.hpp"
#include "resource_manager.hpp"

//<f> Constructors & operator=
Texture::Texture(vk::VulkanPointers vulkan_pointers): m_vulkan_pointers{vulkan_pointers}, m_vulkan_objects_created{false}
{

}

Texture::~Texture() noexcept
{

}

Texture::Texture(Texture&& other) noexcept : m_vulkan_pointers{std::move(other.m_vulkan_pointers)}, m_vulkan_objects_created{std::move(other.m_vulkan_objects_created)},
    m_image{std::move(other.m_image)}, m_image_view{std::move(other.m_image_view)}, m_sampler{std::move(other.m_sampler)}
{

}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
        m_vulkan_pointers = std::move(other.m_vulkan_pointers);
        m_vulkan_objects_created = std::move(other.m_vulkan_objects_created);
        m_image = std::move(other.m_image);
        m_image_view = std::move(other.m_image_view);
        m_sampler = std::move(other.m_sampler);
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Getters/Setters
std::string Texture::Name() const
{
    return m_name;
}
vk::ImageBuffer* Texture::ImageBuffer()
{
    return &m_image;
}

VkImageView* Texture::ImageView()
{
    return &m_image_view;
}

VkSampler* Texture::Sampler()
{
    return &m_sampler;
}

VkDescriptorSet* Texture::DescriptorSet()
{
    return &m_sampler_descriptor_set;
}
//</f> /Getters/Setters

//<f> Methods
void Texture::Cleanup()
{
    m_vulkan_pointers.memory_manager->DestroyImageBuffer(&m_image);

    vkDestroySampler(*m_vulkan_pointers.logical_device, m_sampler, nullptr);
    vkDestroyImageView(*m_vulkan_pointers.logical_device, m_image_view, nullptr);
}

void Texture::LoadTexture(SDL_Surface* surface, const std::string& name)
{
    if(surface != nullptr)
    {
        m_name = name;

        uint32_t width = surface->w, height = surface->h;
        VkDeviceSize image_size = height * surface->pitch;

        // static std::mutex load_mutex;
        // std::lock_guard<std::mutex> lock(load_mutex);
        //copy image data to vulkan buffer
        vk::Buffer staging_buffer;

        m_vulkan_pointers.memory_manager->RequestBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                &staging_buffer);
        void* data{nullptr};
        m_vulkan_pointers.memory_manager->MapMemory(&staging_buffer, &data);
        memcpy(data, surface->pixels, static_cast<size_t>(image_size));
        m_vulkan_pointers.memory_manager->UnmapMemory(&staging_buffer);

        m_vulkan_pointers.memory_manager->RequestImageBuffer(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_image);

        //change image layout to allow buffer copy
        m_vulkan_pointers.commandbuffer->ChangeImageLayout(m_image.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        //copy buffer data to image
        m_vulkan_pointers.commandbuffer->CopyBufferToImage(staging_buffer.buffer, m_image.image, width, height);

        //change image layout to shader read only
        m_vulkan_pointers.commandbuffer->ChangeImageLayout(m_image.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        //free staging variables
        m_vulkan_pointers.memory_manager->DestroyBuffer(&staging_buffer);

        //Create image view and sampler
        m_image_view = m_vulkan_pointers.context->CreateImageView(m_image.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
        CreateSampler();
    }
    else
    {
        throw std::runtime_error("Failed to load texture file: "+name);
    }
}

void Texture::LoadCubemap(SDL_Surface* front, SDL_Surface* back, SDL_Surface* up, SDL_Surface* down, SDL_Surface* right, SDL_Surface* left, const std::string& name)
{
    if(front == nullptr || back == nullptr || up == nullptr || down == nullptr || right == nullptr || left == nullptr)
        throw std::runtime_error("Failed to load cubemap. One of the surfaces is null.");

    m_name = name;
    uint32_t width = front->w, height = front->h;
    VkDeviceSize layer_size = height * front->pitch;//see SDL_Surface definition
    VkDeviceSize image_size = layer_size * 6;

    //<f> Copy to staging
    vk::Buffer staging_buffer;

    m_vulkan_pointers.memory_manager->RequestBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            &staging_buffer);

    uint8_t* data{nullptr};
    m_vulkan_pointers.memory_manager->MapMemory(&staging_buffer, (void**)&data);
    //copy surfaces
    std::vector<SDL_Surface*> surfaces{front, back, up, down, right, left};//surfaces need to be in this order

    for(auto i{0}; i<6; ++i)
    {
        memcpy(data + (layer_size * i), surfaces[i]->pixels, static_cast<size_t>(layer_size));
    }

    m_vulkan_pointers.memory_manager->UnmapMemory(&staging_buffer);
    //</f> /Copy to staging

    m_vulkan_pointers.memory_manager->RequestCubemapBuffer(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_image);

    //change image layout to allow buffer copy
    m_vulkan_pointers.commandbuffer->ChangeCubemapLayout(m_image.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    //copy buffer data to image
    m_vulkan_pointers.commandbuffer->CopyBufferToCubemap(staging_buffer.buffer, m_image.image, width, height);

    //change image layout to shader read only
    m_vulkan_pointers.commandbuffer->ChangeCubemapLayout(m_image.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    //free staging variables
    m_vulkan_pointers.memory_manager->DestroyBuffer(&staging_buffer);

    m_image_view = m_vulkan_pointers.context->CreateCubemapView(m_image.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    CreateCubemapSampler();
}

void Texture::CreateSampler()
{
    VkSamplerCreateInfo create_info{vk::SamplerCreateInfo()};

    //magnify and minify interpolation type
    create_info.magFilter = VK_FILTER_LINEAR;
    create_info.minFilter = VK_FILTER_LINEAR;

    //address mode, ex: repeat, clamp ...
    create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;//X axis
    create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;//Y axis
    create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;//Z axis

    //anisotropic filter - TODO: change to read if context was set to true
    create_info.anisotropyEnable = VK_TRUE;
    create_info.maxAnisotropy = 16;//1 = disabled, to enable we need to request the feature during logical device creation, if the physical has the capability

    create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;//colour for the areas outside the image content

    //False -> texel coordinates normalized to [0,1)
    //True -> texel coordinates as [0, width) and [0, height)
    create_info.unnormalizedCoordinates = VK_FALSE;

    //compare options
    create_info.compareEnable = VK_FALSE;//true, mainly used with shadow maps
    create_info.compareOp = VK_COMPARE_OP_ALWAYS;

    //mipmaps
    create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    create_info.mipLodBias = 0.f;
    create_info.minLod = 0.f;
    create_info.maxLod = 0.f;

    if( vkCreateSampler(*m_vulkan_pointers.logical_device, &create_info, nullptr, &m_sampler) != VK_SUCCESS )
        throw std::runtime_error("Failed to create texture sampler");
}

void Texture::CreateCubemapSampler()
{
    // Create sampler
		VkSamplerCreateInfo create_info{vk::SamplerCreateInfo()};
		create_info.magFilter = VK_FILTER_LINEAR;
		create_info.minFilter = VK_FILTER_LINEAR;

        create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        //anisotropic filter - TODO: change to read if context was set to true, like if(has feature enabled)
        create_info.anisotropyEnable = VK_TRUE;
        create_info.maxAnisotropy = 16;//1 = disabled, to enable we need to request the feature during logical device creation, if the physical has the capability

        create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        //compare options
        create_info.compareEnable = VK_FALSE;//true, mainly used with shadow maps
        create_info.compareOp = VK_COMPARE_OP_ALWAYS;
        // create_info.compareOp = VK_COMPARE_OP_NEVER;

        create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		create_info.mipLodBias = 0.f;
		create_info.minLod = 0.f;
		create_info.maxLod = 0.f;

        if( vkCreateSampler(*m_vulkan_pointers.logical_device, &create_info, nullptr, &m_sampler) != VK_SUCCESS )
            throw std::runtime_error("Failed to create texture cubemap sampler");
}

void Texture::CreateVulkanObjects()
{
    if(m_vulkan_objects_created)
        return;
    m_vulkan_objects_created = true;

    //<f> Create Descriptor set
    std::vector<VkDescriptorSetLayout> layouts{*m_vulkan_pointers.resources->DescriptorSetSamplerLayout()};

    VkDescriptorSetAllocateInfo allocate_info{vk::DescriptiorSetAllocateInfo()};

    allocate_info.descriptorPool = *m_vulkan_pointers.resources->DescriptorPoolSampler();
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = layouts.data();

    if( vkAllocateDescriptorSets(*m_vulkan_pointers.logical_device, &allocate_info, &m_sampler_descriptor_set) != VK_SUCCESS )
        throw std::runtime_error("Failed to allocate descriptor set");
    //</f> /Create Descriptor set

    //<f> Descriptor data update
    std::vector<VkWriteDescriptorSet> write_descriptor = {vk::WriteDescriptorSet()};

    //<f> Shader sampler
    VkDescriptorImageInfo sampler_config{vk::DescriptorImageInfo()};
    sampler_config.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    sampler_config.imageView = m_image_view;
    sampler_config.sampler = m_sampler;

    write_descriptor[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor[0].dstBinding = 0;
    write_descriptor[0].dstSet = m_sampler_descriptor_set;//set to write to
    write_descriptor[0].dstArrayElement = 0;
    write_descriptor[0].descriptorCount = 1;
    write_descriptor[0].pImageInfo = &sampler_config;
    //</f> /Shader sampler

    vkUpdateDescriptorSets(*m_vulkan_pointers.logical_device, static_cast<uint32_t>(write_descriptor.size()), write_descriptor.data(), 0, nullptr);//write to set or copy to set
    //</f> /Descriptor data update
}
//</f> /Methods
