#pragma once
#include "Enemy.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

// Forward declarations
enum class ObjectType;

namespace SceneData {

// Structure for player spawn data
struct PlayerSpawn {
  float x, y;
  PlayerSpawn(float x = 100.0f, float y = 100.0f) : x(x), y(y) {}
};

// Structure for obstacle data
struct ObstacleData {
  float x, y, width, height;
  glm::vec4 color;

  ObstacleData(float x = 0.0f, float y = 0.0f, float w = 50.0f, float h = 50.0f,
               glm::vec4 col = glm::vec4(0.8f, 0.2f, 0.2f, 1.0f))
      : x(x), y(y), width(w), height(h), color(col) {}
};

// Structure for collectible data
struct CollectibleData {
  float x, y;
  glm::vec4 color;

  CollectibleData(float x = 0.0f, float y = 0.0f,
                  glm::vec4 col = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f))
      : x(x), y(y), color(col) {}
};

// Structure for enemy data
struct EnemyData {
  float x, y;
  MovementPattern pattern;
  float speed;
  glm::vec2 patrolPoint1, patrolPoint2; // For patrol patterns
  float radius;                         // For circular patterns

  EnemyData(float x = 0.0f, float y = 0.0f,
            MovementPattern pat = MovementPattern::HORIZONTAL,
            float spd = 100.0f)
      : x(x), y(y), pattern(pat), speed(spd), patrolPoint1(x - 100.0f, y),
        patrolPoint2(x + 100.0f, y), radius(50.0f) {}
};

// Structure for camera settings
struct CameraSettings {
  float followSpeed;
  bool followEnabled;
  glm::vec2 startPosition;

  CameraSettings(float speed = 5.0f, bool enabled = true,
                 glm::vec2 startPos = glm::vec2(0.0f, 0.0f))
      : followSpeed(speed), followEnabled(enabled), startPosition(startPos) {}
};

// Structure for world settings
struct WorldSettings {
  float width, height;
  glm::vec4 backgroundColor;
  std::string backgroundMusic;

  WorldSettings(float w = 2000.0f, float h = 1500.0f,
                glm::vec4 bgColor = glm::vec4(0.1f, 0.1f, 0.15f, 1.0f),
                const std::string &music = "")
      : width(w), height(h), backgroundColor(bgColor), backgroundMusic(music) {}
};

// Structure for tilemap settings
struct TilemapSettings {
  std::string tilesetName;
  std::vector<std::vector<int>> tileData; // 2D array of tile IDs
  int tileWidth, tileHeight;
  bool enabled;

  TilemapSettings(const std::string &tileset = "", int tileW = 64,
                  int tileH = 64, bool en = false)
      : tilesetName(tileset), tileWidth(tileW), tileHeight(tileH), enabled(en) {
  }
};

// Main scene data structure
struct SceneDefinition {
  std::string name;
  std::string description;

  // World settings
  WorldSettings world;
  CameraSettings camera;
  TilemapSettings tilemap;

  // Game objects
  PlayerSpawn playerSpawn;
  std::vector<ObstacleData> obstacles;
  std::vector<CollectibleData> collectibles;
  std::vector<EnemyData> enemies;

  // Transition settings
  std::string nextScene;         // Scene to load on completion
  std::string transitionTrigger; // "collectibles_complete", "enemy_defeat",
                                 // "manual", etc.

  SceneDefinition(const std::string &sceneName = "Untitled Scene")
      : name(sceneName), description(""), nextScene(""),
        transitionTrigger("manual") {}
};

// Scene transition types
enum class TransitionType {
  INSTANT,
  FADE_TO_BLACK,
  SLIDE_LEFT,
  SLIDE_RIGHT,
  SLIDE_UP,
  SLIDE_DOWN
};

// Scene transition data
struct SceneTransition {
  TransitionType type;
  float duration; // In seconds
  glm::vec4 fadeColor;

  SceneTransition(TransitionType t = TransitionType::FADE_TO_BLACK,
                  float dur = 1.0f,
                  glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))
      : type(t), duration(dur), fadeColor(color) {}
};

} // namespace SceneData