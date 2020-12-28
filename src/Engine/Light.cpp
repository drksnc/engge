#include "engge/Engine/Light.hpp"
#include "engge/System/Locator.hpp"
#include "engge/Engine/EntityManager.hpp"

namespace ng {
Light::Light(ngf::Color color, glm::ivec2 pos)
    : _color(color), _pos(pos) {
  sq_resetobject(&_table);
  _id = Locator<EntityManager>::get().getLightId();
}

Light::~Light() = default;

} // namespace ng
