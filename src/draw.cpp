//
// Copyright 2023 Suzuki Yoshinori(wave.suzuki.z@gmail.com)
// since: 26 Apr 2023
//
#include "draw.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_render.h>
#include <array>

namespace
{
//
SDL_Window* window{nullptr};
//
SDL_Renderer* renderer{nullptr};
//
struct Font
{
    SDL_Surface* surf   = nullptr;
    SDL_Texture* tex    = nullptr;
    int          width  = 0;
    int          height = 0;

    //
    [[nodiscard]] bool isValid() const { return surf && tex; }
    //
    void setup(TTF_Font* font, const char* str, SDL_Color color)
    {
        surf   = TTF_RenderUTF8_Blended(font, str, color);
        tex    = SDL_CreateTextureFromSurface(renderer, surf);
        width  = surf->w;
        height = surf->h;
    }
    //
    int print(int x, int y)
    {
        auto rect = SDL_Rect{x, y, width, height};
        SDL_RenderCopy(renderer, tex, nullptr, &rect);
        return x + width;
    }
};

//
std::array<Font, 256> asciiFontMap;

//
void
FontSetup()
{
    TTF_Init();

    auto* font = TTF_OpenFont("res/IBMPlexSansJP-Light.ttf", PointSize);

    const char charMap[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz:&!#$%&\"'()^=*-[]+/?,.@`~";
    for (auto ch : charMap)
    {
        if (ch == '\0')
        {
            break;
        }
        auto& f = asciiFontMap[ch];
        char  buff[2]{ch, '\0'};
        f.setup(font, buff, {255, 255, 255, 255});
    }

    TTF_CloseFont(font);
    TTF_Quit();
}

} // namespace

//
void
CreateWindow(const char* title, bool withJoystick)
{
    if (withJoystick)
    {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
        SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");
    }
    else
    {
        SDL_Init(SDL_INIT_VIDEO);
    }

    auto* window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ScreenWidth, ScreenHeight, 0);
    renderer     = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer)
    {
        FontSetup();
    }
}

//
void
DestroyWindow()
{
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
    }
    if (window)
    {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

//
void
FrameUpdate()
{
    SDL_SetRenderDrawColor(renderer, 0, 10, 30, 255);
    SDL_RenderClear(renderer);

    SDL_JoystickUpdate();
}

//
void
FrameSync()
{
    SDL_RenderPresent(renderer);
}

//
void
Print(int x, int y, const char* msg)
{
    while (*msg != '\0')
    {
        int   ch   = *msg;
        auto& font = asciiFontMap[ch];
        if (font.isValid())
        {
            x = font.print(x, y);
        }
        msg++;
    }
}

//
void
SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

//
void
FillRect(int x, int y, int w, int h)
{
    auto r = SDL_Rect{x, y, w, h};
    SDL_RenderFillRect(renderer, &r);
}

//
void
DrawRect(int x, int y, int w, int h)
{
    auto r = SDL_Rect{x, y, w, h};
    SDL_RenderDrawRect(renderer, &r);
}

//
void
DrawPoint(int x, int y)
{
    SDL_RenderDrawPoint(renderer, x, y);
}
