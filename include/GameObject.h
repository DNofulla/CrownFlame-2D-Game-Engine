#pragma once
#include <glm/glm.hpp>

struct Rectangle {
  float x, y;
  float width, height;

  Rectangle(float x = 0, float y = 0, float w = 0, float h = 0)
      : x(x), y(y), width(w), height(h) {}
};

enum class ObjectType { PLAYER, OBSTACLE, COLLECTIBLE, ENEMY };

class GameObject {
public:
  Rectangle bounds;
  glm::vec4 color;
  ObjectType type;
  bool isStatic;

  GameObject(float x, float y, float width, float height, glm::vec4 color,
             ObjectType type, bool isStatic = false);

  // Get center position
  glm::vec2 getCenter() const;

  // Update position (for non-static objects)
  void setPosition(float x, float y);
  void move(float deltaX, float deltaY);

  // Collision detection
  bool isColliding(const GameObject &other) const;
  bool isColliding(const Rectangle &rect) const;

  // Keep object within bounds
  void constrainToBounds(int screenWidth, int screenHeight);
};

namespace CollisionDetection {
bool checkRectangleCollision(const Rectangle &rect1, const Rectangle &rect2);
bool checkPointInRectangle(float pointX, float pointY, const Rectangle &rect);
} // namespace CollisionDetection