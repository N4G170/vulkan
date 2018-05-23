#include <iostream>
#include "SDL.h"
#include "sdl_init.hpp"
#include <chrono>
#include "vulkan_context.hpp"
#include "model.hpp"
#include "camera.hpp"
#include <algorithm>
#include "resource_manager.hpp"

#include "swapchain.hpp"
#include "renderpass.hpp"
#include "framebuffer.hpp"
#include "pipelines.hpp"
#include "commandbuffer.hpp"
#include "vulkan_resources.hpp"
#include "vulkan_pointers.hpp"

#include "test_scene.hpp"

void Resize(vk::VulkanResources* vulkan_resources, vk::VulkanContext* context, vk::Swapchain* swapchain, vk::Renderpass* renderpass, vk::Pipelines* pipelines,
            vk::Framebuffer* framebuffer, vk::CommandBuffer* commandbuffer)
{
    return;
    // context->WaitForIdle();
    //
    // commandbuffer->Cleanup();
    // swapchain->Cleanup();
    // renderpass->Cleanup();
    // framebuffer->Cleanup();
    //
    // commandbuffer->Init();
    // swapchain->Init();
    // renderpass->Init(swapchain);
    // framebuffer->Init(swapchain, renderpass);
    // pipelines->Init(vulkan_resources, swapchain, renderpass);
    // commandbuffer->InitMainCommanfBuffers(framebuffer);
}

void OnWindowEvent(const SDL_Event* event, bool* resize)
{
    if (event->type == SDL_WINDOWEVENT)
    {
        switch (event->window.event)
        {
            case SDL_WINDOWEVENT_RESIZED:
                *resize = true;
            break;

            default:
            // SDL_Log("Window %d got unknown event %d",
            //         event->window.windowID, event->window.event);
            break;
        }
    }
}
void P(glm::vec3& p, float x, float y, float z)
{
    p.x = x;
    p.y = y;
    p.z = z;
}
glm::vec3 P(float x, float y, float z)
{
    return std::move(glm::vec3{x,y,z});
}

int main(int argc, char* argv[])
{
    SDL_Window* window{nullptr};

    //init SDL subsystems
    SDLInitConfig sdl_config{};//load default values
    bool init_result = InitSDL(window, sdl_config);

    if(!init_result)//failed to initialize sdl subsystems
    {
        //terminate any initialized sdl subsystems
        TerminateSDL();
        DeleteSDLPointers(window);//they are deleted in a sepecific way
        window = nullptr;

        return -2;
    }
    else//sdl started ok
    {
        //<f> Vulkan Init

        //init vulkan
        vk::VulkanPointers vulkan_pointers;
        vk::VulkanContext vulkan_context{window};
        vulkan_context.Init();

        vk::VulkanResources vulkan_resources{&vulkan_context};
        vulkan_resources.MaxDescriptorCount(3000);
        vulkan_resources.InitDescriptorLayoutAndPools();

        //other vulkan objects
        vk::CommandBuffer commandbuffer{&vulkan_context};
        commandbuffer.Init();
        //
        vk::Swapchain swapchain{&vulkan_context, &commandbuffer};
        vk::Renderpass renderpass{&vulkan_context};
        vk::Framebuffer framebuffer{&vulkan_context};
        vk::Pipelines pipelines{&vulkan_context};

        swapchain.Init();
        renderpass.Init(&swapchain);
        framebuffer.Init(&swapchain, &renderpass);
        commandbuffer.InitMainCommanfBuffers(&framebuffer);

        //set variables to be used by resource manager
        vulkan_pointers.context = &vulkan_context;
        vulkan_pointers.logical_device = vulkan_context.LogicalDevice();
        vulkan_pointers.resources = &vulkan_resources;
        vulkan_pointers.memory_manager = vulkan_context.MemoryManager();
        vulkan_pointers.commandbuffer = &commandbuffer;
        //
        pipelines.Init(&vulkan_resources, &swapchain, &renderpass);

        ResourceManager resource_manager{vulkan_pointers};
        //</f> /Vulkan Init

        //<f> Camera
        Camera camera{};

        camera.Position({-4.51862f, -93.4624f, 299.656f});
        camera.Rotation({16.f, 180.f, 0.f});
        camera.Up({0.f, 1.f, 0.f});

        camera.AspectRatio(swapchain.AspectRatio());
        camera.FOVDegrees(45.f);
        camera.ZNear(0.01f);
        camera.ZFar(15000.f);

        camera.Update();
        //</f> /Camera

        //<f> LoadScreen
        auto load_screen = LoadLoadingScreen(&resource_manager);
        resource_manager.InitVulkanObjects();
        std::vector<vk::VulkanModelData> load_screen_data;
        for(auto i{0}; i<load_screen.size(); ++i)
            load_screen_data.push_back(load_screen[i]->CreateVulkanModelData());
        commandbuffer.RegisterModelsData(&swapchain, &renderpass, &framebuffer, &pipelines, load_screen_data);
        bool tmp_bool{false};
        commandbuffer.DrawFrame(&swapchain, &tmp_bool);
        //</f> /LoadScreen

        //<f> Models
        ModelsPointers models_pointers{};
        // auto start_time = std::chrono::high_resolution_clock::now();
        auto models = LoadScene(&resource_manager, models_pointers);
        // auto end_time = std::chrono::high_resolution_clock::now();
        // std::cout<<"Duration: "<<std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()<<std::endl;
        // //init vulkan objects like buffers and descriptors
        resource_manager.InitVulkanObjects();

        // //build models data
        std::vector<vk::VulkanModelData> models_data;
        for(auto i{0}; i<models.size(); ++i)
        models_data.push_back(models[i]->CreateVulkanModelData());

        // start_time = std::chrono::high_resolution_clock::now();
        commandbuffer.RegisterModelsData(&swapchain, &renderpass, &framebuffer, &pipelines, models_data);
        // end_time = std::chrono::high_resolution_clock::now();
        // std::cout<<"Duration Bind: "<<std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()<<std::endl;
        //</f> /Models

        vk::LightData light{};

        light.light_position = glm::vec4{-70.f, -2223.f, -5000.f, 1.f};//same pos as light in skybox (but z must be negative, TODO: check why)
        light.light_colour = glm::vec4{1.f, 1.f, 1.f, 1.f};
        light.diffuse_intensity = .85f;
        light.ambient_intensity = .2f;
        light.specular_intensity = 2.f;

        bool quit{false};
        bool resize{false};
        float mouse_sensibility{1};

        //Create Event handler
        SDL_Event event;
        float frame_cap {1.f / 60 * 1000};
        float last_frame_time {0};

        float fixed_frame_time {0.03f};
        float accumulated_time {0};

        unsigned int mouse_x{swapchain.Extent()->width/2}, mouse_y{swapchain.Extent()->height/2};
        SDL_WarpMouseInWindow(vulkan_context.Window(), mouse_x, mouse_y);//centre mouse

        bool drag{false};
        bool w{false}, a{false}, s{false}, d{false};
        bool locked{false};

        while(!quit)
        {
            auto start_time(std::chrono::high_resolution_clock::now());
            float fps{0};

            accumulated_time += last_frame_time;

            // if(drag)//right mouse down
            if(!locked && drag)
            {
                SDL_SetRelativeMouseMode(SDL_TRUE);
                locked = true;
            }
            else if(locked && !drag)
            {
                SDL_SetRelativeMouseMode(SDL_FALSE);
                locked = false;
                SDL_WarpMouseInWindow(vulkan_context.Window(), mouse_x, mouse_y);//centre mouse on btn release
            }

            //<f> Read inputs
            //Handle events on queue
            while( SDL_PollEvent( &event ) != 0 )
            {
                // OnWindowEvent(&event, &resize);

                //User requests quit
                if( event.type == SDL_QUIT)
                {
                    quit = true;
                    break;
                }

                if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT)
                    drag = true;
                if(event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT)
                    drag = false;

                if(event.type == SDL_KEYDOWN)
                {
                    if(event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        quit = true;
                        break;
                    }

                    if(event.key.keysym.sym == SDLK_w)
                        w = true;
                    if(event.key.keysym.sym == SDLK_a)
                        a = true;
                    if(event.key.keysym.sym == SDLK_s)
                        s = true;
                    if(event.key.keysym.sym == SDLK_d)
                        d = true;
                }

                if(event.type == SDL_KEYUP)
                {
                    if(event.key.keysym.sym == SDLK_w)
                        w = false;
                    if(event.key.keysym.sym == SDLK_a)
                        a = false;
                    if(event.key.keysym.sym == SDLK_s)
                        s = false;
                    if(event.key.keysym.sym == SDLK_d)
                        d = false;
                }

                // if(event.type == SDL_MOUSEMOTION)
                if(event.type == SDL_MOUSEMOTION && drag && locked)
                {
                    if(event.motion.xrel > 0)
                    {
                        camera.LookRight(event.motion.xrel * last_frame_time * mouse_sensibility);
                    }
                    else if(event.motion.xrel < 0)
                    {
                        camera.LookLeft(-event.motion.xrel * last_frame_time * mouse_sensibility);
                    }

                    if(event.motion.yrel > 0)//origin is top left
                    {
                        camera.LookDown(event.motion.yrel * last_frame_time * mouse_sensibility);
                    }
                    else if(event.motion.yrel < 0)
                    {
                        camera.LookUp(-event.motion.yrel * last_frame_time * mouse_sensibility);
                    }
                }
                if(event.type == SDL_MOUSEWHEEL)
                {
                    if(event.wheel.y > 0)
                    {
                        auto fov = camera.FOVDegrees();
                        fov -= 1;
                        if(fov < 20)
                            fov = 20;
                        camera.FOVDegrees(fov);
                    }
                    else if(event.wheel.y < 0)
                    {
                        auto fov = camera.FOVDegrees();
                        fov += 1;
                        if(fov > 60)
                            fov = 60;
                        camera.FOVDegrees(fov);
                    }
                }

                if(event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_KP_MINUS || event.key.keysym.sym == SDLK_MINUS) )
                {
                    mouse_sensibility -= 0.1;
                    if(mouse_sensibility <= 0)
                        mouse_sensibility = 0.1;
                    std::cout<<mouse_sensibility<<std::endl;
                }
                if(event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_KP_PLUS || event.key.keysym.sym == SDLK_PLUS) )
                {
                    mouse_sensibility += 0.1;
                    if(mouse_sensibility > 3)
                        mouse_sensibility = 3;
                    std::cout<<mouse_sensibility<<std::endl;
                }

                //<f> Camera Jumps
                if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_1)
                {
                    camera.Position({-4.51862f, -93.4624f, 299.656f});
                    camera.Rotation({16.f, 180.f, 0.f});
                }
                else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_2)
                {
                    camera.Position({-4.06494f, 403.914f, 622.851f});
                    camera.Rotation({40.6667f, 180.167f, 0.f});
                }
                // else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_3)
                // {
                //     camera.Position({-809.102f, -12.6976f, 10.0665f});
                //     camera.Rotation({48.7665f, 183.617f, 0.f});
                // }
                //</f> /Camera Jumps
                if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_c)
                {
                    std::cout<<"C P: ("<<camera.Position().x<<", "<<camera.Position().y<<", "<<camera.Position().z<<")"<<std::endl;
                    std::cout<<"C R: ("<<camera.Rotation().x<<", "<<camera.Rotation().y<<", "<<camera.Rotation().z<<")"<<std::endl;
                }

                if(models_pointers.ValidTest())
                {
                    auto pos = models_pointers.test_model->Position();
                    auto rot = models_pointers.test_model->Rotation();
                    int mod = 1;

                    if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT)
                        pos.x += 0.1 * mod;
                    if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT)
                        pos.x -= 0.1 * mod;
                    if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN)
                        pos.z += 0.1 * mod;
                    if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP)
                        pos.z -= 0.1 * mod;
                    if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_PAGEDOWN)
                        pos.y -= 0.1 * mod;
                    if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_PAGEUP)
                        pos.y += 0.1 * mod;

                    if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q)
                        rot.y += 90;
                    if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_e)
                        rot.y -= 90;

                    if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r)
                    {
                        std::cout<<"P: ("<<pos.x<<", "<<pos.y<<", "<<pos.z<<")"<<std::endl;
                        std::cout<<"R: ("<<rot.x<<", "<<rot.y<<", "<<rot.z<<")"<<std::endl;
                    }

                    models_pointers.test_model->Position(pos);
                    models_pointers.test_model->Rotation(rot);
                }
            }
            //</f> /Read inputs

            //<f> Apply input
            if(w) camera.MoveForward(100 * last_frame_time);
            if(a) camera.MoveLeft(100 * last_frame_time);
            if(s) camera.MoveBackward(100 * last_frame_time);
            if(d) camera.MoveRight(100 * last_frame_time);
            //</f> /Apply input

            //apply any camera change promoted by inputs
            camera.Update();

            // light.light_position = glm::vec4(camera.Position(), 1);

            //Fixed time step Logic
            while(accumulated_time >= fixed_frame_time)
            {
                // FixedLogic(fixed_frame_time) Function call
                accumulated_time -= fixed_frame_time;
            }

            //<f> Process Models demos
            ProcessFlags(models_pointers, last_frame_time);
            ProcessPatrols(models_pointers, last_frame_time);
            //</f> /Process Models demos

            //Variable time step Logic
            // Logic(last_frame_time) Function call
            for(auto i{0}; i<models.size(); ++i)
                models[i]->UpdateUniformBuffer(last_frame_time, &camera, light);

            //Clear screen
            // SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0x00 );
            // SDL_RenderClear( renderer );

			// Render(renderer, last_frame_time) Function call

			//Update screen
			commandbuffer.DrawFrame(&swapchain, &resize);

            if(resize)
            {
                resize = false;
                Resize(&vulkan_resources, &vulkan_context, &swapchain, &renderpass, &pipelines, &framebuffer, &commandbuffer);
            }

            //Update frame timers
            auto delta_time(std::chrono::high_resolution_clock::now() - start_time);
            float frame_time = std::chrono::duration_cast< std::chrono::duration<float, std::milli> >(delta_time).count();

            //fps cap
            if(frame_time < frame_cap)
            {
                SDL_Delay(frame_cap - frame_time);
                frame_time = frame_cap;
            }
            frame_time /= 1000.f;
            fps = 1.f / frame_time;

            last_frame_time = frame_time;

            SDL_SetWindowTitle(window, ( sdl_config.window_name +" - "+ std::to_string(fps)+" FPS").c_str() );
        }// while(!quit)

        //terminate vulkan
        vulkan_context.WaitForIdle();

        resource_manager.Cleanup();

        pipelines.Cleanup();
        framebuffer.Cleanup();
        renderpass.Cleanup();
        swapchain.Cleanup();
        commandbuffer.Cleanup();

        vulkan_resources.Cleanup();
        vulkan_context.Cleanup();
    }//else run
    //terminate SDL subsystems
    TerminateSDL();
    DeleteSDLPointers(window);//they are deleted in a sepecific way
    window = nullptr;

    return 0;
}
