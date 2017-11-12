#include "sdl_init.hpp"

bool InitSDL(SDL_Window*& window, const SDLInitConfig& config)
{


    //Initialize SDL
    int result = SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO );
    if( result < 0 )
    {
        std::cout<<"SDL could not initialize! SDL_Error: "<<SDL_GetError()<<std::endl;

        return false;
    }
    else
    {
        //Create window
        window = SDL_CreateWindow( config.window_name.c_str(), config.window_x, config.window_y, config.window_w, config.window_h, config.window_flags);
        if( window == nullptr )
        {
            std::cout<<"Window could not be created! SDL_Error: "<<SDL_GetError()<<std::endl;

            return false;
        }
    }

    return true;
}

void TerminateSDL()
{
    //Quit SDL subsystems
    // Mix_Quit();
    // TTF_Quit();
    // IMG_Quit();
    SDL_Quit();
}

void DeleteSDLPointers(SDL_Window* window)
{
    if(window)
        SDL_DestroyWindow(window);
}
