#include "GGObject.h"
#include "Screen.h"

namespace gg
{
GGObject::GGObject()
    : _pAnim(std::nullopt),
      _isVisible(true),
      _zorder(0),
      _direction(UseDirection::Front),
      _prop(false),
      _color(sf::Color::White),
      _angle(0),
      _isTouchable(true),
      _pOwner(nullptr),
      _pRoom(nullptr),
      _state(0)
{
}

GGObject::~GGObject() = default;

void GGObject::setVisible(bool isVisible)
{
    _isVisible = isVisible;
    if (!_isVisible)
    {
        _isTouchable = false;
    }
}

sf::IntRect GGObject::getRealHotspot() const
{
    auto rect = getHotspot();
    auto pos = getPosition();
    return (sf::IntRect)_transform.getTransform().transformRect((sf::FloatRect)rect);
}

void GGObject::setStateAnimIndex(int animIndex)
{
    std::ostringstream s;
    s << "state" << animIndex;
    _state = animIndex;
    if (animIndex == 4)
    {
        _isTouchable = false;
        _isVisible = false;
    }
    setAnimation(s.str());
}

int GGObject::getStateAnimIndex()
{
    if (!_pAnim.has_value())
        return -1;
    if (_pAnim->getName().find("state") == std::string::npos)
        return -1;
    return std::strtol(_pAnim->getName().c_str(), nullptr, 10);
}

void GGObject::setAnimation(const std::string &name)
{
    auto it = std::find_if(_anims.begin(), _anims.end(), [name](std::unique_ptr<GGAnimation> &animation) { return animation->getName() == name; });
    if (it == _anims.end())
    {
        _pAnim = std::nullopt;
        return;
    }

    auto &anim = *(it->get());
    _pAnim = anim;
    _pAnim->setObject(this);
    auto &sprite = _pAnim->getSprite();
    sprite.setColor(_color);
}

void GGObject::move(const sf::Vector2f &offset)
{
    _transform.move(offset);
}

void GGObject::setPosition(const sf::Vector2f &pos)
{
    _transform.setPosition(pos);
}

sf::Vector2f GGObject::getPosition() const
{
    return _transform.getPosition();
}

void GGObject::setUsePosition(const sf::Vector2f &pos)
{
    _usePos = pos;
}

sf::Vector2f GGObject::getUsePosition() const
{
    return _usePos;
}

void GGObject::setColor(const sf::Color &color)
{
    _color = color;
    if (_pAnim)
    {
        _pAnim->getSprite().setColor(color);
    }
}

const sf::Color &GGObject::getColor() const
{
    return _color;
}

void GGObject::setScale(float s)
{
    _transform.setScale(s, s);
}

void GGObject::update(const sf::Time &elapsed)
{
    if (_pAnim)
    {
        _pAnim->update(elapsed);
    }
}

void GGObject::drawHotspot(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform *= _transform.getTransform();
    auto rect = getHotspot();

    sf::RectangleShape s(sf::Vector2f(rect.width, rect.height));
    s.setPosition(rect.left, rect.top);
    s.setOutlineThickness(1);
    s.setOutlineColor(sf::Color::Red);
    s.setFillColor(sf::Color::Transparent);
    target.draw(s, states);

    sf::RectangleShape vl(sf::Vector2f(1, 5));
    vl.setPosition(_usePos.x, _usePos.y - 2);
    vl.setFillColor(sf::Color::Red);
    target.draw(vl, states);

    sf::RectangleShape hl(sf::Vector2f(5, 1));
    hl.setPosition(_usePos.x - 2, _usePos.y);
    hl.setFillColor(sf::Color::Red);
    target.draw(hl, states);
}

void GGObject::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!_isVisible)
        return;
    states.transform *= _transform.getTransform();

    if (_pAnim)
    {
        target.draw(*_pAnim, states);
    }
}

std::ostream &operator<<(std::ostream &os, const GGObject &obj)
{
    return os << obj.getName() << " (" << obj.getPosition().x << "," << obj.getPosition().y << ":" << obj.getZOrder() << ")";
}

} // namespace gg
