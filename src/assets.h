#include <map>
#include <memory>
#include <string>
#include <variant>

class Sprite;
class Texture;
struct SDL_Renderer;

using Asset = std::variant<std::shared_ptr<Texture>, std::shared_ptr<Sprite>>;

using AssetMap = std::map<std::string, Asset>;

AssetMap loadAssets(const std::string&, SDL_Renderer* renderer);

template <typename T>
std::shared_ptr<T> get(const AssetMap& assets, const std::string& asset_name) {
    return std::get<std::shared_ptr<T>>(assets.at(asset_name));
}