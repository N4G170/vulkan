#include "material.hpp"
#include <utility>
#include <iostream>
#include "vulkan_context.hpp"
#include "vulkan_utils.hpp"

//<f> Constructors & operator=
Material::Material(): m_diffuse_map{nullptr}, m_ambient_colour{1}, m_diffuse_colour{1}, m_specular_colour{1}, m_specular_weight{0}
{

}

Material::~Material() noexcept
{
    // Cleanup();
}

Material::Material(const Material& other) : m_diffuse_map{other.m_diffuse_map}, m_ambient_colour{other.m_ambient_colour}, m_diffuse_colour{other.m_diffuse_colour},
    m_specular_colour{other.m_specular_colour}, m_specular_weight{other.m_specular_weight}
{

}

Material::Material(Material&& other) noexcept: m_diffuse_map{std::move(other.m_diffuse_map)}, m_ambient_colour{std::move(other.m_ambient_colour)},
    m_diffuse_colour{std::move(other.m_diffuse_colour)}, m_specular_colour{std::move(other.m_specular_colour)}, m_specular_weight{std::move(other.m_specular_weight)}
{

}

Material& Material::operator=(const Material& other)
{
    if(this != &other)//not same ref
    {
        auto tmp(other);
        *this = std::move(tmp);
    }

    return *this;
}

Material& Material::operator=(Material&& other) noexcept
{
    if(this != &other)//not same ref
    {
        m_diffuse_map = std::move(other.m_diffuse_map);
        m_ambient_colour = std::move(other.m_ambient_colour);
        m_diffuse_colour = std::move(other.m_diffuse_colour);
        m_specular_colour = std::move(other.m_specular_colour);
        m_specular_weight = std::move(other.m_specular_weight);
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods

//</f> /Methods

//<f> Getters/Setters
Texture* Material::DiffuseMap() const
{
    return m_diffuse_map;
}

glm::vec4 Material::AmbientColour() const
{
    return m_ambient_colour;
}

glm::vec4 Material::DiffuseColour() const
{
    return m_diffuse_colour;
}

glm::vec4 Material::SpecularColour() const
{
    return m_specular_colour;
}

glm::float_t Material::SpecularWeight() const
{
    return m_specular_weight;
}

void Material::DiffuseMap(Texture* texture)
{
    m_diffuse_map = texture;
}
void Material::AmbientColour(glm::vec4 colour)
{
    m_ambient_colour = colour;
}
void Material::DiffuseColour(glm::vec4 colour)
{
    m_diffuse_colour = colour;
}
void Material::SpecularColour(glm::vec4 colour)
{
    m_specular_colour = colour;
}
void Material::SpecularWeight(glm::float_t weight)
{
    m_specular_weight = weight;
}
//</f> /Getters/Setters
