#pragma once
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/Sprite.h>
#include <engge/Graphics/Text.hpp>
#include "ControlConstants.hpp"

namespace ng {
class Slider final : public ngf::Drawable {
public:
  typedef std::function<void(float)> Callback;

  Slider(int id, float y, bool enabled = true, float value = 0.f, Callback callback = nullptr)
      : _id(id), _isEnabled(enabled), _y(y), _value(value), onValueChanged(callback) {
  }

  void setEngine(Engine *pEngine) {
    _pEngine = pEngine;
  }

  void setSpriteSheet(SpriteSheet *pSpriteSheet) {
    const auto &uiFontMedium = _pEngine->getResourceManager().getFntFont("UIFontMedium.fnt");
    _text.setFont(uiFontMedium);
    _text.setWideString(Engine::getText(_id));
    auto textRect = _text.getLocalBounds();
    _text.getTransform().setOrigin({textRect.getWidth() / 2.f, textRect.getHeight() / 2.f});
    _text.getTransform().setPosition({Screen::Width / 2.f, _y});

    auto sliderRect = pSpriteSheet->getRect("slider");
    auto handleRect = pSpriteSheet->getRect("slider_handle");
    glm::vec2 scale(Screen::Width / 320.f, Screen::Height / 180.f);
    _sprite.getTransform().setPosition({Screen::Width / 2.f, _y});
    _sprite.getTransform().setScale(scale);
    _sprite.getTransform().setOrigin({sliderRect.getWidth() / 2.f, 0});
    _sprite.setTexture(*pSpriteSheet->getTexture());
    _sprite.setTextureRect(sliderRect);

    _min = Screen::Width / 2.f - (sliderRect.getWidth() * scale.x / 2.f);
    _max = Screen::Width / 2.f + (sliderRect.getWidth() * scale.x / 2.f);
    auto x = _min + _value * (_max - _min);
    _spriteHandle.getTransform().setPosition({x, _y});
    _spriteHandle.getTransform().setScale(scale);
    _spriteHandle.getTransform().setOrigin({handleRect.getWidth() / 2.f, 0});
    _spriteHandle.setTexture(*pSpriteSheet->getTexture());
    _spriteHandle.setTextureRect(handleRect);
  }

  void update(glm::vec2 pos) {
    auto textRect = ng::getGlobalBounds(_sprite);
    bool isDown = ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left);
    if (!isDown) {
      _isDragging = false;
    }
    ngf::Color color;
    if (!_isEnabled) {
      color = ControlConstants::DisabledColor;
    } else if (textRect.contains(pos)) {
      color = ControlConstants::HoveColor;
      ImGuiIO &io = ImGui::GetIO();
      if (!io.WantCaptureMouse && isDown) {
        _isDragging = true;
      }
    } else {
      color = ControlConstants::NormalColor;
    }
    _sprite.setColor(color);
    _text.setColor(color);

    if (_isDragging) {
      auto x = std::clamp(pos.x, _min, _max);
      auto value = (x - _min) / (_max - _min);
      if (_value != value) {
        _value = value;
        if (onValueChanged) {
          onValueChanged.value()(value);
        }
      }
      _spriteHandle.getTransform().setPosition({x, _spriteHandle.getTransform().getPosition().y});
    }
  }

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override {
    _text.draw(target, states);
    _sprite.draw(target, states);
    _spriteHandle.draw(target, states);
  }

private:
  Engine *_pEngine{nullptr};
  int _id{0};
  bool _isEnabled{true};
  float _y{0};
  float _min{0}, _max{0}, _value{0};
  bool _isDragging{false};
  ngf::Sprite _sprite;
  ngf::Sprite _spriteHandle;
  ng::Text _text;
  std::optional<Callback> onValueChanged;
};
}
