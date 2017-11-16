#ifndef VULKAN_STRUCTS_HPP
#define VULKAN_STRUCTS_HPP

#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>
#include <vector>
#include <cstddef>

struct QueueFamiliesIndices
{
    int graphics_family{-1};
    int presentation_family{-1};

    /**
     * \brief Checks is all queue families indices are set(>=0)
     */
    bool AllFound(){ return graphics_family >= 0 && presentation_family >= 0; }

    std::vector<int> ToVector(){ return {graphics_family, presentation_family}; }
    std::vector<uint32_t> ToVectorUInt32(){ return {static_cast<uint32_t>(graphics_family), static_cast<uint32_t>(presentation_family)}; }
};

struct SwapChainSupportStruct
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats{};
    std::vector<VkPresentModeKHR> presentation_modes{};

    bool SwapChainIsCapable(){ return !formats.empty() && !presentation_modes.empty(); }
};

struct Vertex
{
    glm::vec2 position;
    glm::vec3 colour;

    static VkVertexInputBindingDescription CreateBindingDescrioption()
    {
        VkVertexInputBindingDescription description{};

        description.binding = 0;
        description.stride = sizeof(Vertex);
        description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return description;
    }

    static std::vector<VkVertexInputAttributeDescription> CreateAttributeDescription()
    {
        std::vector<VkVertexInputAttributeDescription> description(2);

        //position
        description[0].binding = 0;
        description[0].location = 0;
        description[0].format = VK_FORMAT_R32G32_SFLOAT;//two values 32bit single floats
        description[0].offset = offsetof(Vertex, position);

        //colour
        description[1].binding = 0;
        description[1].location = 1;
        description[1].format = VK_FORMAT_R32G32B32_SFLOAT;//3 values 32bit single floats
        description[1].offset = offsetof(Vertex, colour);

        return description;
    }
};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

//Vertices
const float wx = 0.1136;
const float wy = 0.08;
const float hx = 0.202;
const float hy = 0.1428;
const float ox = -0.0568;
const float oy = -0.2439;

const float leftx = -0.5;
const float rightx = 0.5;

const std::vector<Vertex> vertices
{
    { {ox          , oy},                   {1.0f, 0.0f, 0.0f} },
    { {ox + wx     , oy},                   {1.0f, 1.0f, 0.0f} },
    { {ox + wx + wy, oy + hy},              {1.0f, 1.0f, 1.0f} },
    { {ox + wx + wy, oy + hy + hx},         {0.0f, 1.0f, 1.0f} },
    { {ox + wx     , oy + hy + hx + hy},    {0.0f, 0.0f, 1.0f} },
    { {ox          , oy + hy + hx + hy},    {0.0f, 1.0f, 0.0f} },
    { {ox - wy     , oy + hy + hx},         {1.0f, 0.0f, 1.0f} },
    { {ox - wy     , oy + hy},              {1.0f, 1.0f, 1.0f} },

    { {ox + leftx          , oy},                   {1.0f, 0.0f, 0.0f} },
    { {ox + wx      + leftx, oy},                   {1.0f, 1.0f, 0.0f} },
    { {ox + wx + wy + leftx, oy + hy},              {1.0f, 1.0f, 1.0f} },
    { {ox + wx + wy + leftx, oy + hy + hx},         {0.0f, 1.0f, 1.0f} },
    { {ox + wx + leftx     , oy + hy + hx + hy},    {0.0f, 0.0f, 1.0f} },
    { {ox + leftx          , oy + hy + hx + hy},    {0.0f, 1.0f, 0.0f} },
    { {ox - wy + leftx     , oy + hy + hx},         {1.0f, 0.0f, 1.0f} },
    { {ox - wy + leftx     , oy + hy},              {1.0f, 1.0f, 1.0f} },

    { {ox + rightx          , oy},                   {1.0f, 0.0f, 0.0f} },
    { {ox + wx      + rightx, oy},                   {1.0f, 1.0f, 0.0f} },
    { {ox + wx + wy + rightx, oy + hy},              {1.0f, 1.0f, 1.0f} },
    { {ox + wx + wy + rightx, oy + hy + hx},         {0.0f, 1.0f, 1.0f} },
    { {ox + wx + rightx     , oy + hy + hx + hy},    {0.0f, 0.0f, 1.0f} },
    { {ox + rightx          , oy + hy + hx + hy},    {0.0f, 1.0f, 0.0f} },
    { {ox - wy + rightx     , oy + hy + hx},         {1.0f, 0.0f, 1.0f} },
    { {ox - wy + rightx     , oy + hy},              {1.0f, 1.0f, 1.0f} }

};

//uint type depends on max number of vertices and combinations
const std::vector<uint16_t> vertex_indices
{
    0,1,2,2,3,4,4,5,6,6,7,0,0,2,4,4,6,0,
    8,9,10,8,10,11,8,11,12,8,12,13,8,13,14,8,14,15,
    // 15,14,13,15,13,8,13,8,12,12,8,9,12,9,11,11,10,9//15,14,13 does not draw anything
    16,17,20,16,20,21,16,21,22,16,22,23,17,19,20,17,18,19
};

#endif//VULKAN_STRUCTS_HPP
