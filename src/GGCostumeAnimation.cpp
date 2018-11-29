#include "GGCostumeAnimation.h"

namespace gg
{
GGCostumeAnimation::GGCostumeAnimation(const std::string &name, sf::Texture &texture)
    : _texture(texture), _name(name), _state(AnimationState::Pause), _loop(false)
{
}

GGCostumeAnimation::~GGCostumeAnimation() = default;

void GGCostumeAnimation::play(bool loop)
{
    _loop = loop;
    _state = AnimationState::Play;
}

void GGCostumeAnimation::update(const sf::Time &elapsed)
{
    if (!isPlaying())
        return;
    bool isFinished = !_loop;
    for (auto &layer : _layers)
    {
        isFinished &= layer->update(elapsed);
    }
    if (isFinished)
    {
        pause();
    }
}

void GGCostumeAnimation::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    for (auto &layer : _layers)
    {
        if (!layer->getVisible())
            continue;

        auto frame = layer->getIndex();
        auto &rect = layer->getFrames()[frame];
        auto &sourceRect = layer->getSourceFrames()[frame];
        auto size = layer->getSizes()[frame];
        sf::Vector2i offset;
        if (!layer->getOffsets().empty())
        {
            offset = layer->getOffsets()[frame];
        }
        sf::Sprite sprite(_texture, rect);
        sprite.setOrigin(sf::Vector2f(size.x / 2.f - sourceRect.left + offset.x, size.y / 2.f - sourceRect.top + offset.y));
        target.draw(sprite, states);
    }
}
} // namespace gg
