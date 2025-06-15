#include "Enemy.h"
#include <algorithm>
#include <cmath>

Enemy::Enemy(float x, float y, MovementPattern pattern)
    : GameObject(x, y, 40.0f, 40.0f, glm::vec4(1.0f, 0.5f, 0.5f, 1.0f),
                 ObjectType::OBSTACLE, false),
      movementPattern(pattern), movementSpeed(100.0f), time(0.0f),
      originalPosition(x, y), movementRange(100.0f), movingToB(false),
      circleRadius(50.0f) {
  // Set default movement patterns based on type
  switch (pattern) {
  case MovementPattern::HORIZONTAL:
    setHorizontalMovement(150.0f);
    break;
  case MovementPattern::VERTICAL:
    setVerticalMovement(150.0f);
    break;
  case MovementPattern::CIRCULAR:
    setCircularMovement(glm::vec2(x, y), 80.0f);
    break;
  case MovementPattern::PATROL:
    setPatrolMovement(glm::vec2(x - 100, y), glm::vec2(x + 100, y));
    break;
  }
}

void Enemy::setHorizontalMovement(float range) {
  movementPattern = MovementPattern::HORIZONTAL;
  movementRange = range;
  originalPosition = glm::vec2(bounds.x, bounds.y);
}

void Enemy::setVerticalMovement(float range) {
  movementPattern = MovementPattern::VERTICAL;
  movementRange = range;
  originalPosition = glm::vec2(bounds.x, bounds.y);
}

void Enemy::setCircularMovement(glm::vec2 center, float radius) {
  movementPattern = MovementPattern::CIRCULAR;
  circleCenter = center;
  circleRadius = radius;
}

void Enemy::setPatrolMovement(glm::vec2 pointA, glm::vec2 pointB) {
  movementPattern = MovementPattern::PATROL;
  patrolPointA = pointA;
  patrolPointB = pointB;
  movingToB = true;
}

void Enemy::update(float deltaTime) {
  time += deltaTime;

  switch (movementPattern) {
  case MovementPattern::HORIZONTAL: {
    float oscillation = sin(time * movementSpeed / 50.0f);
    bounds.x = originalPosition.x + oscillation * (movementRange / 2.0f);
    break;
  }

  case MovementPattern::VERTICAL: {
    float oscillation = sin(time * movementSpeed / 50.0f);
    bounds.y = originalPosition.y + oscillation * (movementRange / 2.0f);
    break;
  }

  case MovementPattern::CIRCULAR: {
    float angle = time * movementSpeed / 100.0f;
    bounds.x = circleCenter.x + cos(angle) * circleRadius - bounds.width / 2.0f;
    bounds.y =
        circleCenter.y + sin(angle) * circleRadius - bounds.height / 2.0f;
    break;
  }

  case MovementPattern::PATROL: {
    glm::vec2 currentPos(bounds.x, bounds.y);
    glm::vec2 target = movingToB ? patrolPointB : patrolPointA;

    glm::vec2 direction = target - currentPos;
    float distance = glm::length(direction);

    if (distance < 5.0f) {
      // Switch direction when reaching target
      movingToB = !movingToB;
    } else {
      // Move towards target
      glm::vec2 normalizedDirection = glm::normalize(direction);
      float moveDistance = movementSpeed * deltaTime;

      bounds.x += normalizedDirection.x * moveDistance;
      bounds.y += normalizedDirection.y * moveDistance;
    }
    break;
  }
  }
}