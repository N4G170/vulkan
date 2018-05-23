#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include <string>

//<f> Load Structs
struct LoadTerrainData
{
    static int count;
    std::string name;
    std::string heightmap_path;
    std::string texture_path;
    std::string normals_path;
    float max_height;
    bool smooth;

    LoadTerrainData(): name{"terrain_"+std::to_string(count)}, heightmap_path{}, texture_path{}, normals_path{}, max_height{50}, smooth{true} { ++count; }
};

// struct LoadSkyboxData
// {
//
// };
//
// struct LoadModelData
// {
//
// };
//</f> /Load Structs

#endif //STRUCTS_HPP
