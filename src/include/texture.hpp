#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <vulkan/vulkan.h>
#include "vulkan_pointers.hpp"
#include <string>
#include <mutex>

class Texture
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        explicit Texture(vk::VulkanPointers);
        /** brief Default destructor */
        virtual ~Texture() noexcept;

        /** brief Copy constructor */
        Texture(const Texture& other) = delete;
        /** brief Move constructor */
        Texture(Texture&& other) noexcept;

        /** brief Copy operator */
        Texture& operator= (const Texture& other) = delete;
        /** brief Move operator */
        Texture& operator= (Texture&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        void Cleanup();
        void LoadTexture(SDL_Surface* surface, const std::string& name);
        void LoadCubemap(SDL_Surface* front, SDL_Surface* back, SDL_Surface* up, SDL_Surface* down, SDL_Surface* right, SDL_Surface* left, const std::string& name);

        void CreateSampler();
        void CreateCubemapSampler();

        void CreateVulkanObjects();
        //</f> /Methods

        //<f> Getters/Setters
        std::string Name() const;
        vk::ImageBuffer* ImageBuffer();
        VkImageView* ImageView();
        VkSampler* Sampler();

        VkDescriptorSet* DescriptorSet();

        bool IsCubemap();
        //</f> /Getters/Setters

    protected:
        vk::VulkanPointers m_vulkan_pointers;
        bool m_vulkan_objects_created;
        std::string m_name;
        // vars and stuff
        vk::ImageBuffer m_image;
        VkImageView m_image_view;
        VkSampler m_sampler;

        VkDescriptorSet m_sampler_descriptor_set;
        bool m_is_cubemap;
    private:
};

#endif //TEXTURE_HPP
