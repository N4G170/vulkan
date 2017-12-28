#ifndef MESH_HPP
#define MESH_HPP

#include "vulkan_structs.hpp"
#include "texture.hpp"
#include <vector>
#include "tiny_obj_loader.h"
#include <string>

class Mesh
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        Mesh();
        /** brief Default destructor */
        virtual ~Mesh() noexcept;

        /** brief Copy constructor */
        Mesh(const Mesh& other);
        /** brief Move constructor */
        Mesh(Mesh&& other) noexcept;

        /** brief Copy operator */
        Mesh& operator= (const Mesh& other);
        /** brief Move operator */
        Mesh& operator= (Mesh&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Virtual Methods

        //</f> /Virtual Methods

        //<f> Methods
        void Load(const std::string& path);
        //</f> /Methods

        //<f> Getters/Setters

        //</f> /Getters/Setters

    protected:
        // vars and stuff

    private:
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        Texture* m_texture;
};

#endif //MESH_HPP
