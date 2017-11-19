#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <string>
// #include "vulkan_context.hpp"

class VulkanContext;

class Texture
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        explicit Texture(VulkanContext*);
        /** brief Default destructor */
        virtual ~Texture() noexcept;

        /** brief Copy constructor */
        Texture(const Texture& other);
        /** brief Move constructor */
        Texture(Texture&& other) noexcept;

        /** brief Copy operator */
        Texture& operator= (const Texture& other);
        /** brief Move operator */
        Texture& operator= (Texture&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Virtual Methods

        //</f> /Virtual Methods

        //<f> Methods
        void Cleanup();
        void LoadTextureFile(const std::string& file_path);

        void CreateImageView();
        void CreateSampler();
        //</f> /Methods

        //<f> Getters/Setters

        //</f> /Getters/Setters

    protected:
        // vars and stuff

    private:
        VulkanContext* m_vulkan;

        // VkImage m_image;
        // VkImageView m_image_view;
        // VkDeviceMemory m_image_memory;
        // VkSampler m_image_sampler;

        //<f> Private Methods
        /**
         * \Checks if all needed pointers exist, if not throws a runtime_error
         */
        void CheckPointers();
        //</f> /Private Methods
};

#endif //TEXTURE_HPP
