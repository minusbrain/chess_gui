#include "sprite.h"

#include <imgui.h>

#include <algorithm>
#include <map>
#include <random>
#include <sstream>
#include <vector>

#include "texture.h"

Sprite::Sprite(const std::shared_ptr<Texture> &texture, const SDL_Rect &rect) : _texture(texture), _rect(rect), _relRect() {
    _relRect.x = _rect.x / (float)texture->getWidth();
    _relRect.y = _rect.y / (float)texture->getHeight();
    _relRect.w = _relRect.x + _rect.w / (float)texture->getWidth();
    _relRect.h = _relRect.y + _rect.h / (float)texture->getHeight();
}

Sprite::~Sprite() {}

void Sprite::drawtoGui(float scale) const {
    ImGui::Image((void *)(intptr_t)_texture->getRaw(), ImVec2(_rect.w * scale, _rect.h * scale), ImVec2(_relRect.x, _relRect.y),
                 ImVec2(_relRect.w, _relRect.h));
}

bool Sprite::drawToGuiAsButton(const std::string &buttonId, float scale) const {
    return ImGui::ImageButton(buttonId.c_str(), (void *)(intptr_t)_texture->getRaw(), ImVec2(_rect.w * scale, _rect.h * scale),
                              ImVec2(_relRect.x, _relRect.y), ImVec2(_relRect.w, _relRect.h));
}
