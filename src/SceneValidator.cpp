#include "SceneValidator.h"
#include <algorithm>
#include <cmath>

SceneValidationResult
SceneValidator::validate(const SceneData::SceneDefinition &scene) {
  SceneValidationResult result;

  validateBasicProperties(scene, result);
  validateWorldSettings(scene, result);
  validatePlayerSpawn(scene, result);
  validateObstacles(scene, result);
  validateCollectibles(scene, result);
  validateEnemies(scene, result);
  validateObjectOverlaps(scene, result);
  validateReachability(scene, result);

  return result;
}

void SceneValidator::validateBasicProperties(
    const SceneData::SceneDefinition &scene, SceneValidationResult &result) {
  if (scene.name.empty()) {
    result.addError("Scene name cannot be empty", "scene.name");
  }

  if (scene.name.length() > 50) {
    result.addWarning("Scene name is very long (>50 characters)", "scene.name");
  }

  if (scene.transitionTrigger.empty()) {
    result.addWarning("No transition trigger specified",
                      "scene.transitionTrigger");
  }

  // Validate transition trigger values
  if (!scene.transitionTrigger.empty() &&
      scene.transitionTrigger != "collectibles_complete" &&
      scene.transitionTrigger != "enemies_defeat" &&
      scene.transitionTrigger != "manual") {
    result.addWarning("Unknown transition trigger: " + scene.transitionTrigger,
                      "scene.transitionTrigger");
  }
}

void SceneValidator::validateWorldSettings(
    const SceneData::SceneDefinition &scene, SceneValidationResult &result) {
  if (scene.world.width <= 0 || scene.world.height <= 0) {
    result.addError("World dimensions must be positive", "world");
  }

  if (scene.world.width < 800 || scene.world.height < 600) {
    result.addWarning("World is smaller than default screen size (800x600)",
                      "world");
  }

  if (scene.world.width > 10000 || scene.world.height > 10000) {
    result.addWarning("Very large world size may impact performance", "world");
  }

  if (scene.camera.followSpeed <= 0) {
    result.addError("Camera follow speed must be positive",
                    "camera.followSpeed");
  }

  if (scene.camera.followSpeed > 50) {
    result.addWarning("Very high camera follow speed may cause motion sickness",
                      "camera.followSpeed");
  }
}

void SceneValidator::validatePlayerSpawn(
    const SceneData::SceneDefinition &scene, SceneValidationResult &result) {
  const auto &spawn = scene.playerSpawn;

  // Check if player spawns outside world bounds
  if (spawn.x < 0 || spawn.y < 0 || spawn.x > scene.world.width ||
      spawn.y > scene.world.height) {
    result.addError("Player spawns outside world bounds", "playerSpawn");
  }

  // Check if player spawns inside an obstacle
  for (size_t i = 0; i < scene.obstacles.size(); i++) {
    if (isPointInObstacle(glm::vec2(spawn.x + 25, spawn.y + 25),
                          scene.obstacles[i])) {
      result.addError("Player spawns inside obstacle " + std::to_string(i),
                      "playerSpawn");
    }
  }
}

void SceneValidator::validateObstacles(const SceneData::SceneDefinition &scene,
                                       SceneValidationResult &result) {
  for (size_t i = 0; i < scene.obstacles.size(); i++) {
    const auto &obstacle = scene.obstacles[i];
    std::string location = "obstacle[" + std::to_string(i) + "]";

    if (obstacle.width <= 0 || obstacle.height <= 0) {
      result.addError("Obstacle dimensions must be positive", location);
    }

    if (obstacle.x < 0 || obstacle.y < 0 ||
        obstacle.x + obstacle.width > scene.world.width ||
        obstacle.y + obstacle.height > scene.world.height) {
      result.addWarning("Obstacle extends outside world bounds", location);
    }

    if (obstacle.width < 10 || obstacle.height < 10) {
      result.addWarning("Very small obstacle may be hard to see", location);
    }

    if (obstacle.width > scene.world.width * 0.5f ||
        obstacle.height > scene.world.height * 0.5f) {
      result.addWarning("Very large obstacle may block too much of the world",
                        location);
    }
  }
}

void SceneValidator::validateCollectibles(
    const SceneData::SceneDefinition &scene, SceneValidationResult &result) {
  if (scene.collectibles.empty() &&
      scene.transitionTrigger == "collectibles_complete") {
    result.addError(
        "Scene completion requires collectibles but none are defined",
        "collectibles");
  }

  for (size_t i = 0; i < scene.collectibles.size(); i++) {
    const auto &collectible = scene.collectibles[i];
    std::string location = "collectible[" + std::to_string(i) + "]";

    if (collectible.x < 0 || collectible.y < 0 ||
        collectible.x > scene.world.width ||
        collectible.y > scene.world.height) {
      result.addWarning("Collectible is outside world bounds", location);
    }

    // Check if collectible is inside an obstacle
    for (size_t j = 0; j < scene.obstacles.size(); j++) {
      if (isPointInObstacle(glm::vec2(collectible.x + 15, collectible.y + 15),
                            scene.obstacles[j])) {
        result.addError("Collectible is inside obstacle " + std::to_string(j),
                        location);
      }
    }
  }

  if (scene.collectibles.size() > 50) {
    result.addWarning("Large number of collectibles may impact performance",
                      "collectibles");
  }
}

void SceneValidator::validateEnemies(const SceneData::SceneDefinition &scene,
                                     SceneValidationResult &result) {
  if (scene.enemies.empty() && scene.transitionTrigger == "enemies_defeat") {
    result.addError(
        "Scene completion requires defeating enemies but none are defined",
        "enemies");
  }

  for (size_t i = 0; i < scene.enemies.size(); i++) {
    const auto &enemy = scene.enemies[i];
    std::string location = "enemy[" + std::to_string(i) + "]";

    if (enemy.x < 0 || enemy.y < 0 || enemy.x > scene.world.width ||
        enemy.y > scene.world.height) {
      result.addWarning("Enemy spawns outside world bounds", location);
    }

    if (enemy.speed <= 0) {
      result.addError("Enemy speed must be positive", location);
    }

    if (enemy.speed > 1000) {
      result.addWarning("Very high enemy speed may make game unplayable",
                        location);
    }

    // Check if enemy spawns inside an obstacle
    for (size_t j = 0; j < scene.obstacles.size(); j++) {
      if (isPointInObstacle(glm::vec2(enemy.x + 25, enemy.y + 25),
                            scene.obstacles[j])) {
        result.addWarning("Enemy spawns inside obstacle " + std::to_string(j),
                          location);
      }
    }

    // Validate movement pattern specific settings
    if (enemy.pattern == MovementPattern::CIRCULAR && enemy.radius <= 0) {
      result.addError("Circular movement pattern requires positive radius",
                      location);
    }

    if (enemy.pattern == MovementPattern::PATROL) {
      if (enemy.patrolPoint1.x == enemy.patrolPoint2.x &&
          enemy.patrolPoint1.y == enemy.patrolPoint2.y) {
        result.addWarning("Patrol points are identical - enemy won't move",
                          location);
      }
    }
  }

  if (scene.enemies.size() > 20) {
    result.addWarning("Large number of enemies may impact performance",
                      "enemies");
  }
}

void SceneValidator::validateObjectOverlaps(
    const SceneData::SceneDefinition &scene, SceneValidationResult &result) {
  // Check for overlapping obstacles
  for (size_t i = 0; i < scene.obstacles.size(); i++) {
    for (size_t j = i + 1; j < scene.obstacles.size(); j++) {
      if (doObstaclesOverlap(scene.obstacles[i], scene.obstacles[j])) {
        result.addWarning("Obstacles " + std::to_string(i) + " and " +
                              std::to_string(j) + " overlap",
                          "obstacles");
      }
    }
  }

  // Check for collectibles too close to each other
  for (size_t i = 0; i < scene.collectibles.size(); i++) {
    for (size_t j = i + 1; j < scene.collectibles.size(); j++) {
      float dist = glm::distance(
          glm::vec2(scene.collectibles[i].x, scene.collectibles[i].y),
          glm::vec2(scene.collectibles[j].x, scene.collectibles[j].y));
      if (dist < 50.0f) {
        result.addWarning("Collectibles " + std::to_string(i) + " and " +
                              std::to_string(j) + " are very close",
                          "collectibles");
      }
    }
  }
}

void SceneValidator::validateReachability(
    const SceneData::SceneDefinition &scene, SceneValidationResult &result) {
  // Basic reachability check - ensure collectibles aren't completely surrounded
  // by obstacles This is a simplified check; a full pathfinding analysis would
  // be more accurate

  for (size_t i = 0; i < scene.collectibles.size(); i++) {
    if (!isCollectibleReachable(scene.collectibles[i], scene.playerSpawn,
                                scene.obstacles)) {
      result.addWarning("Collectible " + std::to_string(i) +
                            " may not be reachable",
                        "collectible[" + std::to_string(i) + "]");
    }
  }
}

bool SceneValidator::isPointInObstacle(
    const glm::vec2 &point, const SceneData::ObstacleData &obstacle) {
  return point.x >= obstacle.x && point.x <= obstacle.x + obstacle.width &&
         point.y >= obstacle.y && point.y <= obstacle.y + obstacle.height;
}

bool SceneValidator::doObstaclesOverlap(const SceneData::ObstacleData &a,
                                        const SceneData::ObstacleData &b) {
  return !(a.x + a.width < b.x || b.x + b.width < a.x || a.y + a.height < b.y ||
           b.y + b.height < a.y);
}

bool SceneValidator::isCollectibleReachable(
    const SceneData::CollectibleData &collectible,
    const SceneData::PlayerSpawn &playerSpawn,
    const std::vector<SceneData::ObstacleData> &obstacles) {
  // Simple line-of-sight check - if there's a clear line from player to
  // collectible
  glm::vec2 playerPos(playerSpawn.x, playerSpawn.y);
  glm::vec2 collectiblePos(collectible.x, collectible.y);

  // Sample points along the line
  const int samples = 20;
  for (int i = 0; i <= samples; i++) {
    float t = static_cast<float>(i) / samples;
    glm::vec2 samplePoint = glm::mix(playerPos, collectiblePos, t);

    // Check if this point is inside any obstacle
    for (const auto &obstacle : obstacles) {
      if (isPointInObstacle(samplePoint, obstacle)) {
        return false; // Path blocked
      }
    }
  }

  return true; // Clear path found
}