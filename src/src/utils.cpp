#include "utils.hpp"
#include <fstream>
#include <stdexcept>

std::vector<char> ReadFile(const std::string& filename)
{
    std::ifstream file{filename, std::ios::ate | std::ios::binary | std::ios::in};

    if(!file.is_open())
        throw std::runtime_error("Failed to open file: "+filename);

    std::vector<char> buffer(static_cast<size_t>(file.tellg()));//create buffer with number of chars on file

    //reset read position
    file.seekg(0);
    file.read(buffer.data(), buffer.size());
    file.close();

    return buffer;
}
//
//
// std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);
//
// 			if (is.is_open())
// 			{
// 				size_t size = is.tellg();
// 				is.seekg(0, std::ios::beg);
// 				char* shaderCode = new char[size];
// 				is.read(shaderCode, size);
// 				is.close();
//
// 				assert(size > 0);
//
// 				VkShaderModule shaderModule;
// 				VkShaderModuleCreateInfo moduleCreateInfo{};
// 				moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
// 				moduleCreateInfo.codeSize = size;
// 				moduleCreateInfo.pCode = (uint32_t*)shaderCode;
//
// 				VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));
//
// 				delete[] shaderCode;
//
// 				return shaderModule;
// 			}
// 			else
// 			{
// 				std::cerr << "Error: Could not open shader file \"" << fileName << "\"" << std::endl;
// 				return VK_NULL_HANDLE;
// }
