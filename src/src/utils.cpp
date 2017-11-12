#include "utils.hpp"
#include <fstream>
#include <stdexcept>

std::vector<char> ReadFile(const std::string& filename)
{
    std::ifstream file{filename, std::ios::ate | std::ios::binary};

    if(!file.is_open())
        throw std::runtime_error("Failed to open file: "+filename);

    std::vector<char> buffer(static_cast<size_t>(file.tellg()));//create buffer with number of chars on file

    //reset read position
    file.seekg(0);
    file.read(buffer.data(), buffer.size());

    return buffer;
}
