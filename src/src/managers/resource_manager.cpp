#include "resource_manager.hpp"
#include <utility>
#include "vulkan_utils.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE//because vulkan depth goes from 0 to 1 and gl used -1 to 1
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#include "texture.hpp"
#include "material.hpp"
#include "model.hpp"
#include "mesh.hpp"
#include <iostream>
#include <SDL_image.h>

//<f> Constructors & operator=
ResourceManager::ResourceManager(vk::VulkanPointers vulkan_pointers): m_vulkan_pointers{vulkan_pointers}, m_default_material{}
{
    //create default material
    m_default_material.DiffuseMap(GetTexture("data/textures/dot.png"));
}

ResourceManager::~ResourceManager() noexcept
{

}

ResourceManager::ResourceManager(ResourceManager&& other) noexcept
{

}

ResourceManager& ResourceManager::operator=(ResourceManager&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
Model* ResourceManager::LoadModel(const std::string& path)
{
    //TODO: add later verification if model is already loaded
    auto cached_model = LoadCachedModel(path);

    Model* model = new Model{m_vulkan_pointers};
    model->Name(cached_model->name);

    for(auto cached_mesh : cached_model->meshes)
    {
        Mesh mesh{m_vulkan_pointers};

        mesh.MeshMaterial(cached_mesh.material);
        for(auto cached_vertex : cached_mesh.vertices)
        {
            mesh.AddVertex(cached_vertex);
        }
        for(auto cached_index : cached_mesh.indices)
        {
            mesh.AddIndex(cached_index);
        }

        model->StoreMesh(std::move(mesh));
    }

    std::lock_guard<std::mutex> lock{m_model_vector_mutex};
    m_models.push_back(model);
    return m_models.back();
}

Model* ResourceManager::LoadQuadrilateral(const glm::vec3& top_left, const glm::vec3& top_right, const glm::vec3& bottom_left, const glm::vec3& bottom_right, Material* material)
{
    Mesh mesh{m_vulkan_pointers};
    if(material == nullptr)
        material = &m_default_material;

    mesh.MeshMaterial(material);

    vk::Vertex vertex_tl{};
    vk::Vertex vertex_tr{};
    vk::Vertex vertex_bl{};
    vk::Vertex vertex_br{};

    //convert vertex data to local vertex
    vertex_tl.position = top_left;
    vertex_tr.position = top_right;
    vertex_bl.position = bottom_left;
    vertex_br.position = bottom_right;

    //normals TODO: do this the right way
    vertex_tl.normal = glm::cross( top_right - top_left, bottom_left - top_left );
    vertex_tr.normal = glm::cross( top_left - top_right, bottom_right - top_right );
    vertex_bl.normal = glm::cross( top_left - bottom_left, bottom_right - bottom_left );
    vertex_br.normal = glm::cross( bottom_left - bottom_right, top_right - bottom_right );

    // TODO: set uv
    vertex_tl.uv = glm::vec2(0,0);
    vertex_tr.uv = glm::vec2(1,0);
    vertex_bl.uv = glm::vec2(0,1);
    vertex_br.uv = glm::vec2(1,1);

    //diffuse colour
    vertex_tl.diffuse_colour = material->DiffuseColour();
    vertex_tr.diffuse_colour = material->DiffuseColour();
    vertex_bl.diffuse_colour = material->DiffuseColour();
    vertex_br.diffuse_colour = material->DiffuseColour();

    mesh.AddVertex(vertex_tl);//0
    mesh.AddVertex(vertex_tr);//1
    mesh.AddVertex(vertex_bl);//2
    mesh.AddVertex(vertex_br);//3
    //clockwise
    //face 1
    mesh.AddIndex(0);
    mesh.AddIndex(1);
    mesh.AddIndex(3);
    //face 2
    mesh.AddIndex(0);
    mesh.AddIndex(3);
    mesh.AddIndex(2);

    Model* model = new Model{m_vulkan_pointers};
    model->StoreMesh(std::move(mesh));

    std::lock_guard<std::mutex> lock{m_model_vector_mutex};
    m_models.push_back(model);
    return m_models.back();
}

void ResourceManager::InitVulkanObjects()
{
    //TODO:recheck the use of the m_vulkan_objects_created flag to avoid double creation
    for(auto& model : m_models)
    {
        model->CreateVulkanObjects();
    }
    for(auto& texture : m_textures)
        texture.second.CreateVulkanObjects();
}

void ResourceManager::Cleanup()
{
    //TODO check if right
    for(auto& model : m_models)
    {
        model->Cleanup();
        delete(model);
    }
    m_models.clear();

    for(auto& surface : m_loaded_surfaces)
    {
        SDL_FreeSurface(surface.second);
    }
    m_loaded_surfaces.clear();

    for(auto& texture : m_textures)
    {
        texture.second.Cleanup();
    }
    m_textures.clear();
    m_materials.clear();
}

void ResourceManager::PreLoadSurface(const std::string& path)
{
    if(m_loaded_surfaces.find(path) != std::end(m_loaded_surfaces))//already loaded
        return;

    SDL_Surface* surface{ IMG_Load(path.data()) };

    if(surface != nullptr)
    {
        //check if surface has 4 channels
        if(surface->format->BytesPerPixel < 4)
            surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);

        m_loaded_surfaces[path] = surface;
    }
}
SDL_Surface* ResourceManager::PreLoadAndGetSurface(const std::string& path)
{
    PreLoadSurface(path);

    return m_loaded_surfaces[path];
}

/**
 * \brief Load a texture from file or reuses a surface pointer (if already loaded) to create a Texture instance
 * \n might call PreLoadSurface inside
 */
void ResourceManager::PreLoadTexture(const std::string& path)
{
    if(m_textures.find(path) != std::end(m_textures))//already loaded
        return;

    //create new
    Texture texture{ m_vulkan_pointers };
    texture.LoadTexture(PreLoadAndGetSurface(path), path);
    m_textures.insert(std::make_pair(path, std::move(texture)));
}
Texture* ResourceManager::PreLoadAndGetTexture(const std::string& path)
{
    PreLoadTexture(path);

    return &m_textures.at(path);
}

Texture* ResourceManager::PreLoadAndGetCubemap(const std::string& front_path, const std::string& back_path, const std::string& up_path,
                              const std::string& down_path, const std::string& right_path, const std::string& left_path)
{
    //get surfaces
    SDL_Surface* front = PreLoadAndGetSurface(front_path);
    SDL_Surface* back = PreLoadAndGetSurface(back_path);
    SDL_Surface* up = PreLoadAndGetSurface(up_path);
    SDL_Surface* down = PreLoadAndGetSurface(down_path);
    SDL_Surface* right = PreLoadAndGetSurface(right_path);
    SDL_Surface* left = PreLoadAndGetSurface(left_path);

    Texture texture{ m_vulkan_pointers };
    texture.LoadCubemap(front, back, up, down, right, left, "skybox");

    m_textures.insert(std::make_pair("skybox", std::move(texture)));

    return &m_textures.at("skybox");
}

Model* ResourceManager::LoadTerrain(const LoadTerrainData& data)
{
    auto surface = PreLoadAndGetSurface(data.heightmap_path);
    //create vector with pixel data

    uint32_t* pixels = static_cast<uint32_t*>(surface->pixels);//each pixel has 4 uint8_t elements
    int map_width = surface->w; int map_height = surface->h;

    std::vector<float> heightmap;
    for(auto i{0}; i < map_width * map_height; ++i)//store values to smooth them
    {
        uint8_t height = ((uint8_t)(pixels[i] >> 8));
        heightmap.push_back(static_cast<float>(height));
    }
    //<f> Smooth
    if(data.smooth)
    {
        for(auto i{0}; i<map_height; ++i)//rows
        {
            for(auto j{0}; j<map_width; ++j)//columns
            {
                float avg = 0;
                int count = 0;

                auto index = i * map_width + j;//this pixel
                for(auto m{i -1}; m < i + 2; ++m)//rows
                {
                    for(auto n{j -1}; n < j + 2; ++n)//columns
                    {
                        if(m >= 0 && m < map_height && n >= 0 && n < map_width)
                        {
                            avg += heightmap[ m * map_width + n ];
                            ++count;
                        }
                    }
                }
                // std::cout<<avg<<std::endl;
                if(count > 0)
                    heightmap[index] = avg / count;
            }
        }
    }
    //</f> /Smooth

    //<f> Load Normals?
    std::vector<glm::vec3> normals;
    if(!data.normals_path.empty())
    {
        SDL_Surface* normal_map = PreLoadAndGetSurface(data.normals_path);
        uint32_t* normal_pixels = static_cast<uint32_t*>(normal_map->pixels);//each pixel has 4 uint8_t elements
        for(auto i{0}; i < map_width * map_height; ++i)//store values to smooth them
        {
            uint8_t x = ((uint8_t)(normal_pixels[i]));
            uint8_t y = ((uint8_t)(normal_pixels[i] >> 8));
            uint8_t z = ((uint8_t)(normal_pixels[i] >> 16));
            float normal_x = (x - 128) / 128.f;
            float normal_y = (y - 128) / 128.f;
            float normal_z = (z - 128) / 128.f;

            normals.push_back( glm::vec3{ normal_x, normal_y, -normal_z } );
            // std::cout<<normal_x<<" - "<<normal_y<<" - "<<normal_z<<std::endl;
        }
    }
    //</f> /Load Normals?


    Mesh mesh{m_vulkan_pointers};
    mesh.Name(data.name);
    Material* material = nullptr;

    //load material
    static int count = 0;
    if(data.texture_path.empty())//use default
    {
        //load map as texture
        material = CreateMaterial(data.heightmap_path + std::to_string(count));
        material->DiffuseMap(PreLoadAndGetTexture(data.heightmap_path));
    }
    else
    {
        //load texture
        material = CreateMaterial(data.texture_path + std::to_string(count));
        material->DiffuseMap(PreLoadAndGetTexture(data.texture_path));
    }
    ++count;
    mesh.MeshMaterial(material);

    //<f> Create Vertices
    for(auto i{0}; i < map_width * map_height; ++i)
    {
        int x = i % map_width;
        int z = i / map_width;//y of the image bu z in the world
        int y = data.max_height * heightmap[i]/255;//green value
        vk::Vertex vertex;
        //convert vertex data to local vertex
        vertex.position = {-x, y, -z};

        //normals TODO:check if right
        if(!normals.empty())
        {
            vertex.normal = normals[i];
        }
        else
        {
            vertex.normal = {0,-1,0};//up
        }
        //relative positions
        vertex.uv = glm::vec2(x/(float)map_width, z/(float)map_height);

        //diffuse colour
        vertex.diffuse_colour = glm::vec4{1.f};

        mesh.AddVertex(vertex);
    }
    //</f> /Create Vertices

    //<f> Create Mesh
    for(auto i{0}; i<map_height - 1; ++i)//rows, we do not need to run on last one
    {
        for(auto j{0}; j<map_width - 1; ++j)//columns, we do not ned to process the last one
        {
            auto current = i * map_width + j;
            auto right = i * map_width + (j+1);
            auto bottom = (i+1) * map_width + j;
            auto bottom_right = (i+1) * map_width + (j+1);

            //bottom triangle
            mesh.AddIndex(current);
            mesh.AddIndex(bottom_right);
            mesh.AddIndex(bottom);
            //right triangle
            mesh.AddIndex(current);
            mesh.AddIndex(right);
            mesh.AddIndex(bottom_right);
        }
    }
    //</f> /Create Mesh

    Model* model = new Model{m_vulkan_pointers};
    model->Name(data.name);
    model->StoreMesh(std::move(mesh));

    std::lock_guard<std::mutex> lock{m_model_vector_mutex};
    m_models.push_back(model);
    return m_models.back();
}

Material* ResourceManager::CreateMaterial(const std::string& name)
{
    if(m_materials.find(name) != std::end(m_materials))
        return nullptr;

    m_materials[name] = m_default_material;

    return &m_materials[name];
}
//</f> /Methods

//<f> Getters/Setters
Texture* ResourceManager::GetTexture(const std::string& path)
{
    if(m_textures.find(path) != std::end(m_textures))
        return &m_textures.at(path);

    //load new
    Texture texture{ m_vulkan_pointers };
    texture.LoadTexture(PreLoadAndGetSurface(path), path);
    m_textures.insert(std::make_pair(path, std::move(texture)));

    return &m_textures.at(path);
}
//</f> /Getters/Setters

//<f> Private Functions
Material* ResourceManager::LoadMaterial(aiMaterial* material_data)
{
    aiString ai_name;
    if(material_data->Get(AI_MATKEY_NAME, ai_name) != AI_SUCCESS)
        return &m_default_material;

    //got name
    std::string name{ai_name.C_Str()};
    if(m_materials.find(name) != std::end(m_materials))//already loaded
        return &m_materials[name];

    //create new
    Material material;
    //diffuse_map
    if(material_data->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString path{};
        material_data->GetTexture(aiTextureType_DIFFUSE, 0, &path);
        //TODO: correct this change
        std::string str= "data/models/castle/";
        str.append(path.C_Str());
        material.DiffuseMap(GetTexture(str));
        // material.DiffuseMap(GetTexture(path.C_Str()));
    }
    else//use default
    {
        material.DiffuseMap(GetTexture("data/textures/dot.png"));
    }
    //colours
    aiColor4D colour;
    float weight{0.f};
    if(material_data->Get(AI_MATKEY_COLOR_DIFFUSE, colour) == AI_SUCCESS)
        material.DiffuseColour(glm::vec4(colour.r, colour.g, colour.b, 1.f));
    if(material_data->Get(AI_MATKEY_COLOR_AMBIENT, colour) == AI_SUCCESS)
        material.AmbientColour(glm::vec4(colour.r, colour.g, colour.b, 1.f));
    if(material_data->Get(AI_MATKEY_COLOR_SPECULAR, colour) == AI_SUCCESS)
        material.SpecularColour(glm::vec4(colour.r, colour.g, colour.b, 1.f));
    if(material_data->Get(AI_MATKEY_SHININESS_STRENGTH, weight) == AI_SUCCESS)
        material.SpecularWeight(glm::float_t(weight));

    m_materials[name] = std::move(material);

    return &m_materials[name];
}

ModelDataCache* ResourceManager::LoadCachedModel(const std::string& path)
{
    if(m_cached_models.find(path) != std::end(m_cached_models))
        return &m_cached_models[path];

    //cache new model
    Assimp::Importer model_importer;
	// Flags for loading the mesh
	static const int assimpFlags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices;

    const aiScene* scene{model_importer.ReadFile(path.c_str(), assimpFlags)};
    // const aiScene* scene{m_assimp_importer.ReadFile(path.c_str(), assimpFlags)};

    ModelDataCache model{};
    model.name = path;

    //create vertex buffer
    auto scale{1.f};

    for(uint32_t m{0}; m < scene->mNumMeshes; ++m)//check every mesh in the scene/model
    {
        MeshDataCache mesh{};
        Material* material{nullptr};

        auto ai_mesh{scene->mMeshes[m]};

        //<f> Process material
        if(scene->HasMaterials())
        {
            auto material_data{scene->mMaterials[ai_mesh->mMaterialIndex]};
            //check if it has diffuse texture
            material = LoadMaterial(material_data);
        }
        else
            material = &m_default_material;

        mesh.material = material;
        //</f> /Process material

        //<f> Vertices
        //check every vertex in the current mesh
        for(uint32_t v{0}; v < ai_mesh->mNumVertices; ++v)
        {
            auto vectex_data{ai_mesh->mVertices[v]};
            auto vertex_normal{ai_mesh->mNormals[v]};
            vk::Vertex vertex{};

            //convert vertex data to local vertex
            //glm make_* functions can convert ASSIMP vectors to glm vectors
			vertex.position = glm::make_vec3(&vectex_data.x) * scale;
			// vertex.normal = glm::make_vec3(&vectex_data.x);
			vertex.normal = glm::make_vec3(&vertex_normal.x);
			// Texture coordinates and colors may have multiple channels, we only use the first [0] one
            if(scene->mMeshes[m]->HasTextureCoords(0))
			    vertex.uv = glm::make_vec2(&scene->mMeshes[m]->mTextureCoords[0][v].x);
			// Model may not have vertex colors
			// vertex.diffuse_colour = (mesh->HasVertexColors(0)) ? glm::make_vec3(&mesh->mColors[0][v].r) : glm::vec3(1.0f);

            //connect colours to vertices
            vertex.diffuse_colour = glm::vec3(material->DiffuseColour());
            vertex.ambient_colour = glm::vec3(material->AmbientColour());
            vertex.specular_colour = glm::vec3(material->SpecularColour());

            vertex.specular_weight = material->SpecularWeight();

            vertex.uv.y *= -1;
			// Vulkan uses a right-handed NDC (contrary to OpenGL), so simply flip Y-Axis
			// vertex.position.y *= -1.0f;
			// vertex.normal.y *= -1.0f;
			// vertex.normal.z *= -1.0f;
            mesh.vertices.push_back(vertex);
        }
        //</f> /Vertices

        //<f> Indices
        for (uint32_t f{0}; f < ai_mesh->mNumFaces; ++f)//check every face of current mesh
        {
            // We assume that all faces are triangulated
			for (uint32_t i{0}; i < 3; ++i)
            {
                mesh.indices.push_back(static_cast<uint32_t>(ai_mesh->mFaces[f].mIndices[i]));
            }
        }
        //</f> /Indices

        //store mesh
        model.meshes.push_back(mesh);
    }

    std::lock_guard<std::mutex> lock{m_model_cache_mutex};
    m_cached_models[path] = model;

    return &m_cached_models[path];
}
//</f> /Private Functions
