#include "texture.hpp"
#include <utility>
#include <SDL_image.h>
#include <stdexcept>
#include <iostream>
#include "vulkan_context.hpp"

//<f> Constructors & operator=
Texture::Texture(VulkanContext* context):m_vulkan{context}
{

}

Texture::~Texture() noexcept
{
    // Cleanup();
}

Texture::Texture(const Texture& other)
{

}

Texture::Texture(Texture&& other) noexcept
{

}

Texture& Texture::operator=(const Texture& other)
{
    if(this != &other)//not same ref
    {
        auto tmp(other);
        *this = std::move(tmp);
    }

    return *this;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
void Texture::Cleanup()
{
    CheckPointers();

    VkDevice logical_device{*(m_vulkan->LogicalDevice())};

    vkDestroySampler(logical_device, m_vulkan->m_image_sampler, nullptr);
    vkDestroyImageView(logical_device, m_vulkan->m_image_view, nullptr);
    vkDestroyImage(logical_device, m_vulkan->m_image, nullptr);
    vkFreeMemory(logical_device, m_vulkan->m_image_memory, nullptr);
}

void Texture::LoadTextureFile(const std::string& file_path)
{
    CheckPointers();

    SDL_Surface* surface{ IMG_Load(file_path.data()) };

    if(surface != nullptr)
    {
        //check if surface has 4 channels
        if(surface->format->BytesPerPixel < 4)
            surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);

        VkDevice logical_device{*(m_vulkan->LogicalDevice())};
        uint32_t width = surface->w, height = surface->h;
        VkDeviceSize image_size = height * surface->pitch;

        //copy image data to vulkan buffer
        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;

        m_vulkan->CreateBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                staging_buffer, staging_buffer_memory);

        void* data{nullptr};
        vkMapMemory(logical_device, staging_buffer_memory, 0, image_size, 0, &data);
        memcpy(data, surface->pixels, static_cast<size_t>(image_size));
        vkUnmapMemory(logical_device, staging_buffer_memory);

        SDL_FreeSurface(surface);

        m_vulkan->CreateImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vulkan->m_image, m_vulkan->m_image_memory);

        //change image layout to allow buffer copy
        m_vulkan->ChangeImageLayout(m_vulkan->m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        //copy buffer data to image
        m_vulkan->CopyBufferToImage(staging_buffer, m_vulkan->m_image, width, height);

        //change image layout to shader read only
        m_vulkan->ChangeImageLayout(m_vulkan->m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        //free staging variables
        vkDestroyBuffer(logical_device, staging_buffer, nullptr);
        vkFreeMemory(logical_device, staging_buffer_memory, nullptr);

        //create image view
        // m_vulkan->m_image_view = m_vulkan->CreateImageView(m_vulkan->m_image, VK_FORMAT_R8G8B8A8_UNORM);

        // m_vulkan->m_image_sampler = m_vulkan->CreateSampler();
    }
    else
    {
        throw std::runtime_error("Failed to load texture file");
    }
}

void Texture::CreateImageView()
{
    m_vulkan->m_image_view = m_vulkan->CreateImageView(m_vulkan->m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Texture::CreateSampler()
{
    m_vulkan->m_image_sampler = m_vulkan->CreateSampler();
}
//</f> /Methods

//<f> Private Methods
/**
 * \Checks if all needed pointers exist, if not throws a runtime_error
 */
void Texture::CheckPointers()
{
    if(m_vulkan == nullptr)
        throw std::runtime_error("Vulkan context pointer is null");
}
//</f> /Private Methods
