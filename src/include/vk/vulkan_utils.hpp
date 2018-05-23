#ifndef VULKAN_UTILS
#define VULKAN_UTILS

#include <vulkan/vulkan.h>
#include "utils.hpp"
#include <stdexcept>
#include <string>
#include <map>
#include <fstream>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <string>

namespace vk
{
    inline VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo()
    {
        VkDescriptorPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        return info;
    }

    inline VkDescriptorSetAllocateInfo DescriptiorSetAllocateInfo()
    {
        VkDescriptorSetAllocateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        info.pNext = nullptr;

        return info;
    }

    inline VkDescriptorBufferInfo DescriptorBufferInfo()
    {
        VkDescriptorBufferInfo config{};
        return config;
    }

    inline VkDescriptorImageInfo DescriptorImageInfo()
    {
        VkDescriptorImageInfo info{};
        return info;
    }

    inline VkCommandPoolCreateInfo CommandPoolCreateInfo()
    {
        VkCommandPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        return info;
    }

    inline VkCommandBufferAllocateInfo CommandBufferAllocateInfo()
    {
        VkCommandBufferAllocateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.pNext = nullptr;

        return info;
    }

    inline VkCommandBufferBeginInfo CommandBufferBeginInfo()
    {
        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.pNext = nullptr;
        return info;
    }

    inline VkRenderPassBeginInfo RenderPassBeginInfo()
    {
        VkRenderPassBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.pNext = nullptr;
        return info;
    }

    inline VkSemaphoreCreateInfo SemaphoreCreateInfo()
    {
        VkSemaphoreCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkSubmitInfo SubmitInfo()
    {
        VkSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.pNext = nullptr;
        return info;
    }

    inline VkPresentInfoKHR PresentInfoKHR()
    {
        VkPresentInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.pNext = nullptr;
        return info;
    }

    inline VkMemoryAllocateInfo MemoryAllocateInfo()
    {
        VkMemoryAllocateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.pNext = nullptr;
        return info;
    }

    inline VkImageCreateInfo ImageCreateInfo()
    {
        VkImageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkImageMemoryBarrier ImageMemoryBarrier()
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        return barrier;
    }

    inline VkImageViewCreateInfo ImageViewCreateInfo()
    {
        VkImageViewCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        //need these two or driver screams
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkSamplerCreateInfo SamplerCreateInfo()
    {
        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkInstanceCreateInfo InstanceCreateInfo()
    {
        VkInstanceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pNext  = nullptr;//need to be set to nullptr or vkCreateInstance seg faults
        return info;
    }

    inline VkDebugReportCallbackCreateInfoEXT DebugReportCallbackCreateInfoEXT()
    {
        VkDebugReportCallbackCreateInfoEXT info{};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        return info;
    }

    inline VkDeviceCreateInfo DeviceCreateInfo()
    {
        VkDeviceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        return info;
    }

    inline VkSwapchainCreateInfoKHR SwapchainCreateInfoKHR()
    {
        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        //need these or the driver screams at me (seg fault)
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    //<f> Pipeline
    inline VkShaderModuleCreateInfo ShaderModuleCreateInfo()
    {
        VkShaderModuleCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        //needed or driver screams at me
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkShaderModule LoadShader(const char *fileName, VkDevice device)
		{
			std::ifstream file(fileName, std::ios::binary | std::ios::in | std::ios::ate);

			if (file.is_open())
			{
				size_t size = file.tellg();
				file.seekg(0, std::ios::beg);
				char* shaderCode = new char[size];
				file.read(shaderCode, size);
				file.close();

				assert(size > 0);

				VkShaderModule shaderModule;
				VkShaderModuleCreateInfo moduleCreateInfo{};
				moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				moduleCreateInfo.codeSize = size;
				moduleCreateInfo.pCode = (uint32_t*)shaderCode;

                try
                {
				if(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule) != VK_SUCCESS)
                    throw std::runtime_error("Failed to create shder module");
                }
                catch(std::exception& e)
                {
                    std::cout<<e.what()<<std::endl;
                }
				delete[] shaderCode;

				return shaderModule;
			}
			else
			{
				std::cerr << "Error: Could not open shader file \"" << fileName << "\"" << std::endl;
				return VK_NULL_HANDLE;
			}
}

    inline VkPipelineShaderStageCreateInfo LoadShaderFile(VkDevice* logical_device, const std::string& path, VkShaderStageFlagBits stage_bits, VkShaderModule* module)
    {
        //Get code
        auto shader{ReadFile(path)};

        //<f> Create Module
        VkShaderModuleCreateInfo shader_module_create_info{vk::ShaderModuleCreateInfo()};

        shader_module_create_info.codeSize = shader.size();
        //why the hell is it a uint32_t* and not a char*
        shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(shader.data());

        if( vkCreateShaderModule(*logical_device, &shader_module_create_info, nullptr, module) != VK_SUCCESS)
            throw std::runtime_error("Failed to create shader module");
        //</f> /Create Module

        //<f> Create Stage Info
        VkPipelineShaderStageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        //needed or driver screams at me
        info.pNext = nullptr;
        info.flags = 0;

        info.stage = stage_bits;
        info.module = *module;
        info.pName = "main";//name entry point for the shader stage
        //</f> /Create Stage Info

        return info;
    }

    inline VkPipelineShaderStageCreateInfo LoadShaderFile(VkDevice* logical_device, const std::string& path, VkShaderStageFlagBits stage_bits, std::map<std::string, VkShaderModule>& modules)
    {
        VkShaderModule module{VK_NULL_HANDLE};

        if(modules.find(path) != std::end(modules))//we already have loaded the shader
            module = modules[path];
        else
        {
            //Get code
            auto shader{ReadFile(path)};

            //<f> Create Module
            VkShaderModuleCreateInfo shader_module_create_info{vk::ShaderModuleCreateInfo()};

            shader_module_create_info.codeSize = shader.size();
            //why the hell is it a uint32_t* and not a char*
            shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(shader.data());

            if( vkCreateShaderModule(*logical_device, &shader_module_create_info, nullptr, &module) != VK_SUCCESS)
                throw std::runtime_error("Failed to create shader module");

            //store module
            modules[path] = module;
            //</f> /Create Module
        }

        //<f> Create Stage Info
        VkPipelineShaderStageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        //needed or driver screams at me
        info.pNext = nullptr;
        info.flags = 0;

        info.stage = stage_bits;
        info.module = module;
        info.pName = "main";//name entry point for the shader stage
        //</f> /Create Stage Info

        return info;
    }

    inline VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo()
    {
        VkPipelineVertexInputStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo()
    {
        VkPipelineInputAssemblyStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo()
    {
        VkPipelineViewportStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo()
    {
        VkPipelineRasterizationStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo()
    {
        VkPipelineMultisampleStateCreateInfo info{};//not in use, for now
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        info.pNext = nullptr;
        return info;
    }

    inline VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo()
    {
        VkPipelineColorBlendStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo()
    {
        VkPipelineLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo()
    {
        VkPipelineDepthStencilStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo()
    {
        VkGraphicsPipelineCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }
    //</f> /Pipeline

    inline VkRenderPassCreateInfo RenderPassCreateInfo()
    {
        VkRenderPassCreateInfo info{};
        info.sType =VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkFramebufferCreateInfo FramebufferCreateInfo()
    {
        VkFramebufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo()
    {
        VkDescriptorSetLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }

    inline VkWriteDescriptorSet WriteDescriptorSet()
    {
        VkWriteDescriptorSet set{};
        set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        set.pNext = nullptr;
        return set;
    }

    inline bool DepthFormatHasStencil(VkFormat format)
    {
        return format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT;
    }
}

#endif//VULKAN_UTILS
