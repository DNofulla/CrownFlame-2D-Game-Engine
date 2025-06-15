#include "GameObject.h"
#include <algorithm>

GameObject::GameObject(float x, float y, float width, float height,
                       glm::vec4 color, ObjectType type, bool isStatic)
    : bounds(x, y, width, height), color(color), type(type),
      isStatic(isStatic) {}

glm::vec2 GameObject::getCenter() const {
  return glm::vec2(bounds.x + bounds.width / 2.0f,
                   bounds.y + bounds.height / 2.0f);
}

void GameObject::setPosition(float x, float y) {
  if (!isStatic) {
    bounds.x = x;
    bounds.y = y;
  }
}

void GameObject::move(float deltaX, float deltaY) {
  if (!isStatic) {
    bounds.x += deltaX;
    bounds.y += deltaY;
  }
}

bool GameObject::isColliding(const GameObject &other) const {
  return CollisionDetection::checkRectangleCollision(bounds, other.bounds);
}

bool GameObject::isColliding(const Rectangle &rect) const {
  return CollisionDetection::checkRectangleCollision(bounds, rect);
}

void GameObject::constrainToBounds(int screenWidth, int screenHeight) {
  if (!isStatic) {
    bounds.x =
        std::max(0.0f, std::min(bounds.x, static_cast<float>(screenWidth) -
                                              bounds.width));
    bounds.y =
        std::max(0.0f, std::min(bounds.y, static_cast<float>(screenHeight) -
                                              bounds.height));
  }
}

namespace CollisionDetection {
bool checkRectangleCollision(const Rectangle &rect1, const Rectangle &rect2) {
  return (rect1.x < rect2.x + rect2.width && rect1.x + rect1.width > rect2.x &&
          rect1.y < rect2.y + rect2.height && rect1.y + rect1.height > rect2.y);
}

bool checkPointInRectangle(float pointX, float pointY, const Rectangle &rect) {
  return (pointX >= rect.x && pointX <= rect.x + rect.width &&
          pointY >= rect.y && pointY <= rect.y + rect.height);
}
} // namespace CollisionDetection