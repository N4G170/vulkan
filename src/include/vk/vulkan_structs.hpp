#ifndef VULKAN_STRUCTS_HPP
#define VULKAN_STRUCTS_HPP

#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>
#include <vector>
#include <cstddef>

namespace vk
{
enum class PipelineType
{
    DIFFUSE,
    POINTS,
    SPECULAR,
    WIREFRAME,
    TOON,
    NORMALS_DEBUG,
    SKYBOX,
    UI
};

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

struct SwapchainSupport
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats{};
    std::vector<VkPresentModeKHR> presentation_modes{};

    bool SwapchainIsCapable(){ return !formats.empty() && !presentation_modes.empty(); }
};

struct PipelineCreateInfos
{
    //auxilia vars
    VkVertexInputBindingDescription vertex_input_binding_description;
    std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions;

    //main vars
    VkPipelineVertexInputStateCreateInfo vertex_input_create_info;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info;
    VkViewport viewport;
    VkRect2D scissors;//like destination rect
    VkPipelineViewportStateCreateInfo viewport_state_create_info;
    VkPipelineRasterizationStateCreateInfo rasterizer_create_info;
    VkPipelineMultisampleStateCreateInfo multisampling_create_info;
    VkPipelineColorBlendAttachmentState colour_blend_attachment;
    VkPipelineColorBlendStateCreateInfo colour_blend_create_info;
    VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info;

    PipelineCreateInfos(){}
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;//texture_coord
    glm::vec3 diffuse_colour;
    glm::vec3 ambient_colour;
    glm::vec3 specular_colour;
    glm::float32 specular_weight;

    static VkVertexInputBindingDescription CreateBindingDescription()
    {
        VkVertexInputBindingDescription description{};

        description.binding = 0;
        description.stride = sizeof(Vertex);
        description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return description;
    }

    static std::vector<VkVertexInputAttributeDescription> CreateAttributeDescription()
    {
        std::vector<VkVertexInputAttributeDescription> description(7);

        //position
        description[0].binding = 0;
        description[0].location = 0;
        description[0].format = VK_FORMAT_R32G32B32_SFLOAT;//3 values 32bit single floats
        description[0].offset = offsetof(Vertex, position);

        //colour
        description[1].binding = 0;
        description[1].location = 1;
        description[1].format = VK_FORMAT_R32G32B32_SFLOAT;//3 values 32bit single floats
        description[1].offset = offsetof(Vertex, normal);

        //texture coordinate
        description[2].binding = 0;
        description[2].location = 2;
        description[2].format = VK_FORMAT_R32G32_SFLOAT;
        description[2].offset = offsetof(Vertex, uv);

        //diffuse colour
        description[3].binding = 0;
        description[3].location = 3;
        description[3].format = VK_FORMAT_R32G32B32_SFLOAT;//3 values 32bit single floats
        description[3].offset = offsetof(Vertex, diffuse_colour);

        //ambient colour
        description[4].binding = 0;
        description[4].location = 4;
        description[4].format = VK_FORMAT_R32G32B32_SFLOAT;//3 values 32bit single floats
        description[4].offset = offsetof(Vertex, ambient_colour);

        //specular_colour
        description[5].binding = 0;
        description[5].location = 5;
        description[5].format = VK_FORMAT_R32G32B32_SFLOAT;//3 values 32bit single floats
        description[5].offset = offsetof(Vertex, specular_colour);

        //specular_weight
        description[6].binding = 0;
        description[6].location = 6;
        description[6].format = VK_FORMAT_R32_SFLOAT;//1 value 32bit single float
        description[6].offset = offsetof(Vertex, specular_weight);

        return description;
    }
};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 normal_matrix;

    glm::vec4 camera_direction;

    glm::vec4 light_position;
    glm::vec4 light_colour;
    glm::float_t diffuse_intensity;
    glm::float_t ambient_intensity;
    glm::float_t specular_intensity;
};

struct GeometryBufferObject
{
    glm::mat4 model;
    glm::mat4 projection;
};

struct LightData
{
    glm::vec4 light_position;
    glm::vec4 light_colour;
    glm::float_t diffuse_intensity;
    glm::float_t ambient_intensity;
    glm::float_t specular_intensity;
};

/**
 * \brief Simple encaplulation of a VkBuffer (no memory management)
 */
struct Buffer
{
    /**
     * \brief Id of the block "owning" this buffer
     */
    uint32_t block_id;

    /**
     * \brief The buffer handle
     */
    VkBuffer buffer;

    /**
     * \brief The offset of this buffer in its memory block.
     */
    VkDeviceSize offset;

    /**
     * \brief The size of the buffer (in bytes). Stored to avoid querying for it.
     */
    VkDeviceSize size;

    /**
     * \brief Usage flags of this buffer. Stored to avoid querying for them.
     */
    VkBufferUsageFlags usage_flags;

    Buffer() : block_id{0}, buffer{VK_NULL_HANDLE}, offset{0}, size{0}, usage_flags{0} {}
};

/**
 * \brief Simple encaplulation of a VkImage (no memory management)
 */
struct ImageBuffer
{
    /**
     * \brief Id of the block "owning" this buffer
     */
    uint32_t block_id;

    /**
     * \brief The buffer handle
     */
    VkImage image;

    /**
     * \brief The offset of this buffer in its memory block.
     */
    VkDeviceSize offset;

    /**
     * \brief The size of the buffer (in bytes). Stored to avoid querying for it.
     */
    VkDeviceSize size;

    /**
     * \brief Usage flags of this buffer. Stored to avoid querying for them.
     */
    VkBufferUsageFlags usage_flags;

    ImageBuffer() : block_id{0}, image{VK_NULL_HANDLE}, offset{0}, size{0}, usage_flags{0} {}
};

struct VulkanMeshData
{
    Buffer* vertex_buffer;
    Buffer* index_buffer;
    VkDescriptorSet* sampler_descriptor_set;
    uint32_t index_vector_size;
    VulkanMeshData():vertex_buffer{nullptr}, index_buffer{nullptr}, sampler_descriptor_set{nullptr}, index_vector_size{0} {}
};

/**
 * \brief Struct to pass model data for the creation of the command buffer
 */
struct VulkanModelData
{
    std::vector<VulkanMeshData> meshes_data;
    VkDescriptorSet* uniform_descriptor_set;
    PipelineType pipeline_type;

    VulkanModelData(): meshes_data{}, uniform_descriptor_set{nullptr}, pipeline_type{PipelineType::DIFFUSE} {}
};
}//namespace vk
#endif//VULKAN_STRUCTS_HPP
