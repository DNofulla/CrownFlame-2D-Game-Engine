#pragma once
#include "GameWorld.h"
#include "SceneData.h"
#include <memory>
#include <string>

// Forward declarations
class AudioManager;

class Scene {
private:
  SceneData::SceneDefinition sceneDefinition;
  bool isLoaded;
  bool isActive;
  std::unique_ptr<GameWorld> gameWorld;

  // Scene completion tracking
  int initialCollectibleCount;
  int initialEnemyCount;
  bool completionTriggered;

public:
  Scene();
  explicit Scene(const SceneData::SceneDefinition &definition);
  ~Scene();

  // Scene lifecycle
  bool loadScene(int screenWidth, int screenHeight);
  void unloadScene();
  void activateScene();
  void deactivateScene();

  // Scene operations
  void update(float deltaTime);
  void render(void *renderer);

  // Scene queries
  bool isSceneLoaded() const { return isLoaded; }
  bool isSceneActive() const { return isActive; }
  bool isSceneComplete() const;

  // Scene data access
  const SceneData::SceneDefinition &getDefinition() const {
    return sceneDefinition;
  }
  SceneData::SceneDefinition &getDefinition() { return sceneDefinition; }
  void setDefinition(const SceneData::SceneDefinition &definition);

  // GameWorld access (for Application/input handling)
  GameWorld *getGameWorld() { return gameWorld.get(); }
  const GameWorld *getGameWorld() const { return gameWorld.get(); }

  // Scene name and metadata
  const std::string &getName() const { return sceneDefinition.name; }
  const std::string &getDescription() const {
    return sceneDefinition.description;
  }
  const std::string &getNextScene() const { return sceneDefinition.nextScene; }

  // Scene information for UI display
  int getObjectCount() const;
  int getObstacleCount() const {
    return static_cast<int>(sceneDefinition.obstacles.size());
  }
  int getCollectibleCount() const {
    return static_cast<int>(sceneDefinition.collectibles.size());
  }
  int getEnemyCount() const {
    return static_cast<int>(sceneDefinition.enemies.size());
  }
  float getWorldWidth() const { return sceneDefinition.world.width; }
  float getWorldHeight() const { return sceneDefinition.world.height; }
  const std::string &getTransitionTrigger() const {
    return sceneDefinition.transitionTrigger;
  }

  // Audio management
  void setAudioManager(AudioManager *audioManager);

  // Screen size management
  void updateScreenSize(int width, int height);

private:
  // Internal helper methods
  void applySceneSettings();
  void createSceneObjects();
  void setupCamera();
  void setupTilemap();
  bool checkCompletionConditions();

  // Object creation helpers
  void createObstacles();
  void createCollectibles();
  void createEnemies();
  void createPlayer();
};