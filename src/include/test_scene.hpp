#ifndef TEST_SCENE_HPP
#define TEST_SCENE_HPP

#include <string>
#include <glm/glm.hpp>
#include "vulkan_structs.hpp"
#include "resource_manager.hpp"
#include <utility>
#include <future>
#include "sol.hpp"
#include "vulkan_utils.hpp"

vk::PipelineType StringToShaderType(const std::string& str)
{
    if(str == "diffuse")
        return vk::PipelineType::DIFFUSE;
    if(str == "wireframe")
        return vk::PipelineType::WIREFRAME;
    if(str == "toon")
        return vk::PipelineType::TOON;
    if(str == "points")
        return vk::PipelineType::POINTS;
    if(str == "skybox")
        return vk::PipelineType::SKYBOX;
    //default
    return vk::PipelineType::DIFFUSE;
}

struct ModelsPointers
{
    Model* test_model;

    std::vector<Model*> flags;
    std::vector<Model*> patrol;

    Model* light_model;

    ModelsPointers() : test_model{nullptr}, light_model{nullptr} {}

    bool ValidTest() { return test_model != nullptr; }

    bool ValidFlags() { return flags.size() > 0; }

    bool ValidPatrol() { return patrol.size() > 0; }

    bool ValidLight() { return light_model != nullptr; }
};

struct ModelLoadData
{
    std::string path;
    vk::PipelineType pipeline_type;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    std::string model_type;

    ModelLoadData() : path{""}, pipeline_type{vk::PipelineType::DIFFUSE}, position{0}, rotation{0}, scale{1}, model_type{""}{}
    ModelLoadData(const std::string& file_path, vk::PipelineType pipeline, glm::vec3 start_position, glm::vec3 start_rotation, glm::vec3 start_scale, const std::string& defined_type) :
    path{file_path}, pipeline_type{pipeline}, position{start_position}, rotation{start_rotation}, scale{start_scale}, model_type{defined_type}{}

    ModelLoadData(const ModelLoadData& other){ *this = other; }

    ModelLoadData& operator= (const ModelLoadData& other)
    {
        path = other.path;
        pipeline_type = other.pipeline_type;
        position = other.position;
        rotation = other.rotation;
        scale = other.scale;
        model_type = other.model_type;
        return *this;
    }
};

Model* LoadModel(ResourceManager* resource_manager, const ModelLoadData& data)
{
    Model* model = resource_manager->LoadModel(data.path);

    model->PipelineType(data.pipeline_type);
    model->InitTransform(data.position, data.rotation, data.scale);

    return model;
};

std::vector<Model*> LoadLoadingScreen(ResourceManager* resource_manager)
{
    std::vector<Model*> models;
    //<f> UI
    // pointers.test_model = resource_manager->LoadQuadrilateral( {1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0}, mat );
    auto homogenous_value = [](int value, int side)->float { float half_side = (float)side/2; return (value - half_side) / half_side; };

    struct rect
    {
        int x;
        int y;
        int w;
        int h;
    } test_rect;

    //<f> Controls
    auto mat = resource_manager->CreateMaterial("load_0");
    mat->DiffuseMap( resource_manager->PreLoadAndGetTexture("data/textures/ui/load.png") );

    auto  surface = resource_manager->PreLoadAndGetSurface("data/textures/ui/load.png");
    int window_w = 1760;
    int window_h = 990;

    test_rect.x = window_w/2 - surface->w/2;
    test_rect.y = window_h/2 - surface->h/2;
    test_rect.w = surface->w;
    test_rect.h = surface->h;

    glm::vec3 tl{ homogenous_value(test_rect.x, window_w), homogenous_value(test_rect.y, window_h), 0 };
    glm::vec3 tr{ homogenous_value(test_rect.x + test_rect.w, window_w), homogenous_value(test_rect.y, window_h), 0 };
    glm::vec3 bl{ homogenous_value(test_rect.x, window_w), homogenous_value(test_rect.y + test_rect.h, window_h), 0 };
    glm::vec3 br{ homogenous_value(test_rect.x + test_rect.w, window_w), homogenous_value(test_rect.y + test_rect.h, window_h), 0 };

    auto model = resource_manager->LoadQuadrilateral( tl, tr, bl, br, mat );
    // pointers.test_model = resource_manager->LoadQuadrilateral( {0,0,0}, {1,0,0}, {0,1,0}, {1,1,0}, mat );
    model->PipelineType(vk::PipelineType::UI);
    model->InitTransform({0,0,0}, {0,0,0}, {1,1,1});
    models.push_back(model);
    //</f> /Controls

    //</f> /UI

    return models;
}

std::vector<Model*> LoadScene(ResourceManager* resource_manager, ModelsPointers& pointers)
{
    std::vector<Model*> models;
    std::vector<ModelLoadData> load_data;

    //<f> UI
    // pointers.test_model = resource_manager->LoadQuadrilateral( {1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0}, mat );
    auto homogenous_value = [](int value, int side)->float { float half_side = (float)side/2; return (value - half_side) / half_side; };

    struct rect
    {
        int x;
        int y;
        int w;
        int h;
    } test_rect;

    int window_w = 1760;
    int window_h = 990;

    //<f> Controls
    auto mat = resource_manager->CreateMaterial("ui_0");
    mat->DiffuseMap( resource_manager->PreLoadAndGetTexture("data/textures/ui/controls.png") );

    auto  surface = resource_manager->PreLoadAndGetSurface("data/textures/ui/controls.png");
    test_rect.x = test_rect.y = 20;
    test_rect.w = surface->w; test_rect.h = surface->h;

    glm::vec3 tl{ homogenous_value(test_rect.x, window_w), homogenous_value(test_rect.y, window_h), 0 };
    glm::vec3 tr{ homogenous_value(test_rect.x + test_rect.w, window_w), homogenous_value(test_rect.y, window_h), 0 };
    glm::vec3 bl{ homogenous_value(test_rect.x, window_w), homogenous_value(test_rect.y + test_rect.h, window_h), 0 };
    glm::vec3 br{ homogenous_value(test_rect.x + test_rect.w, window_w), homogenous_value(test_rect.y + test_rect.h, window_h), 0 };

    auto rect_controls = resource_manager->LoadQuadrilateral( tl, tr, bl, br, mat );
    // pointers.test_model = resource_manager->LoadQuadrilateral( {0,0,0}, {1,0,0}, {0,1,0}, {1,1,0}, mat );
    rect_controls->PipelineType(vk::PipelineType::UI);
    rect_controls->InitTransform({0,0,0}, {0,0,0}, {1,1,1});
    models.push_back(rect_controls);
    //</f> /Controls
    //<f> block
    test_rect.x = 435;
    test_rect.y = 20;
    test_rect.w = 50; test_rect.h = 50;

    tl = { homogenous_value(test_rect.x, window_w), homogenous_value(test_rect.y, window_h), 0 };
    tr = { homogenous_value(test_rect.x + test_rect.w, window_w), homogenous_value(test_rect.y, window_h), 0 };
    bl = { homogenous_value(test_rect.x, window_w), homogenous_value(test_rect.y + test_rect.h, window_h), 0 };
    br = { homogenous_value(test_rect.x + test_rect.w, window_w), homogenous_value(test_rect.y + test_rect.h, window_h), 0 };

    pointers.test_model = resource_manager->LoadQuadrilateral( tl, tr, bl, br );
    pointers.test_model->PipelineType(vk::PipelineType::UI);
    pointers.test_model->InitTransform({0,0,0}, {0,0,0}, {1,1,1});
    models.push_back(pointers.test_model);
    //</f> /block
    //<f> test UI
    for(auto i{0}; i<10; ++i)
    {
        for(auto j{0}; j<3; ++j)
        {
            std::string mat_name = "ui_test_"+std::to_string((i*3+j));
            mat = resource_manager->CreateMaterial(mat_name);
            test_rect.x = window_w - 325 + 25*i + 5*i;//base offset + w of previous pieces + spacing
            test_rect.y = 20 + 25*j + 5*j;//base offset + h of previous pieces + spacing
            test_rect.w = 25; test_rect.h = 25;

            if(j == 0) mat->DiffuseColour( {.1+1*i/10., 0, 0, 1} );
            else if(j == 1)  mat->DiffuseColour( {0, .1+1*i/10., 0, 1} );
            else if(j == 2)  mat->DiffuseColour( {0, 0, .1+1*i/10., 1} );

            tl = { homogenous_value(test_rect.x, 1760), homogenous_value(test_rect.y, 990), 0 };
            tr = { homogenous_value(test_rect.x + test_rect.w, 1760), homogenous_value(test_rect.y, 990), 0 };
            bl = { homogenous_value(test_rect.x, 1760), homogenous_value(test_rect.y + test_rect.h, 990), 0 };
            br = { homogenous_value(test_rect.x + test_rect.w, 1760), homogenous_value(test_rect.y + test_rect.h, 990), 0 };

            Model* m = resource_manager->LoadQuadrilateral( tl, tr, bl, br, mat );
            m->PipelineType(vk::PipelineType::UI);
            models.push_back(m);
        }
    }
    //</f> /test UI
    //</f> /UI

    //<f> Skybox
    ModelLoadData skybox_data{};
    skybox_data.path = "data/models/shapes/cube.obj";
    skybox_data.position = {0, 0, 0};
    skybox_data.rotation = {0, -90, 0};
    skybox_data.scale = {5000, 5000, 5000};
    skybox_data.pipeline_type = vk::PipelineType::SKYBOX;
    //<f> cubemap
    auto skybox_mat = resource_manager->CreateMaterial("skybox");
    std::string sky_path = "data/textures/skybox/";
    skybox_mat->DiffuseMap( resource_manager->PreLoadAndGetCubemap(sky_path+"front.png", sky_path+"back.png", sky_path+"up.png", sky_path+"down.png", sky_path+"right.png", sky_path+"left.png") );
    //</f> /cubemap
    auto skybox_model = LoadModel(resource_manager, skybox_data);
    skybox_model->MeshAt(0)->MeshMaterial(skybox_mat);
    models.push_back(skybox_model);
    //</f> /Skybox

    //<f> Terrain
    LoadTerrainData terrain_data{};

    //<f> Main Terrain
    terrain_data.heightmap_path = "data/textures/terrain/map.png";
    terrain_data.texture_path = "data/textures/terrain/map_texture.png";
    terrain_data.normals_path = "data/textures/terrain/map_normals.png";
    terrain_data.max_height = 100;
    terrain_data.smooth = true;
    auto terrain_0 = resource_manager->LoadTerrain(terrain_data);
    terrain_0->PipelineType(vk::PipelineType::DIFFUSE);
    terrain_0->InitTransform({1030, 0, 610}, {0, 0, 0}, {2.f, 2.f, 2.f});
    pointers.test_model = terrain_0;
    models.push_back( terrain_0 );
    //</f> /Main Terrain

    //<f> Big Terrain
    terrain_data.heightmap_path = "data/textures/terrain/land.jpg";
    terrain_data.texture_path = "";
    terrain_data.normals_path = "";
    terrain_data.max_height = 50;
    terrain_data.smooth = true;
    auto terrain_1 = resource_manager->LoadTerrain(terrain_data);
    terrain_1->PipelineType(vk::PipelineType::DIFFUSE);
    // terrain_1->InitTransform({2500, -1000, 960}, {0, 0, 0}, {3.34f, 3.34f, 3.34f});
    terrain_1->InitTransform({750, -1000, 287}, {0, 0, 0}, {1.f, 1.f, 1.f});
    models.push_back( terrain_1 );
    //</f> /Big Terrain
    //</f> /Terrain

    //<f> lua
    sol::state lua_state;

    lua_state.script_file("data/scripts/scenes.lua");

    sol::table filenames = lua_state["scripts"];

    // for(auto f{1}; f <= 1; ++f)
    for(auto f{1}; f <= filenames.size(); ++f)
    {
        lua_state.script_file(filenames[f]);
        sol::table scene = lua_state["models"];
        glm::vec3 parent_position{0};

        parent_position.x = lua_state["parent_position"][1], parent_position.y = lua_state["parent_position"][2], parent_position.z = lua_state["parent_position"][3];

        // for(auto i{1}; i <= 1; ++i)
        for(auto i{1}; i <= scene.size(); ++i)
        {
            sol::table obj_table = scene[i];
            ModelLoadData data{};
            data.path = obj_table["path"];
            data.position.x = obj_table["position"][1], data.position.y = obj_table["position"][2], data.position.z = obj_table["position"][3];
            data.position.x += parent_position.x, data.position.y += parent_position.y, data.position.z += parent_position.z;//add parent pos
            data.rotation.x = obj_table["rotation"][1], data.rotation.y = obj_table["rotation"][2], data.rotation.z = obj_table["rotation"][3];
            data.scale.x = obj_table["scale"][1], data.scale.y = obj_table["scale"][2], data.scale.z = obj_table["scale"][3];
            data.pipeline_type = StringToShaderType(scene[i]["shader"]);
            data.model_type = obj_table["model_type"];
            load_data.push_back(data);
        }

        if(lua_state["has_generate_models"] == true)
        {
            sol::table backup_army = lua_state["GeneratedModels"]();
            for(auto i{1}; i <= backup_army.size(); ++i)
            {
                sol::table obj_table = backup_army[i];
                ModelLoadData data{};
                data.path = obj_table["path"];
                data.position.x = obj_table["position"][1], data.position.y = obj_table["position"][2], data.position.z = obj_table["position"][3];
                data.position.x += parent_position.x, data.position.y += parent_position.y, data.position.z += parent_position.z;//add parent pos
                data.rotation.x = obj_table["rotation"][1], data.rotation.y = obj_table["rotation"][2], data.rotation.z = obj_table["rotation"][3];
                data.scale.x = obj_table["scale"][1], data.scale.y = obj_table["scale"][2], data.scale.z = obj_table["scale"][3];
                data.pipeline_type = StringToShaderType(backup_army[i]["shader"]);
                load_data.push_back(data);
            }
        }
    }
    //</f> /lua

    for(auto& data : load_data)
    {
        Model* model{LoadModel(resource_manager, data)};
        if(data.model_type == "")
        {

        }
        else if(data.model_type == "flag")
        {
            pointers.flags.push_back(model);
        }
        else if(data.model_type == "patrol")
        {
            pointers.patrol.push_back(model);

        }
        else if(data.model_type == "test")
        {
            pointers.test_model = model;
        }
        models.push_back( model );
    }
    return models;
};

void ProcessFlags(ModelsPointers pointers, float last_frame_time)
{
    if(!pointers.ValidFlags())
        return;

    static float flag_rotation_step{10.f};

    auto flag_rot{pointers.flags[0]->Rotation()};
    flag_rot.y += flag_rotation_step * last_frame_time;
    if(flag_rot.y >= 20) flag_rotation_step = -10.f;
    if(flag_rot.y <= -20) flag_rotation_step = 10.f;
    for(auto& flag : pointers.flags)
        flag->Rotation(flag_rot);
}

void ProcessPatrols(ModelsPointers pointers, float last_frame_time)
{
    if(!pointers.ValidPatrol())
        return;

    //parollman 00 goes from z=0 to <= -100
    static float step{-10.f};
    bool flip{false};
    static bool rotate{false};

    //rotate before move
    if(rotate)
    {
        rotate = false;
        auto rot{pointers.patrol[0]->Rotation()};
        rot.y = rot.y + 180;

        for(auto& patrolman : pointers.patrol)
            patrolman->Rotation(rot);
    }

    //<f> Move
    auto pos{pointers.patrol[0]->Position()};
    pos.z += step * last_frame_time;
    // pointers.patrol[0]->Position(pos);

    if(pos.z < -100 || pos.z > 0) flip = true;

    for(auto& patrolman : pointers.patrol)
    {
        pos = patrolman->Position();
        pos.z += step * last_frame_time;
        patrolman->Position(pos);
    }
    //</f> /Move

    if(flip){ step *= -1; rotate = true; }

}
#endif //TEST_SCENE_HPP
