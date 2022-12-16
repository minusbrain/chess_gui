
#include "assets.h"

#include <SDL_render.h>

#include <fstream>
#include <memory>

#include "json_helper.h"
#include "sprite.h"
#include "texture.h"

AssetMap loadAssets(const std::string& asset_file, SDL_Renderer* renderer) {
    AssetMap assets;
    std::ifstream is{asset_file};
    if (!is.is_open()) {
        throw std::invalid_argument{"Could not open file " + asset_file};
    }
    json j = json::parse(is);

    auto textures = get_element_by_path(j, "/textures");
    if (textures) {
        for (auto texture : *textures) {
            std::string tex_name = json_child_or_fail<std::string>(texture, "name");
            std::string tex_file = json_child_or_fail<std::string>(texture, "file");
            int tex_transx = json_child_or<int>(texture, "transparencyX", 0);
            int tex_transy = json_child_or<int>(texture, "transparencyY", 0);

            auto tex = std::make_shared<Texture>(renderer, tex_file, SDL_Point{tex_transx, tex_transy});
            assets.emplace(tex_name, std::move(tex));
        }

        auto sprites = get_element_by_path(j, "/sprites");
        if (sprites) {
            for (auto sprite : *sprites) {
                std::string spr_name = json_child_or_fail<std::string>(sprite, "name");
                std::string spr_tex_name = json_child_or_fail<std::string>(sprite, "texture");
                int spr_posx = json_child_or<int>(sprite, "posx", 0);
                int spr_posy = json_child_or<int>(sprite, "posy", 0);
                int spr_sizex = json_child_or<int>(sprite, "sizex", 0);
                int spr_sizey = json_child_or<int>(sprite, "sizey", 0);
                SDL_Rect spr_rect{spr_posx, spr_posy, spr_sizex, spr_sizey};

                auto tex = std::get<std::shared_ptr<Texture>>(assets.at(spr_tex_name));

                // std::cout << "Loaded: " << spr_name << " " << spr_rect << " " << tex << " " << tex->getHeight() << '\n';

                auto spr = std::make_shared<Sprite>(tex, spr_rect);
                assets.emplace(spr_name, std::move(spr));
            }
        }
    }
    return assets;
}
