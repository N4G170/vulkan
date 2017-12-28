#include "mesh.hpp"
#include <utility>
#include <stdexcept>
#include <iostream>

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

    auto size{attributes.vertices.size() / 3};//each vertex uses 3 positions each -> total number of vertices
    std::vector<bool> used_vertices( size, false );//to check if we already stored a vertex

    m_vertices.resize( size );

    for(auto i{0}; i < shapes.size(); ++i)
    {
        auto shape{shapes[i]};

        //run through all of this face's indices
        // for(auto j{0}; j<)
    }

    for(auto i{0}; i<size; ++i)//size is ajusted to 3 positions per vertex
    {
        //<f> Indices
        int ix{ i * 3 + 0};
        int iy{ i * 3 + 1};
        int iz{ i * 3 + 2};
        int i_texture_x{ i * 2 + 0};
        int i_texture_y{ i * 2 + 1};
        //</f> /Indices

        //<f> vectors
        glm::vec3 vertex{attributes.vertices[ix], attributes.vertices[iy], attributes.vertices[iz]};
        glm::vec3 normal{attributes.normals[ix], attributes.normals[iy], attributes.normals[iz]};
        glm::vec2 texture_coord{attributes.texcoords[i_texture_x], attributes.texcoords[i_texture_y]};
        //</f> /vectors

        Vertex aux_vertex{};
        aux_vertex.position = vertex;
        aux_vertex.normal = normal;
        aux_vertex.texture_coordinate = texture_coord;

        m_vertices[i] = aux_vertex;
    }

    //indices
    size = shapes.size();

    for(auto i{0}; i<size; ++i)
    {

    }
}
//</f> /Methods
