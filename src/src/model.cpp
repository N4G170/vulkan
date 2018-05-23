#include "model.hpp"
#include <utility>
#include <stdexcept>
#include <iostream>
#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE//because vulkan depth goes from 0 to 1 and gl used -1 to 1
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include "constants.hpp"
#include "vulkan_context.hpp"
#include "resource_manager.hpp"
#include "vulkan_utils.hpp"
#include "mesh.hpp"

//<f> Constructors & operator=
Model::Model(vk::VulkanPointers vulkan_pointers) : m_name{"model"}, m_vulkan_pointers{vulkan_pointers}, m_vulkan_objects_created{false} {}

Model::~Model() noexcept {}

Model::Model(Model&& other) noexcept : m_name{std::move(other.m_name)}, m_vulkan_pointers{std::move(other.m_vulkan_pointers)},
    m_meshes{std::move(other.m_meshes)}, m_vulkan_objects_created{std::move(other.m_vulkan_objects_created)}, m_uniform_buffer{std::move(other.m_uniform_buffer)},
    m_uniform_descriptor_set{std::move(other.m_uniform_descriptor_set)}, m_pipeline_type{std::move(other.m_pipeline_type)},
    m_position{std::move(other.m_position)}, m_rotation{std::move(other.m_rotation)}, m_scale{std::move(other.m_scale)} {}

Model& Model::operator=(Model&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
        m_name=std::move(other.m_name);
        m_vulkan_pointers = std::move(other.m_vulkan_pointers);
        m_meshes = std::move(other.m_meshes);
        m_vulkan_objects_created = std::move(other.m_vulkan_objects_created);
        m_uniform_buffer = std::move(other.m_uniform_buffer);
        m_uniform_descriptor_set = std::move(other.m_uniform_descriptor_set);
        m_pipeline_type = std::move(other.m_pipeline_type);
        m_position = std::move(other.m_position);
        m_rotation = std::move(other.m_rotation);
        m_scale = std::move(other.m_scale);
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
void Model::Cleanup()
{
    m_vulkan_pointers.memory_manager->DestroyBuffer(&m_uniform_buffer);

    for(auto& mesh : m_meshes)
        mesh.Cleanup();
}

void Model::CreateVulkanObjects()
{
    if(m_vulkan_objects_created)
        return;
    m_vulkan_objects_created = true;

    CreateUniformBuffer();
    CreateDescriptorSet();//needs uniform buffer

    for(auto& mesh : m_meshes)
    {
        mesh.CreateVertexBuffer();
        mesh.CreateIndexBuffer();
    }
}

void Model::CreateUniformBuffer()
{
    VkDeviceSize buffer_size{sizeof(vk::UniformBufferObject)};

    //Source of memory transfer
    m_vulkan_pointers.memory_manager->RequestBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &m_uniform_buffer);
}

void Model::CreateDescriptorSet()
{
    //<f> Create Descriptor set
    std::vector<VkDescriptorSetLayout> layouts{*m_vulkan_pointers.resources->DescriptorSetUniformLayout()};

    VkDescriptorSetAllocateInfo allocate_info{vk::DescriptiorSetAllocateInfo()};

    allocate_info.descriptorPool = *m_vulkan_pointers.resources->DescriptorPoolUniform();
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = layouts.data();

    if( vkAllocateDescriptorSets(*m_vulkan_pointers.logical_device, &allocate_info, &m_uniform_descriptor_set) != VK_SUCCESS )
        throw std::runtime_error("Failed to allocate descriptor set");
    //</f> /Create Descriptor set

    //<f> Descriptor data update

    std::vector<VkWriteDescriptorSet> write_descriptor = {vk::WriteDescriptorSet()};

    //<f> Uniform buffer
    //config buffer descriptor
    VkDescriptorBufferInfo buffer_config{vk::DescriptorBufferInfo()};
    buffer_config.buffer = m_uniform_buffer.buffer;
    buffer_config.offset = 0;
    buffer_config.range = sizeof(vk::UniformBufferObject);

    //update set
    write_descriptor[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor[0].dstBinding = 0;//shader binding index
    write_descriptor[0].dstSet = m_uniform_descriptor_set;//set to write to
    write_descriptor[0].dstArrayElement = 0;//no array == index 0

    write_descriptor[0].descriptorCount = 1;
    write_descriptor[0].pBufferInfo = &buffer_config;
    //</f> /Uniform buffer

    vkUpdateDescriptorSets(*m_vulkan_pointers.logical_device, static_cast<uint32_t>(write_descriptor.size()), write_descriptor.data(), 0, nullptr);//write to set or copy to set
    //</f> /Descriptor data update
}

void Model::UpdateUniformBuffer(float delta_time, Camera* camera, const vk::LightData& light)
{
    // static auto start_time{std::chrono::high_resolution_clock::now()};
    //
    // auto current_time{std::chrono::high_resolution_clock::now()};
    //
    // float time{ std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count() / 1000.f };

    vk::UniformBufferObject ubo{};

    glm::mat4 model{1.f};
    //translate
    model = glm::translate(model, m_position);
    //rotate
    model = glm::rotate(model, glm::radians(m_rotation.x), c_unit_vec_x);
    model = glm::rotate(model, glm::radians(m_rotation.y), c_unit_vec_y);
    model = glm::rotate(model, glm::radians(m_rotation.z), c_unit_vec_z);
    // model = glm::rotate(model, glm::radians(180.f), c_unit_vec_z);
    //scale
    model = glm::scale(model, m_scale);

    ubo.model = model;
    ubo.view = camera->View();
    ubo.projection = camera->Projection();
    ubo.normal_matrix = glm::inverseTranspose(ubo.view * ubo.model);
    // obj.projection[1][1] *= -1;//so we have y going up
    ubo.light_position = light.light_position;
    ubo.light_colour = light.light_colour;
    ubo.diffuse_intensity = light.diffuse_intensity;
    ubo.ambient_intensity = light.ambient_intensity;
    ubo.specular_intensity = light.specular_intensity;

    ubo.camera_direction = glm::vec4(camera->Direction(), .0f);

    void* data;
    m_vulkan_pointers.memory_manager->MapMemory(&m_uniform_buffer, &data);
    memcpy(data, &ubo, sizeof(ubo));
    m_vulkan_pointers.memory_manager->UnmapMemory(&m_uniform_buffer);
}
//</f> /Methods

//<f> Getters/Setters
void Model::Name(const std::string& name)
{
    m_name = name;
}

std::string Model::Name() const
{
    return m_name;
}

size_t Model::MeshCount() const
{
    return m_meshes.size();
}
Mesh* Model::MeshAt(size_t index)
{
    if(index < m_meshes.size())
        return &m_meshes.at(index);
    return nullptr;
}

void Model::StoreMesh(Mesh&& mesh)
{
    m_meshes.push_back(std::move(mesh));
}

vk::Buffer* Model::UniformBuffer()
{
    return &m_uniform_buffer;
}

VkDescriptorSet* Model::DescriptorSet()
{
    return &m_uniform_descriptor_set;
}

vk::PipelineType Model::PipelineType() const
{
    return m_pipeline_type;
}
void Model::PipelineType(vk::PipelineType type)
{
    m_pipeline_type = type;
}

vk::VulkanModelData Model::CreateVulkanModelData()
{
    vk::VulkanModelData data;

    for(auto& mesh : m_meshes)
    {
        vk::VulkanMeshData mesh_data;
        mesh_data.vertex_buffer = mesh.VertexBuffer();
        mesh_data.index_buffer = mesh.IndexBuffer();
        mesh_data.index_vector_size = mesh.IndexVectorSize();
        mesh_data.sampler_descriptor_set = mesh.MeshMaterial()->DiffuseMap()->DescriptorSet();

        data.meshes_data.push_back(mesh_data);
    }
    data.uniform_descriptor_set = &m_uniform_descriptor_set;
    data.pipeline_type = m_pipeline_type;

    return data;
}

void Model::InitTransform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
{
    Position(position);
    Rotation(rotation);
    Scale(scale);
}

glm::vec3 Model::Position() const
{
    return m_position;
}

glm::vec3 Model::Rotation() const
{
    return m_rotation;
}

glm::vec3 Model::Scale() const
{
    return m_scale;
}

void Model::Position(const glm::vec3& position)
{
    m_position = position;
}

void Model::Rotation(const glm::vec3& rotation)
{
    m_rotation = rotation;
}

void Model::Scale(const glm::vec3& scale)
{
    m_scale = scale;
}
//</f> /Getters/Setters
