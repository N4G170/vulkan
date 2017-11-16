#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include <iostream>
#include "config.hpp"

#ifndef SDL_INIT_HPP
#define SDL_INIT_HPP


/**
 * \brief Struct with operator() overrides of SDL pointers correct destruction (to be used by unique_ptr)
 */
struct SDLDeleters
{
    void operator() (SDL_Window* window)
    {
        //MessageWriter::Instance()->WriteLineToConsole("Calling destroy for SDL_window object pointer...");
        std::cout << "Calling destroy for SDL_window object pointer... \n";
        SDL_DestroyWindow(window);
    }

    void operator() (SDL_Renderer* screen_renderer)
    {
        //MessageWriter::Instance()->WriteLineToConsole("Calling destroy for SDL_Renderer object pointer...");
        std::cout << "Calling destroy for SDL_Renderer object pointer... \n";
        SDL_DestroyRenderer(screen_renderer);
    }

    void operator() (TTF_Font* font)
    {
        //MessageWriter::Instance()->WriteLineToConsole("Calling destroy for TTF_Font object pointer...");
        std::cout << "Calling destroy for TTF_Font object pointer... \n";
        TTF_CloseFont( font );
    }
};

bool InitSDL(SDL_Window*& window, const SDLInitConfig& config = {});

/**
 * \brief Terminate SDL subsystems.\n
 * You are responsible for clearing any SDL pointer used (ex: window, renderer), after Terminate is called
 */
void TerminateSDL();

void DeleteSDLPointers(SDL_Window* window);

#endif //SDL_INIT_HPP
