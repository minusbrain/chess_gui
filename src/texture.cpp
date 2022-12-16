#include "texture.h"

#include <SDL_image.h>

#include <string>

Texture::Texture(SDL_Renderer *renderer, const std::string &file, const SDL_Point &transparencyPicker)
    : _texture(nullptr), _width(0), _height(0) {
    SDL_Surface *loadedSurface = IMG_Load(file.c_str());
    if (loadedSurface == nullptr) {
        return;
    }

    _width = loadedSurface->w;
    _height = loadedSurface->h;

    auto rgb = SDL_GetRGB(SDL_GetPixel(loadedSurface, transparencyPicker.x, transparencyPicker.y), loadedSurface->format);
    SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, std::get<0>(rgb), std::get<1>(rgb), std::get<2>(rgb)));
    _texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);

    SDL_FreeSurface(loadedSurface);
}

bool Texture::is_valid() const { return _texture != nullptr; }

const SDL_Texture *Texture::getRaw() const { return _texture; }

int Texture::getWidth() const { return _width; }
int Texture::getHeight() const { return _height; }

Texture::~Texture() { SDL_DestroyTexture(_texture); }
