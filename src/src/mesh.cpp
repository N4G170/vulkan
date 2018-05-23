#include "mesh.hpp"
#include <utility>
#include <iostream>

//<f> Constructors & operator=
Mesh::Mesh(vk::VulkanPointers vulkan_pointers): m_vulkan_pointers{vulkan_pointers}
{

}

Mesh::~Mesh() noexcept
{

}

Mesh::Mesh(Mesh&& other) noexcept : m_vulkan_pointers{std::move(other.m_vulkan_pointers)}, m_material{std::move(other.m_material)}, m_vertices{std::move(other.m_vertices)},
    m_indices{std::move(other.m_indices)}, m_vertex_buffer{std::move(other.m_vertex_buffer)}, m_index_buffer{std::move(other.m_index_buffer)}
{

}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
        m_vulkan_pointers = std::move(other.m_vulkan_pointers);
        m_material = std::move(other.m_material);
        m_vertices = std::move(other.m_vertices);
        m_indices = std::move(other.m_indices);
        m_vertex_buffer = std::move(other.m_vertex_buffer);
        m_index_buffer = std::move(other.m_index_buffer);
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
void Mesh::Cleanup()
{
    m_vulkan_pointers.memory_manager->DestroyBuffer(&m_vertex_buffer);
    m_vulkan_pointers.memory_manager->DestroyBuffer(&m_index_buffer);
}

void Mesh::CreateVertexBuffer()
{
    // VkDeviceSize buffer_size {sizeof(vertices[0]) * vertices.size()};
    VkDeviceSize buffer_size {m_vertices.size() * sizeof(m_vertices[0])};
    vk::Buffer staging_buffer;

    //Source of memory transfer
    m_vulkan_pointers.memory_manager->RequestBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer);

    //<f> Mapping
    void* data;
    m_vulkan_pointers.memory_manager->MapMemory(&staging_buffer, &data);
    memcpy(data, m_vertices.data(), static_cast<size_t>(buffer_size));
    m_vulkan_pointers.memory_manager->UnmapMemory(&staging_buffer);
    //</f> /Mapping

    m_vulkan_pointers.memory_manager->RequestBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_vertex_buffer);

    m_vulkan_pointers.commandbuffer->CopyBuffer(staging_buffer.buffer, m_vertex_buffer.buffer, buffer_size, 0, 0);

    //free staging vars
    m_vulkan_pointers.memory_manager->DestroyBuffer(&staging_buffer);
}
void Mesh::CreateIndexBuffer()
{
    // VkDeviceSize buffer_size{sizeof(vertex_indices[0]) * vertex_indices.size()};
    VkDeviceSize buffer_size{m_indices.size() * sizeof(m_indices[0])};

    vk::Buffer staging_buffer{};
    m_vulkan_pointers.memory_manager->RequestBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &staging_buffer);

    //<f> Mapping
    void* data;
    m_vulkan_pointers.memory_manager->MapMemory(&staging_buffer, &data);
    memcpy(data, m_indices.data(), static_cast<size_t>(buffer_size));
    m_vulkan_pointers.memory_manager->UnmapMemory(&staging_buffer);
    //</f> /Mapping

    m_vulkan_pointers.memory_manager->RequestBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_index_buffer);

    m_vulkan_pointers.commandbuffer->CopyBuffer(staging_buffer.buffer, m_index_buffer.buffer, buffer_size, 0, 0);

    //free staging vars
    m_vulkan_pointers.memory_manager->DestroyBuffer(&staging_buffer);
}
//</f> /Methods

//<f> Getters/Setters
void Mesh::Name(const std::string& name)
{
    m_name = name;
}

std::string Mesh::Name()
{
    return m_name;
}

void Mesh::AddVertex(vk::Vertex vertex)
{
    m_vertices.push_back(vertex);
}

void Mesh::AddIndex(uint32_t index)
{
    m_indices.push_back(index);
}

vk::Buffer* Mesh::VertexBuffer()
{
    return &m_vertex_buffer;
}

vk::Buffer* Mesh::IndexBuffer()
{
    return &m_index_buffer;
}

Material* Mesh::MeshMaterial() const
{
    return m_material;
}

void Mesh::MeshMaterial(Material* material)
{
    m_material = material;
}
//</f> /Getters/Setters
