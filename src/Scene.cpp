#include "Scene.h"
#include "AudioManager.h"
#include <iostream>

Scene::Scene()
    : isLoaded(false), isActive(false), gameWorld(nullptr),
      initialCollectibleCount(0), initialEnemyCount(0),
      completionTriggered(false) {}

Scene::Scene(const SceneData::SceneDefinition &definition)
    : sceneDefinition(definition), isLoaded(false), isActive(false),
      gameWorld(nullptr), initialCollectibleCount(0), initialEnemyCount(0),
      completionTriggered(false) {}

Scene::~Scene() { unloadScene(); }

bool Scene::loadScene(int screenWidth, int screenHeight) {
  if (isLoaded) {
    std::cout << "Scene '" << sceneDefinition.name << "' is already loaded."
              << std::endl;
    return true;
  }

  std::cout << "Loading scene: " << sceneDefinition.name << std::endl;

  // Create a new GameWorld instance
  gameWorld = std::make_unique<GameWorld>();

  // Initialize the GameWorld with basic settings (no default objects)
  gameWorld->initializeEmpty(screenWidth, screenHeight);

  // Apply scene-specific settings
  applySceneSettings();

  // Create all scene objects
  createSceneObjects();

  // Setup camera
  setupCamera();

  // Setup tilemap if enabled
  if (sceneDefinition.tilemap.enabled) {
    setupTilemap();
  }

  // Track initial counts for completion conditions
  initialCollectibleCount =
      static_cast<int>(sceneDefinition.collectibles.size());
  initialEnemyCount = static_cast<int>(sceneDefinition.enemies.size());
  completionTriggered = false;

  isLoaded = true;
  std::cout << "Scene '" << sceneDefinition.name << "' loaded successfully."
            << std::endl;
  return true;
}

void Scene::unloadScene() {
  if (!isLoaded)
    return;

  std::cout << "Unloading scene: " << sceneDefinition.name << std::endl;

  isActive = false;
  gameWorld.reset();
  isLoaded = false;
}

void Scene::activateScene() {
  if (!isLoaded) {
    std::cerr << "Cannot activate scene '" << sceneDefinition.name
              << "' - not loaded!" << std::endl;
    return;
  }

  isActive = true;
  std::cout << "Activated scene: " << sceneDefinition.name << std::endl;
}

void Scene::deactivateScene() {
  isActive = false;
  std::cout << "Deactivated scene: " << sceneDefinition.name << std::endl;
}

void Scene::update(float deltaTime) {
  if (!isLoaded || !isActive || !gameWorld)
    return;

  gameWorld->update(deltaTime);

  // Check for scene completion
  if (!completionTriggered && isSceneComplete()) {
    completionTriggered = true;
    std::cout << "Scene '" << sceneDefinition.name << "' completed!"
              << std::endl;
  }
}

void Scene::render(void *renderer) {
  if (!isLoaded || !isActive || !gameWorld)
    return;

  gameWorld->render(renderer);
}

bool Scene::isSceneComplete() const {
  if (!isLoaded || !gameWorld)
    return false;

  const std::string &trigger = sceneDefinition.transitionTrigger;

  if (trigger == "collectibles_complete") {
    // Check if all collectibles have been collected
    const auto &objects = gameWorld->getObjects();
    int currentCollectibles = 0;
    for (const auto &obj : objects) {
      if (obj->type == ObjectType::COLLECTIBLE) {
        currentCollectibles++;
      }
    }
    return currentCollectibles == 0;
  } else if (trigger == "enemies_defeat") {
    // Check if all enemies have been defeated
    return gameWorld->getEnemies().empty();
  } else if (trigger == "manual") {
    // Manual completion (e.g., reaching a specific area or trigger)
    return false; // Would need additional logic for manual triggers
  }

  return false;
}

void Scene::setDefinition(const SceneData::SceneDefinition &definition) {
  sceneDefinition = definition;

  // If already loaded, need to reload
  if (isLoaded) {
    // Use default screen dimensions since GameWorld doesn't expose getters
    int currentWidth = 800;
    int currentHeight = 600;
    unloadScene();
    loadScene(currentWidth, currentHeight);
  }
}

void Scene::setAudioManager(AudioManager *audioManager) {
  if (gameWorld) {
    gameWorld->setAudioManager(audioManager);
  }
}

void Scene::updateScreenSize(int width, int height) {
  if (gameWorld) {
    gameWorld->updateScreenSize(width, height);
  }
}

void Scene::applySceneSettings() {
  if (!gameWorld)
    return;

  // Apply world settings
  // Note: GameWorld doesn't currently have setters for world size,
  // but we can store these for future use or when GameWorld is extended

  // Apply camera settings
  gameWorld->setCameraFollowSpeed(sceneDefinition.camera.followSpeed);
  gameWorld->enableCameraFollow(sceneDefinition.camera.followEnabled);

  // Background music would be handled by AudioManager
  // if (!sceneDefinition.world.backgroundMusic.empty()) {
  //     // Load and play background music
  // }
}

void Scene::createSceneObjects() {
  createPlayer();
  createObstacles();
  createCollectibles();
  createEnemies();
}

void Scene::createPlayer() {
  if (!gameWorld)
    return;

  const auto &spawn = sceneDefinition.playerSpawn;
  auto *playerPtr = gameWorld->createPlayer(spawn.x, spawn.y);
  // The GameWorld should set its internal player reference
}

void Scene::createObstacles() {
  if (!gameWorld)
    return;

  for (const auto &obstacle : sceneDefinition.obstacles) {
    auto *obj = gameWorld->createObstacle(obstacle.x, obstacle.y,
                                          obstacle.width, obstacle.height);
    if (obj) {
      obj->color = obstacle.color;
    }
  }
}

void Scene::createCollectibles() {
  if (!gameWorld)
    return;

  for (const auto &collectible : sceneDefinition.collectibles) {
    auto *obj = gameWorld->createCollectible(collectible.x, collectible.y);
    if (obj) {
      obj->color = collectible.color;
    }
  }
}

void Scene::createEnemies() {
  if (!gameWorld)
    return;

  for (const auto &enemy : sceneDefinition.enemies) {
    auto *enemyObj = gameWorld->createEnemy(enemy.x, enemy.y, enemy.pattern);
    if (enemyObj) {
      // Apply additional enemy settings
      enemyObj->setMovementSpeed(enemy.speed);

      // Set patrol points for patrol enemies
      if (enemy.pattern == MovementPattern::PATROL) {
        enemyObj->setPatrolMovement(enemy.patrolPoint1, enemy.patrolPoint2);
      }

      // Set center and radius for circular enemies
      if (enemy.pattern == MovementPattern::CIRCULAR) {
        glm::vec2 center(enemy.x, enemy.y);
        enemyObj->setCircularMovement(center, enemy.radius);
      }
    }
  }
}

void Scene::setupCamera() {
  if (!gameWorld)
    return;

  // Camera is already configured in applySceneSettings()
  // Additional camera setup can be done here if needed
}

void Scene::setupTilemap() {
  if (!gameWorld)
    return;

  // Get the tile map manager and configure it
  auto &tileMapManager = gameWorld->getTileMapManager();

  if (!sceneDefinition.tilemap.tilesetName.empty()) {
    // Load the tileset if specified
    // tileMapManager.loadTileset(sceneDefinition.tilemap.tilesetName);

    // Create tilemap from data if available
    if (!sceneDefinition.tilemap.tileData.empty()) {
      // Convert 2D vector to the format expected by TileMapManager
      // This would need to be implemented based on TileMapManager's API
    }
  }
}

bool Scene::checkCompletionConditions() { return isSceneComplete(); }