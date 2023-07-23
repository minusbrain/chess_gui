#pragma once

#include <SDL.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "SDL_image.h"
#include "sdl_helper.h"

class SdlContext;
class Texture;

class Sprite {
   public:
    struct RelRect {
        float x;
        float y;
        float w;
        float h;
    };

    Sprite(const std::shared_ptr<Texture> &texture, const SDL_Rect &rect);
    void drawtoGui(float scale) const;
    bool drawToGuiAsButton(const std::string &buttonId, float scale) const;

    virtual ~Sprite();

   private:
    const std::shared_ptr<Texture> _texture;
    SDL_Rect _rect;
    RelRect _relRect;
};

inline std::ostream &operator<<(std::ostream &os, const Sprite::RelRect &r) {
    os << "Sprite::RelRect{x=" << r.x << ",y=" << r.y << ",w=" << r.w << ",h=" << r.h << "}";
    return os;
}