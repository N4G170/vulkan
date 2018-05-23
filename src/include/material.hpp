#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <string>
#include "texture.hpp"
#include <glm/glm.hpp>

class Material
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        Material();
        /** brief Default destructor */
        virtual ~Material() noexcept;

        /** brief Copy constructor */
        Material(const Material& other);
        /** brief Move constructor */
        Material(Material&& other) noexcept;

        /** brief Copy operator */
        Material& operator= (const Material& other);
        /** brief Move operator */
        Material& operator= (Material&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods

        //</f> /Methods

        //<f> Getters/Setters
        Texture* DiffuseMap() const;
        glm::vec4 AmbientColour() const;
        glm::vec4 DiffuseColour() const;
        glm::vec4 SpecularColour() const;
        glm::float_t SpecularWeight() const;

        void DiffuseMap(Texture*);
        void AmbientColour(glm::vec4);
        void DiffuseColour(glm::vec4);
        void SpecularColour(glm::vec4);
        void SpecularWeight(glm::float_t);
        //</f> /Getters/Setters

    protected:
        // vars and stuff

    private:
        //<f> Maps
        Texture* m_diffuse_map;
        //</f> /Maps

        //<f> Colours
        glm::vec4 m_ambient_colour;
        glm::vec4 m_diffuse_colour;
        glm::vec4 m_specular_colour;
        glm::float32 m_specular_weight;
        //</f> /Colours
};

#endif //MATERIAL_HPP
