#pragma once

#include <SDL.h>

#include <iostream>
#include <tuple>

#include "SDL_image.h"

static const SDL_Color WHITE = {255, 255, 255, 255};
static const SDL_Color BLACK = {0, 0, 0, 255};
static const SDL_Color RED = {255, 0, 0, 255};
static const SDL_Color GREEN = {0, 255, 0, 255};
static const SDL_Color BLUE = {0, 0, 255, 255};

inline int SDL_SetRenderDrawColor(SDL_Renderer *rend, const SDL_Color &color) {
    return SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a);
}

inline std::tuple<uint8_t, uint8_t, uint8_t> SDL_GetRGB(uint32_t pixel, SDL_PixelFormat *format) {
    uint8_t r, g, b;
    SDL_GetRGB(pixel, format, &r, &g, &b);

    return {r, g, b};
}

inline uint32_t SDL_GetPixel(SDL_Surface *surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1:
            return *p;
            break;

        case 2:
            return *(Uint16 *)p;
            break;

        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;
            break;

        case 4:
            return *(Uint32 *)p;
            break;

        default:
            return 0; /* shouldn't happen, but avoids warnings */
    }
}

inline SDL_Point operator+(SDL_Point left, SDL_Point const &right) {
    left.x += right.x;
    left.y += right.y;
    return left;
}

inline SDL_Point operator+(const SDL_Point &left, SDL_Point &&right) {
    right.x += left.x;
    right.y += left.y;
    return right;
}

inline SDL_Point &operator+=(SDL_Point &assignee, const SDL_Point &summand) {
    assignee.x += summand.x;
    assignee.y += summand.y;

    return assignee;
}

inline SDL_Point &operator-=(SDL_Point &assignee, const SDL_Point &summand) {
    assignee.x -= summand.x;
    assignee.y -= summand.y;

    return assignee;
}

inline std::ostream &operator<<(std::ostream &os, const SDL_Point &p) {
    os << "SDL_Point{x=" << p.x << ",y=" << p.y << "}";
    return os;
}

inline std::ostream &operator<<(std::ostream &os, const SDL_Rect &r) {
    os << "SDL_Rect{x=" << r.x << ",y=" << r.y << ",w=" << r.w << ",h=" << r.h << "}";
    return os;
}
