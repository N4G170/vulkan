#ifndef RESOURCE_MAMAGER_HPP
#define RESOURCE_MAMAGER_HPP

#include <map>
#include <string>
#include <mutex>
#include "vulkan_pointers.hpp"

#include "material.hpp"
#include "texture.hpp"
#include "model.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include "structs.hpp"

struct MeshDataCache
{
    Material* material;
    std::vector<vk::Vertex> vertices;
    std::vector<uint32_t> indices;

    MeshDataCache(): material{nullptr} {}
};

struct ModelDataCache
{
    std::string name;
    std::vector<MeshDataCache> meshes;
};



class ResourceManager
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        explicit ResourceManager(vk::VulkanPointers);
        /** brief Default destructor */
        virtual ~ResourceManager() noexcept;

        /** brief Copy constructor */
        ResourceManager(const ResourceManager& other) = delete;
        /** brief Move constructor */
        ResourceManager(ResourceManager&& other) noexcept;

        /** brief Copy operator */
        ResourceManager& operator= (const ResourceManager& other) = delete;
        /** brief Move operator */
        ResourceManager& operator= (ResourceManager&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        /**
         * \brief Load the model file at \param path.
         * \nReturns a pointer to the stored model
         */
        Model* LoadModel(const std::string& path);

        /**
         * \brief Loads a quadrilateral shape with the given vertices.
         */
        Model* LoadQuadrilateral(const glm::vec3& top_left, const glm::vec3& top_right, const glm::vec3& bottom_left, const glm::vec3& bottom_right, Material* material = nullptr);

        /**
         * \brief Init vulkan dependant objects on models and textures
         * \nOnly call after vulkan context init
         */
        void InitDescriptorLayoutAndPools();

        void InitVulkanObjects();
        void Cleanup();
        void PreLoadSurface(const std::string& path);
        SDL_Surface* PreLoadAndGetSurface(const std::string& path);
        /**
        * \brief Load a texture from file or reuses a surface pointer (if already loaded) to create a Texture instance
        * \n might call PreLoadSurface inside
        */
        void PreLoadTexture(const std::string& path);
        Texture* PreLoadAndGetTexture(const std::string& path);
        Texture* PreLoadAndGetCubemap(const std::string& front_path, const std::string& back_path, const std::string& up_path,
                                      const std::string& down_path, const std::string& right_path, const std::string& left_path);

        Model* LoadTerrain(const LoadTerrainData& data);

        /**
         * \brief Creates an empty material
         * \return  nullptr if the name alredy exists
         */
        Material* CreateMaterial(const std::string& name);
        //</f> /Methods

        //<f> Getters/Setters
        Texture* GetTexture(const std::string& path);
        //</f> /Getters/Setters

    protected:
        // vars and stuff
        vk::VulkanPointers m_vulkan_pointers;
        Assimp::Importer m_assimp_importer;
        std::map<std::string, ModelDataCache> m_cached_models;

        //<f> Models
        // std::map<std::string, Model> m_models;
        public:std::vector<Model*> m_models;//will change to map we I start reusing loaded model
        std::mutex m_model_cache_mutex;
        std::mutex m_model_vector_mutex;
        //</f> /Models

        //<f> Materials
        std::map<std::string, SDL_Surface*> m_loaded_surfaces;
        std::map<std::string, Texture> m_textures;

        Material m_default_material;
        std::map<std::string, Material> m_materials;
        //</f> /Materials
    private:
        //<f> Private Functions
        Material* LoadMaterial(aiMaterial* material_data);
        ModelDataCache* LoadCachedModel(const std::string& path);
        //</f> /Private Functions
};

#endif //RESOURCE_MAMAGER_HPP
