#include "pipelines.hpp"
#include <utility>
#include <vector>
#include "vulkan_utils.hpp"
#include "vulkan_context.hpp"
#include "swapchain.hpp"
#include "renderpass.hpp"
#include <iostream>

namespace vk
{
//<f> Constructors & operator=
Pipelines::Pipelines(VulkanContext* context): m_context{context}, m_create_infos{}, m_graphics_pipeline_create_info{}
{

}

Pipelines::~Pipelines() noexcept
{

}

Pipelines::Pipelines(Pipelines&& other) noexcept
{

}

Pipelines& Pipelines::operator=(Pipelines&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
void Pipelines::Cleanup()
{
    for(auto& module : m_shader_modules)
        vkDestroyShaderModule(*m_context->LogicalDevice(), module.second, nullptr);
    m_shader_modules.clear();

    for(auto& pipeline : m_graphics_pipelines)
        vkDestroyPipeline(*m_context->LogicalDevice(), pipeline.second, nullptr);
    m_graphics_pipelines.clear();

    vkDestroyPipelineLayout(*m_context->LogicalDevice(), m_graphics_layout, nullptr);//the pipeline uses this so it is destroyed after the pipeline
    m_graphics_layout = VK_NULL_HANDLE;
}

void Pipelines::Init(VulkanResources* vulkan_resources, Swapchain* swapchain, Renderpass* renderpass)
{
    CreateGraphicsLayout(vulkan_resources);
    CreateGraphicsCreateInfo(swapchain, renderpass);
    CreateGraphicsMain();
    CreateGraphicsDerivatives();

    m_init_ok = true;
}
//</f> /Methods

//<f> Getters/Setters
VkPipeline* Pipelines::Pipeline(PipelineType type)
{
    if(m_graphics_pipelines.find(type) != std::end(m_graphics_pipelines))
        return &m_graphics_pipelines[type];
    return nullptr;
}
//</f> /Getters/Setters

//<f> Private Methods
void Pipelines::CreateGraphicsLayout(VulkanResources* vulkan_resources)
{
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{vk::PipelineLayoutCreateInfo()};

    std::vector<VkDescriptorSetLayout> layouts{*vulkan_resources->DescriptorSetUniformLayout(), *vulkan_resources->DescriptorSetSamplerLayout()};

    pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(layouts.size());
    pipeline_layout_create_info.pSetLayouts = layouts.data();

    if(vkCreatePipelineLayout(*m_context->LogicalDevice(), &pipeline_layout_create_info, nullptr, &m_graphics_layout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout");
}

void Pipelines::CreateGraphicsCreateInfo(Swapchain* swapchain, Renderpass* renderpass)
{
    for(auto& pipeline : m_graphics_pipelines)
        pipeline.second = VK_NULL_HANDLE;

    //<f> Vertex Input
    m_create_infos.vertex_input_create_info = vk::PipelineVertexInputStateCreateInfo();

    m_create_infos.vertex_input_binding_description = Vertex::CreateBindingDescription();
    m_create_infos.vertex_input_attribute_descriptions = Vertex::CreateAttributeDescription();

    m_create_infos.vertex_input_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_create_infos.vertex_input_attribute_descriptions.size());
    m_create_infos.vertex_input_create_info.pVertexAttributeDescriptions = m_create_infos.vertex_input_attribute_descriptions.data();

    m_create_infos.vertex_input_create_info.vertexBindingDescriptionCount = 1;
    m_create_infos.vertex_input_create_info.pVertexBindingDescriptions = &m_create_infos.vertex_input_binding_description;

    m_create_infos.input_assembly_create_info = vk::PipelineInputAssemblyStateCreateInfo();

    // input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    // input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    m_create_infos.input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    m_create_infos.input_assembly_create_info.primitiveRestartEnable = VK_FALSE;
    //</f> /Vertex Input

    //<f> Viewport & scissors
    m_create_infos.viewport = {};//like srouce rect
    m_create_infos.viewport.x = 0;
    m_create_infos.viewport.y = 0;
    m_create_infos.viewport.width = swapchain->Extent()->width;
    m_create_infos.viewport.height = swapchain->Extent()->height;
    m_create_infos.viewport.minDepth = 0.f;//default
    m_create_infos.viewport.maxDepth = 1.f;//default

    m_create_infos.scissors = {};//like destination rect
    m_create_infos.scissors.offset = {0,0};
    m_create_infos.scissors.extent = *swapchain->Extent();

    m_create_infos.viewport_state_create_info = vk::PipelineViewportStateCreateInfo();

    m_create_infos.viewport_state_create_info.viewportCount = 1;
    m_create_infos.viewport_state_create_info.pViewports = &m_create_infos.viewport;
    m_create_infos.viewport_state_create_info.scissorCount = 1;
    m_create_infos.viewport_state_create_info.pScissors = &m_create_infos.scissors;
    //</f> /Viewport & scissors

    //<f> Rasterizer
    m_create_infos.rasterizer_create_info = vk::PipelineRasterizationStateCreateInfo();
    m_create_infos.rasterizer_create_info.depthClampEnable = VK_FALSE;//True needs enabling gpu feature
    m_create_infos.rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;//if true all geometry is discarded, basically disables any output to the framebuffer

    m_create_infos.rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;//fill, line(wireframe), point(vectex only). Other than fill needs enabling gpu feature
    m_create_infos.rasterizer_create_info.lineWidth = 1.0f;//bigger than 1 needs enabling gpu feature (wideLines)

    m_create_infos.rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    // m_create_infos.rasterizer_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;//vertex order for faces to be considered front-facing (counter-clock because of y flip)
    m_create_infos.rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;//vertex order for faces to be considered front-facing

    //alter the depth values by adding a constant value or biasing them based on a fragment's slope
    //can be used for shadow mapping
    m_create_infos.rasterizer_create_info.depthBiasEnable = VK_FALSE;
    m_create_infos.rasterizer_create_info.depthBiasConstantFactor = 0;
    m_create_infos.rasterizer_create_info.depthBiasClamp = 0;
    m_create_infos.rasterizer_create_info.depthBiasSlopeFactor = 0;
    //</f> /Rasterizer

    //<f> Multisampling
    m_create_infos.multisampling_create_info = vk::PipelineMultisampleStateCreateInfo();//not in use, for now

    m_create_infos.multisampling_create_info.sampleShadingEnable = VK_FALSE;
    m_create_infos.multisampling_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    m_create_infos.multisampling_create_info.minSampleShading = 1.0f; // Optional
    m_create_infos.multisampling_create_info.pSampleMask = nullptr; // Optional
    m_create_infos.multisampling_create_info.alphaToCoverageEnable = VK_FALSE; // Optional
    m_create_infos.multisampling_create_info.alphaToOneEnable = VK_FALSE; // Optional
    //</f> /Multisampling

    //<f> Colour Blending
    //need more studying
    m_create_infos.colour_blend_attachment = {};
    m_create_infos.colour_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    m_create_infos.colour_blend_attachment.blendEnable = VK_FALSE;
    m_create_infos.colour_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    m_create_infos.colour_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    m_create_infos.colour_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    m_create_infos.colour_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    m_create_infos.colour_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    m_create_infos.colour_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    //need more studying
    m_create_infos.colour_blend_create_info = vk::PipelineColorBlendStateCreateInfo();

    m_create_infos.colour_blend_create_info.logicOpEnable = VK_FALSE;
    m_create_infos.colour_blend_create_info.logicOp = VK_LOGIC_OP_COPY; // Optional
    m_create_infos.colour_blend_create_info.attachmentCount = 1;
    m_create_infos.colour_blend_create_info.pAttachments = &m_create_infos.colour_blend_attachment;
    m_create_infos.colour_blend_create_info.blendConstants[0] = 0.0f; // Optional
    m_create_infos.colour_blend_create_info.blendConstants[1] = 0.0f; // Optional
    m_create_infos.colour_blend_create_info.blendConstants[2] = 0.0f; // Optional
    m_create_infos.colour_blend_create_info.blendConstants[3] = 0.0f; // Optional
    //</f> /Colour Blending

    //<f> Depth and stencil
    m_create_infos.depth_stencil_create_info = vk::PipelineDepthStencilStateCreateInfo();

    m_create_infos.depth_stencil_create_info.depthTestEnable = VK_TRUE;
    m_create_infos.depth_stencil_create_info.depthWriteEnable = VK_TRUE;

    m_create_infos.depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;//smaller depth = closer

    m_create_infos.depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
    m_create_infos.depth_stencil_create_info.stencilTestEnable = VK_FALSE;
    //</f> /Depth and stencil

    //<f> Pipeline proper
    m_graphics_pipeline_create_info = vk::GraphicsPipelineCreateInfo();
    m_graphics_pipeline_create_info.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;


    m_graphics_pipeline_create_info.pVertexInputState = &m_create_infos.vertex_input_create_info;
    m_graphics_pipeline_create_info.pInputAssemblyState = &m_create_infos.input_assembly_create_info;
    m_graphics_pipeline_create_info.pViewportState = &m_create_infos.viewport_state_create_info;
    m_graphics_pipeline_create_info.pRasterizationState = &m_create_infos.rasterizer_create_info;
    m_graphics_pipeline_create_info.pMultisampleState = &m_create_infos.multisampling_create_info;
    m_graphics_pipeline_create_info.pDepthStencilState = &m_create_infos.depth_stencil_create_info;
    m_graphics_pipeline_create_info.pColorBlendState = &m_create_infos.colour_blend_create_info;
    m_graphics_pipeline_create_info.pDynamicState = nullptr; // not in use

    m_graphics_pipeline_create_info.layout = m_graphics_layout;

    m_graphics_pipeline_create_info.renderPass = *renderpass->RenderPass();
    m_graphics_pipeline_create_info.subpass = 0;//subpass index

    //for use when we need to create another pipeline similar to the one being referenced
    //VK_PIPELINE_CREATE_DERIVATIVE_BIT must be set in 'flags' struct field
    m_graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    m_graphics_pipeline_create_info.basePipelineIndex = -1;
    //</f> /Pipeline proper
}

void Pipelines::CreateGraphicsMain()
{
    //<f> Load Diffuse Shader
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages(2);
    //create pipeline shader
    shader_stages[0] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/diffuse.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, m_shader_modules);
    shader_stages[1] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/diffuse.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, m_shader_modules);

    // shader_stages[0] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/diffuse.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, &vert_module);
    // shader_stages[1] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/diffuse.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, &frag_module);

    m_graphics_pipeline_create_info.stageCount = shader_stages.size();//shader stages
    m_graphics_pipeline_create_info.pStages = shader_stages.data();

    if( vkCreateGraphicsPipelines(*m_context->LogicalDevice(), VK_NULL_HANDLE, 1, &m_graphics_pipeline_create_info, nullptr, &m_graphics_pipelines[PipelineType::DIFFUSE]) != VK_SUCCESS )
        throw std::runtime_error("Failed to create DIFFUSE graphics pipeline");

    //</f> /Load Diffuse Shader
}

void Pipelines::CreateGraphicsDerivatives()
{
    //now load derivatives
    m_graphics_pipeline_create_info.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
    //we can pnlçy use the handle or the index
    m_graphics_pipeline_create_info.basePipelineHandle = m_graphics_pipelines[PipelineType::DIFFUSE];
    m_graphics_pipeline_create_info.basePipelineIndex = -1;//we use the hande

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages(2);
    m_graphics_pipeline_create_info.stageCount = shader_stages.size();//shader stages
    m_graphics_pipeline_create_info.pStages = shader_stages.data();

    //<f> SECULAR
    shader_stages[0] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/specular.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, m_shader_modules);
    shader_stages[1] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/specular.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, m_shader_modules);

    if( vkCreateGraphicsPipelines(*m_context->LogicalDevice(), VK_NULL_HANDLE, 1, &m_graphics_pipeline_create_info, nullptr, &m_graphics_pipelines[PipelineType::SPECULAR]) != VK_SUCCESS )
        throw std::runtime_error("Failed to create SPECULAR graphics pipeline");
    //</f> /SECULAR

    //<f> TOON
    shader_stages[0] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/toon.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, m_shader_modules);
    shader_stages[1] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/toon.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, m_shader_modules);

    if( vkCreateGraphicsPipelines(*m_context->LogicalDevice(), VK_NULL_HANDLE, 1, &m_graphics_pipeline_create_info, nullptr, &m_graphics_pipelines[PipelineType::TOON]) != VK_SUCCESS )
        throw std::runtime_error("Failed to create TOON graphics pipeline");
    //</f> /TOON

    //<f> UI
    shader_stages[0] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/ui.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, m_shader_modules);
    shader_stages[1] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/ui.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, m_shader_modules);

    if( vkCreateGraphicsPipelines(*m_context->LogicalDevice(), VK_NULL_HANDLE, 1, &m_graphics_pipeline_create_info, nullptr, &m_graphics_pipelines[PipelineType::UI]) != VK_SUCCESS )
        throw std::runtime_error("Failed to create UI graphics pipeline");
    //</f> /UI

    //<f> Wireframe
    m_create_infos.rasterizer_create_info.polygonMode = VK_POLYGON_MODE_LINE;//wireframe
    shader_stages[0] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/wireframe.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, m_shader_modules);
    shader_stages[1] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/wireframe.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, m_shader_modules);

    if( vkCreateGraphicsPipelines(*m_context->LogicalDevice(), VK_NULL_HANDLE, 1, &m_graphics_pipeline_create_info, nullptr, &m_graphics_pipelines[PipelineType::WIREFRAME]) != VK_SUCCESS )
        throw std::runtime_error("Failed to create WIREFRAME graphics pipeline");
    //</f> /Wireframe

    //<f> Vertex Points
    m_create_infos.rasterizer_create_info.polygonMode = VK_POLYGON_MODE_POINT;//points only
    shader_stages[0] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/wireframe.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, m_shader_modules);
    shader_stages[1] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/wireframe.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, m_shader_modules);

    if( vkCreateGraphicsPipelines(*m_context->LogicalDevice(), VK_NULL_HANDLE, 1, &m_graphics_pipeline_create_info, nullptr, &m_graphics_pipelines[PipelineType::POINTS]) != VK_SUCCESS )
        throw std::runtime_error("Failed to create WIREFRAME graphics pipeline");
    //</f> /Vertex Points

    //<f> NORMALS_DEBUG
    // shader_stages.resize(3);
    // shader_stages[0] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/normals_debug.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, m_shader_modules);
    // shader_stages[1] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/normals_debug.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, m_shader_modules);
    // shader_stages[2] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/normals_debug.geom.spv", VK_SHADER_STAGE_GEOMETRY_BIT, m_shader_modules);
    // m_create_infos.rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    // m_graphics_pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());//shader stages
    // m_graphics_pipeline_create_info.pStages = shader_stages.data();
    //
    // if( vkCreateGraphicsPipelines(*m_context->LogicalDevice(), VK_NULL_HANDLE, 1, &m_graphics_pipeline_create_info, nullptr, &m_graphics_pipelines[PipelineType::NORMALS_DEBUG]) != VK_SUCCESS )
    //     throw std::runtime_error("Failed to create NORMALS_DEBUG graphics pipeline");
    //</f> /NORMALS_DEBUG

    //<f> Skybox
    m_create_infos.rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    m_create_infos.rasterizer_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    shader_stages[0] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, m_shader_modules);
    shader_stages[1] = vk::LoadShaderFile(m_context->LogicalDevice(), "data/shaders/skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, m_shader_modules);

    if( vkCreateGraphicsPipelines(*m_context->LogicalDevice(), VK_NULL_HANDLE, 1, &m_graphics_pipeline_create_info, nullptr, &m_graphics_pipelines[PipelineType::SKYBOX]) != VK_SUCCESS )
        throw std::runtime_error("Failed to create WIREFRAME graphics pipeline");
    //</f> /Skybox
}
//</f> /Private Methods

}//namespace vk
