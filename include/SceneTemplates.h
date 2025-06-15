#pragma once
#include "SceneData.h"
#include <map>
#include <string>
#include <vector>

class SceneTemplates {
public:
  enum class TemplateType {
    EMPTY,
    TUTORIAL,
    MAZE,
    ARENA,
    PLATFORMER,
    COLLECTION_CHALLENGE,
    ENEMY_GAUNTLET,
    OBSTACLE_COURSE
  };

  struct TemplateInfo {
    std::string name;
    std::string description;
    TemplateType type;

    TemplateInfo(const std::string &n, const std::string &desc, TemplateType t)
        : name(n), description(desc), type(t) {}
  };

  // Get list of available templates
  static std::vector<TemplateInfo> getAvailableTemplates();

  // Create scene from template
  static SceneData::SceneDefinition
  createFromTemplate(TemplateType type, const std::string &sceneName = "");

  // Template-specific creators
  static SceneData::SceneDefinition createEmpty(const std::string &name);
  static SceneData::SceneDefinition createTutorial(const std::string &name);
  static SceneData::SceneDefinition createMaze(const std::string &name);
  static SceneData::SceneDefinition createArena(const std::string &name);
  static SceneData::SceneDefinition createPlatformer(const std::string &name);
  static SceneData::SceneDefinition
  createCollectionChallenge(const std::string &name);
  static SceneData::SceneDefinition
  createEnemyGauntlet(const std::string &name);
  static SceneData::SceneDefinition
  createObstacleCourse(const std::string &name);

private:
  // Helper functions for creating specific elements
  static std::vector<SceneData::ObstacleData>
  createBorderWalls(float worldWidth, float worldHeight,
                    float thickness = 20.0f);
  static std::vector<SceneData::ObstacleData>
  createMazeWalls(float worldWidth, float worldHeight);
  static std::vector<SceneData::ObstacleData>
  createArenaObstacles(float worldWidth, float worldHeight);
  static std::vector<SceneData::ObstacleData>
  createPlatformerObstacles(float worldWidth, float worldHeight);

  static std::vector<SceneData::CollectibleData>
  createGridCollectibles(float worldWidth, float worldHeight, int count);
  static std::vector<SceneData::CollectibleData>
  createRandomCollectibles(float worldWidth, float worldHeight, int count);
  static std::vector<SceneData::CollectibleData>
  createPathCollectibles(float worldWidth, float worldHeight);

  static std::vector<SceneData::EnemyData>
  createBasicEnemies(float worldWidth, float worldHeight, int count);
  static std::vector<SceneData::EnemyData>
  createGauntletEnemies(float worldWidth, float worldHeight);
  static std::vector<SceneData::EnemyData>
  createArenaEnemies(float worldWidth, float worldHeight);

  // Utility functions
  static glm::vec4 getRandomColor();
  static float randomFloat(float min, float max);
  static bool isPointFree(const glm::vec2 &point,
                          const std::vector<SceneData::ObstacleData> &obstacles,
                          float radius = 30.0f);
};