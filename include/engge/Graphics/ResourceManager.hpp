#pragma once
#include <map>
#include <memory>
#include <engge/System/NonCopyable.hpp>
#include <ngf/Graphics/Texture.h>

namespace ng {
class GGFont;
class FntFont;
class SpriteSheet;

struct TextureResource {
  std::shared_ptr<ngf::Texture> _texture;
  size_t _size;
};

class ResourceManager : public NonCopyable {
private:
  std::map<std::string, TextureResource> _textureMap;
  std::map<std::string, std::shared_ptr<GGFont>> _fontMap;
  std::map<std::string, std::shared_ptr<FntFont>> _fntFontMap;
  std::map<std::string, std::shared_ptr<SpriteSheet>> _spriteSheetMap;

public:
  ResourceManager();
  ~ResourceManager();

  std::shared_ptr<ngf::Texture> getTexture(const std::string &id);
  GGFont &getFont(const std::string &id);
  FntFont &getFntFont(const std::string &id);
  const SpriteSheet &getSpriteSheet(const std::string &id);

  [[nodiscard]] const std::map<std::string, TextureResource> &getTextureMap() const { return _textureMap; }

private:
  void load(const std::string &id);
  void loadFont(const std::string &id);
  void loadFntFont(const std::string &id);
  void loadSpriteSheet(const std::string &id);
};
} // namespace ng