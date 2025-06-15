#pragma once
#include "GameObject.h"
#include <glm/glm.hpp>

enum class MovementPattern {
  HORIZONTAL, // Move left and right
  VERTICAL,   // Move up and down
  CIRCULAR,   // Move in a circle around a point
  PATROL      // Move between two points
};

class Enemy : public GameObject {
private:
  MovementPattern movementPattern;
  float movementSpeed;
  float time; // For tracking animation/movement time

  // Pattern-specific data
  glm::vec2 originalPosition;
  glm::vec2 patrolPointA, patrolPointB;
  glm::vec2 circleCenter;
  float circleRadius;
  float movementRange;
  bool movingToB; // For patrol pattern

public:
  Enemy(float x, float y,
        MovementPattern pattern = MovementPattern::HORIZONTAL);

  // Movement pattern setup
  void setHorizontalMovement(float range);
  void setVerticalMovement(float range);
  void setCircularMovement(glm::vec2 center, float radius);
  void setPatrolMovement(glm::vec2 pointA, glm::vec2 pointB);

  // Update enemy position based on movement pattern
  void update(float deltaTime);

  // Getters
  MovementPattern getMovementPattern() const { return movementPattern; }
  float getMovementSpeed() const { return movementSpeed; }
  void setMovementSpeed(float speed) { movementSpeed = speed; }
};