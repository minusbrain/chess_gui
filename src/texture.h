#include <SDL.h>
#include <base/helpers.h>

#include <string>

#include "sdl_helper.h"

class Texture : base::NONCOPYABLE {
   public:
    Texture(SDL_Renderer *renderer, const std::string &file, const SDL_Point &transparencyPicker);
    Texture() = delete;
    Texture(Texture &&) = default;

    Texture &operator=(Texture &&) = default;

    virtual ~Texture();
    virtual bool is_valid() const;

    const SDL_Texture *getRaw() const;
    int getWidth() const;
    int getHeight() const;

   private:
    SDL_Texture *_texture;
    int _width;
    int _height;
};