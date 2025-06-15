#pragma once
#include "Enemy.h"
#include "GameObject.h"
#include "GameState.h"
#include "TileMapManager.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

// Forward declaration
class AudioManager;

class GameWorld {
private:
  std::vector<std::unique_ptr<GameObject>> gameObjects;
  std::vector<std::unique_ptr<Enemy>> enemies;
  GameObject *player;
  int screenWidth, screenHeight;

  // Camera system
  glm::vec2 cameraPosition;
  float cameraFollowSpeed;
  bool cameraFollowEnabled;

  // World bounds (larger than screen)
  float worldWidth, worldHeight;

  // Game state
  GameStateManager gameStateManager;

  // Texture for pig enemies (will be loaded in implementation)
  void *pigTexture; // Using void* to avoid gl2d dependency in header

  // Font for game over banner
  void *gameFont; // Using void* to avoid gl2d dependency in header

  // Audio system
  AudioManager *audioManager;

  // Tile system
  TileMapManager tileMapManager;

public:
  GameWorld();
  ~GameWorld();

  // Initialize the game world
  void initialize(int width, int height);

  // Add objects to the world
  void addObject(std::unique_ptr<GameObject> object);
  GameObject *createPlayer(float x, float y);
  GameObject *createObstacle(float x, float y, float width, float height);
  GameObject *createCollectible(float x, float y);

  // Enemy management
  Enemy *createEnemy(float x, float y, MovementPattern pattern);
  void updateEnemies(float deltaTime);
  void checkPlayerEnemyCollisions();

  // Update game logic
  void update(float deltaTime);
  void updatePlayer(float moveX, float moveY, float speed, float deltaTime);

  // Collision handling
  void handleCollisions();
  bool checkPlayerCollisions(float newX, float newY);

  // Rendering
  void render(void *renderer);
  void renderGameOverBanner(void *renderer);

  // Getters
  GameObject *getPlayer() const { return player; }
  const std::vector<std::unique_ptr<GameObject>> &getObjects() const {
    return gameObjects;
  }
  const std::vector<std::unique_ptr<Enemy>> &getEnemies() const {
    return enemies;
  }
  GameStateManager &getGameStateManager() { return gameStateManager; }

  // Screen management
  void updateScreenSize(int width, int height);

  // Camera management
  void updateCamera(float deltaTime);
  glm::vec2 getCameraPosition() const { return cameraPosition; }
  void setCameraFollowSpeed(float speed) { cameraFollowSpeed = speed; }
  void enableCameraFollow(bool enable) { cameraFollowEnabled = enable; }

  // Audio management
  void setAudioManager(AudioManager *manager) { audioManager = manager; }

  // Tile map management
  TileMapManager &getTileMapManager() { return tileMapManager; }
  bool initializeTileSystem();
};