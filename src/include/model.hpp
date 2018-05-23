#ifndef MODEL_HPP
#define MODEL_HPP

#include <vector>
#include <string>
#include <iostream>
#include "camera.hpp"
#include "mesh.hpp"
#include "vulkan_pointers.hpp"

class Model
{
    public:
        enum class Primitives
        {
            CUBE,
            SPHERE
        };

    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        explicit Model(vk::VulkanPointers);
        /** brief Default destructor */
        virtual ~Model() noexcept;

        /** brief Copy constructor */
        Model(const Model& other) = delete;
        /** brief Move constructor */
        Model(Model&& other) noexcept;

        /** brief Copy operator */
        Model& operator= (const Model& other) = delete;
        /** brief Move operator */
        Model& operator= (Model&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        void Cleanup();
        // void Load(const std::string& path);
        // void Load(Primitives primitive);
        // void LoadMaterial(const std::string& path);
        /**
         * \brief To be called AFTER both \see Load and \see LoadTexture to create the needed vulkan objects
         */
        void CreateVulkanObjects();

        void CreateUniformBuffer();
        void CreateDescriptorSet();
        void UpdateUniformBuffer(float delta_time, Camera* camera, const vk::LightData& light);
        //</f> /Methods

        //<f> Getters/Setters
        void Name(const std::string& name);
        std::string Name() const;

        size_t MeshCount() const;
        Mesh* MeshAt(size_t index);
        void StoreMesh(Mesh&&);

        vk::Buffer* UniformBuffer();
        VkDescriptorSet* DescriptorSet();

        vk::PipelineType PipelineType() const;
        void PipelineType(vk::PipelineType type);

        vk::VulkanModelData CreateVulkanModelData();

        void InitTransform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
        glm::vec3 Position() const;
        glm::vec3 Rotation() const;
        glm::vec3 Scale() const;
        void Position(const glm::vec3& position);
        void Rotation(const glm::vec3& rotation);
        void Scale(const glm::vec3& scale);
        //</f> /Getters/Setters

    protected:
        // vars and stuff

    private:
        std::string m_name;
        vk::VulkanPointers m_vulkan_pointers;
        std::vector<Mesh> m_meshes;
        bool m_vulkan_objects_created;

        //<f> Vulkan structs & Vars
        vk::Buffer m_uniform_buffer;
        VkDescriptorSet m_uniform_descriptor_set;
        vk::PipelineType m_pipeline_type;
        //</f> /Vulkan structs & Vars

        //<f> Transform vars
        glm::vec3 m_position;
        glm::vec3 m_rotation;
        glm::vec3 m_scale;
        //</f> /Transform vars
};

#endif //MODEL_HPP
