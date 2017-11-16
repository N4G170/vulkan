#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "SDL.h"
#include "SDL_image.h"
#include <string>

/**
 * \brief struck holding sdl init configurations
 */
struct SDLInitConfig
{
    //<f> Window
    std::string window_name {"SDL Vulkan Window"};
    int window_x {SDL_WINDOWPOS_UNDEFINED};
    int window_y {SDL_WINDOWPOS_UNDEFINED};
    int window_w {1760};
    int window_h {990};
    Uint32 window_flags {SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE};
    //</f>

    //<f> Main Renderer

    //</f>

    //<f> Image
    /**
     * \brief Var holding flags for image init.\n
     * Possible values IMG_INIT_JPG, IMG_INIT_PNG, IMG_INIT_TIF, to init more than one use bitwise or '|' ex: IMG_INIT_JPG | IMG_INIT_PNG
     */
    int image_flags {IMG_INIT_PNG};
    //</f>

    //<f> Audio

    //</f>
};

#endif//CONFIG_HPP
