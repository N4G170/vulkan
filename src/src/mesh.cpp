#include "mesh.hpp"
#include <utility>
#include <stdexcept>
#include <iostream>
#include <unordered_map>

//<f> Constructors & operator=
Mesh::Mesh()
{

}

Mesh::~Mesh() noexcept
{

}

Mesh::Mesh(const Mesh& other)
{

}

Mesh::Mesh(Mesh&& other) noexcept
{

}

Mesh& Mesh::operator=(const Mesh& other)
{
    if(this != &other)//not same ref
    {
        auto tmp(other);
        *this = std::move(tmp);
    }

    return *this;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
void Mesh::Load(const std::string& path)
{
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string error;

    if( !tinyobj::LoadObj(&attributes, &shapes, &materials, &error, path.data()) )
        throw std::runtime_error(error);

    // auto size{attributes.vertices.size() / 3};//each vertex uses 3 positions each -> total number of vertices
    // std::vector<bool> used_vertices( size, false );//to check if we already stored a vertex
    std::unordered_map<int, int> used_vertices{};

    // m_vertices.resize( size );


    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            if(used_vertices[index.vertex_index] == 0)//not stored (if index is 0 we repeat the insert)
            {
                Vertex vertex{};

                vertex.position =
                {
                    attributes.vertices[3 * index.vertex_index + 0],
                    attributes.vertices[3 * index.vertex_index + 1],
                    attributes.vertices[3 * index.vertex_index + 2]
                };

                vertex.texture_coordinate =
                {
                    attributes.texcoords[2 * index.texcoord_index + 0],
                    -attributes.texcoords[2 * index.texcoord_index + 1]
                    // 1.0f - attributes.texcoords[2 * index.texcoord_index + 1]
                };

                used_vertices[index.vertex_index] = static_cast<int>(m_vertices.size());//store index position on vertex vector
                m_vertices.push_back(vertex);
            }

            m_indices.push_back(used_vertices[index.vertex_index]);
        }
    }

    // m_vertices.push_back({{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}});
    // m_vertices.push_back({{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}});
    // m_vertices.push_back({{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}});
    // m_vertices.push_back({{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}});
    //
    // m_indices.push_back(0);
    // m_indices.push_back(1);
    // m_indices.push_back(2);
    // m_indices.push_back(2);
    // m_indices.push_back(3);
    // m_indices.push_back(0);

    // for(auto i{0}; i < shapes.size(); ++i)
    // {
    //     auto shape{shapes[i]};
    //
    //     //run through all of this face's indices
    //     // for(auto j{0}; j<)
    // }

    // for(auto i{0}; i<size; ++i)//size is ajusted to 3 positions per vertex
    // {
    //     //<f> Indices
    //     int ix{ i * 3 + 0};
    //     int iy{ i * 3 + 1};
    //     int iz{ i * 3 + 2};
    //     int i_texture_x{ i * 2 + 0};
    //     int i_texture_y{ i * 2 + 1};
    //     //</f> /Indices
    //
    //     //<f> vectors
    //     glm::vec3 vertex{attributes.vertices[ix], attributes.vertices[iy], attributes.vertices[iz]};
    //     glm::vec3 normal{attributes.normals[ix], attributes.normals[iy], attributes.normals[iz]};
    //     glm::vec2 texture_coord{attributes.texcoords[i_texture_x], attributes.texcoords[i_texture_y]};
    //     //</f> /vectors
    //
    //     Vertex aux_vertex{};
    //     aux_vertex.position = vertex;
    //     aux_vertex.normal = normal;
    //     aux_vertex.texture_coordinate = texture_coord;
    //
    //     m_vertices[i] = aux_vertex;
    // }
    //
    // //indices
    // size = shapes.size();
    //
    // for(auto i{0}; i<size; ++i)
    // {
    //
    // }
}
//</f> /Methods

//<f> Getters/Setters
size_t Mesh::VertexBufferSizeNeeded() const
{
    return m_vertices.size() * sizeof(m_vertices[0]);
}

size_t Mesh::IndicesBufferSizeNeeded() const
{
    return m_indices.size() * sizeof(m_indices[0]);
}

Vertex* Mesh::VertexData()
{
    return m_vertices.data();
}

uint32_t* Mesh::IndicesData()
{
    return m_indices.data();
}

size_t Mesh::VertexVectorSize()
{
    return m_vertices.size();
}

size_t Mesh::IndicesVectorSize()
{
    return m_indices.size();
}
//</f> /Getters/Setters
