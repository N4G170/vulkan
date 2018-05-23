#ifndef MESH_HPP
#define MESH_HPP

#include "material.hpp"
#include "vulkan_pointers.hpp"
#include <string>

class Mesh
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        explicit Mesh(vk::VulkanPointers);
        /** brief Default destructor */
        virtual ~Mesh() noexcept;

        /** brief Copy constructor */
        Mesh(const Mesh& other) = delete;
        /** brief Move constructor */
        Mesh(Mesh&& other) noexcept;

        /** brief Copy operator */
        Mesh& operator= (const Mesh& other) = delete;
        /** brief Move operator */
        Mesh& operator= (Mesh&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        void Cleanup();
        void CreateVertexBuffer();
        void CreateIndexBuffer();
        //</f> /Methods

        //<f> Getters/Setters
        void Name(const std::string& name);
        std::string Name();

        void AddVertex(vk::Vertex);
        void AddIndex(uint32_t);

        vk::Buffer* VertexBuffer();
        vk::Buffer* IndexBuffer();
        uint32_t IndexVectorSize() { return m_indices.size(); }

        Material* MeshMaterial() const;
        void MeshMaterial(Material*);
        //</f> /Getters/Setters

    protected:
        vk::VulkanPointers m_vulkan_pointers;
        std::string m_name;

        // vars and stuff
        Material* m_material;
        std::vector<vk::Vertex> m_vertices;
        std::vector<uint32_t> m_indices;

        vk::Buffer m_vertex_buffer;
        vk::Buffer m_index_buffer;
    private:
};

#endif //MESH_HPP
