#include "GameWorld.h"
#include "AudioManager.h"
#include <algorithm>
#include <gl2d/gl2d.h>
#include <iostream>

GameWorld::GameWorld()
    : player(nullptr), screenWidth(640), screenHeight(480),
      cameraPosition(0.0f, 0.0f), cameraFollowSpeed(5.0f),
      cameraFollowEnabled(true), worldWidth(2000.0f), worldHeight(1500.0f),
      pigTexture(nullptr), gameFont(nullptr), audioManager(nullptr),
      currentPathIndex(0), followingPath(false) {}

GameWorld::~GameWorld() {
  gameObjects.clear();
  enemies.clear();

  // Clean up pig texture safely
  if (pigTexture) {
    static_cast<gl2d::Texture *>(pigTexture)->cleanup();
    delete static_cast<gl2d::Texture *>(pigTexture);
    pigTexture = nullptr;
  }

  // Clean up font safely
  if (gameFont) {
    static_cast<gl2d::Font *>(gameFont)->cleanup();
    delete static_cast<gl2d::Font *>(gameFont);
    gameFont = nullptr;
  }
}

void GameWorld::initialize(int width, int height) {
  screenWidth = width;
  screenHeight = height;

  // Clear existing objects
  gameObjects.clear();
  enemies.clear();
  player = nullptr;

  // Initialize tile system
  initializeTileSystem();

  // Load pig texture if not already loaded
  if (!pigTexture) {
    pigTexture = new gl2d::Texture();
    static_cast<gl2d::Texture *>(pigTexture)
        ->loadFromFile(RESOURCES_PATH "textures/sprites/pig.png");
  }

  // Load font if not already loaded
  if (!gameFont) {
    gameFont = new gl2d::Font();
    // Try to load a system font, fallback to a simple approach if not available
    try {
      static_cast<gl2d::Font *>(gameFont)->createFromFile(
          "C:/Windows/Fonts/arial.ttf");
    } catch (...) {
      // If no system font available, we'll render text differently
      delete static_cast<gl2d::Font *>(gameFont);
      gameFont = nullptr;
    }
  }

  // Create player
  player = createPlayer(100.0f, 100.0f);

  // Create obstacles spread across a larger world
  createObstacle(300.0f, 200.0f, 80.0f, 80.0f);
  createObstacle(500.0f, 300.0f, 60.0f, 120.0f);
  createObstacle(200.0f, 350.0f, 100.0f, 50.0f);
  createObstacle(800.0f, 150.0f, 70.0f, 90.0f);
  createObstacle(1000.0f, 400.0f, 120.0f, 60.0f);
  createObstacle(600.0f, 600.0f, 80.0f, 80.0f);
  createObstacle(1200.0f, 250.0f, 100.0f, 150.0f);
  createObstacle(400.0f, 800.0f, 90.0f, 70.0f);
  createObstacle(1400.0f, 500.0f, 110.0f, 80.0f);

  // Create collectibles spread across the world
  createCollectible(450.0f, 150.0f);
  createCollectible(150.0f, 250.0f);
  createCollectible(400.0f, 400.0f);
  createCollectible(700.0f, 300.0f);
  createCollectible(900.0f, 200.0f);
  createCollectible(1100.0f, 350.0f);
  createCollectible(650.0f, 550.0f);
  createCollectible(1300.0f, 600.0f);
  createCollectible(350.0f, 750.0f);
  createCollectible(1500.0f, 400.0f);
  createCollectible(800.0f, 700.0f);
  createCollectible(1200.0f, 100.0f);

  // Create enemies with different movement patterns
  createEnemy(400.0f, 300.0f, MovementPattern::HORIZONTAL);
  createEnemy(700.0f, 450.0f, MovementPattern::VERTICAL);
  createEnemy(900.0f, 300.0f, MovementPattern::CIRCULAR);
  createEnemy(1200.0f, 400.0f, MovementPattern::PATROL);
  createEnemy(600.0f, 200.0f, MovementPattern::HORIZONTAL);
  createEnemy(1000.0f, 600.0f, MovementPattern::VERTICAL);
  createEnemy(500.0f, 700.0f, MovementPattern::CIRCULAR);

  // Reset game state
  gameStateManager.resetGame();

  // Initialize pathfinding
  currentPath.clear();
  currentPathIndex = 0;
  followingPath = false;
}

void GameWorld::initializeEmpty(int width, int height) {
  screenWidth = width;
  screenHeight = height;

  // Clear existing objects
  gameObjects.clear();
  enemies.clear();
  player = nullptr;

  // Initialize tile system
  initializeTileSystem();

  // Load pig texture if not already loaded
  if (!pigTexture) {
    pigTexture = new gl2d::Texture();
    static_cast<gl2d::Texture *>(pigTexture)
        ->loadFromFile(RESOURCES_PATH "textures/sprites/pig.png");
  }

  // Load font if not already loaded
  if (!gameFont) {
    gameFont = new gl2d::Font();
    // Try to load a system font, fallback to a simple approach if not available
    try {
      static_cast<gl2d::Font *>(gameFont)->createFromFile(
          "C:/Windows/Fonts/arial.ttf");
    } catch (...) {
      // If no system font available, we'll render text differently
      delete static_cast<gl2d::Font *>(gameFont);
      gameFont = nullptr;
    }
  }

  // Reset game state
  gameStateManager.resetGame();

  // Initialize pathfinding
  currentPath.clear();
  currentPathIndex = 0;
  followingPath = false;

  // DO NOT create any game objects - the Scene system will handle that
}

void GameWorld::addObject(std::unique_ptr<GameObject> object) {
  gameObjects.push_back(std::move(object));
}

GameObject *GameWorld::createPlayer(float x, float y) {
  auto playerObj = std::make_unique<GameObject>(
      x, y, 50.0f, 50.0f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), ObjectType::PLAYER,
      false);
  GameObject *playerPtr = playerObj.get();
  addObject(std::move(playerObj));

  // Set the internal player reference so updatePlayer works
  player = playerPtr;

  return playerPtr;
}

GameObject *GameWorld::createObstacle(float x, float y, float width,
                                      float height) {
  auto obstacle = std::make_unique<GameObject>(
      x, y, width, height, glm::vec4(0.8f, 0.2f, 0.2f, 1.0f),
      ObjectType::OBSTACLE, true);
  GameObject *obstaclePtr = obstacle.get();
  addObject(std::move(obstacle));
  return obstaclePtr;
}

GameObject *GameWorld::createCollectible(float x, float y) {
  auto collectible = std::make_unique<GameObject>(
      x, y, 30.0f, 30.0f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
      ObjectType::COLLECTIBLE, true);
  GameObject *collectiblePtr = collectible.get();
  addObject(std::move(collectible));
  return collectiblePtr;
}

Enemy *GameWorld::createEnemy(float x, float y, MovementPattern pattern) {
  auto enemy = std::make_unique<Enemy>(x, y, pattern);
  enemy->type = ObjectType::ENEMY;
  Enemy *enemyPtr = enemy.get();
  enemies.push_back(std::move(enemy));
  return enemyPtr;
}

void GameWorld::updateEnemies(float deltaTime) {
  for (auto &enemy : enemies) {
    enemy->update(deltaTime);

    // Keep enemies within world bounds
    enemy->bounds.x = std::max(
        0.0f, std::min(enemy->bounds.x, worldWidth - enemy->bounds.width));
    enemy->bounds.y = std::max(
        0.0f, std::min(enemy->bounds.y, worldHeight - enemy->bounds.height));
  }
}

void GameWorld::checkPlayerEnemyCollisions() {
  if (!player || gameStateManager.isGameOver())
    return;

  for (const auto &enemy : enemies) {
    if (player->isColliding(*enemy)) {
      gameStateManager.triggerGameOver();
      std::cout << "Game Over! You touched a pig!" << std::endl;
      break;
    }
  }
}

void GameWorld::update(float deltaTime) {
  // Update game state
  gameStateManager.update(deltaTime);

  // Only update game objects if playing
  if (gameStateManager.isPlaying()) {
    // Cache screen bounds for better performance
    const float screenW = static_cast<float>(screenWidth);
    const float screenH = static_cast<float>(screenHeight);

    // Update non-static objects if needed
    for (auto &obj : gameObjects) {
      if (!obj->isStatic && obj.get() != player) {
        // Only constrain non-player objects to screen bounds
        obj->constrainToBounds(screenW, screenH);
      }
    }

    // Constrain player to larger world bounds
    if (player) {
      player->bounds.x = std::max(
          0.0f, std::min(player->bounds.x, worldWidth - player->bounds.width));
      player->bounds.y =
          std::max(0.0f, std::min(player->bounds.y,
                                  worldHeight - player->bounds.height));
    }

    // Update enemies
    updateEnemies(deltaTime);

    // Update pathfinding (will be called from Application with player speed)
    // updatePathfinding(deltaTime, playerSpeed); // This will be called from
    // Application

    // Check collisions
    handleCollisions();
    checkPlayerEnemyCollisions();
  }
}

void GameWorld::updatePlayer(float moveX, float moveY, float speed,
                             float deltaTime) {
  if (!player)
    return;

  // Calculate movement
  float deltaX = moveX * speed * deltaTime;
  float deltaY = moveY * speed * deltaTime;

  // Store current position
  float currentX = player->bounds.x;
  float currentY = player->bounds.y;

  // Try moving horizontally first
  player->setPosition(currentX + deltaX, currentY);
  if (checkPlayerCollisions(player->bounds.x, player->bounds.y)) {
    player->setPosition(currentX, currentY); // Revert horizontal movement
  }

  // Try moving vertically
  player->setPosition(player->bounds.x, currentY + deltaY);
  if (checkPlayerCollisions(player->bounds.x, player->bounds.y)) {
    player->setPosition(player->bounds.x, currentY); // Revert vertical movement
  }

  // With camera following, player doesn't need to be constrained to screen
  // bounds player->constrainToBounds(screenWidth, screenHeight);
}

void GameWorld::handleCollisions() {
  if (!player)
    return;

  // Check collisions with collectibles
  for (auto it = gameObjects.begin(); it != gameObjects.end();) {
    if ((*it)->type == ObjectType::COLLECTIBLE && player->isColliding(**it)) {
      std::cout << "Collected item!" << std::endl;

      // Play collectible pickup sound
      if (audioManager) {
        audioManager->playSoundMulti("collectible_pickup");
      }

      it = gameObjects.erase(it);
      // Update player pointer if it was the erased object
      if (it != gameObjects.end() && (*it).get() == player) {
        player = nullptr;
      }
    } else {
      ++it;
    }
  }
}

bool GameWorld::checkPlayerCollisions(float newX, float newY) {
  if (!player)
    return false;

  Rectangle testRect(newX, newY, player->bounds.width, player->bounds.height);

  for (const auto &obj : gameObjects) {
    if (obj.get() != player && obj->type == ObjectType::OBSTACLE) {
      if (CollisionDetection::checkRectangleCollision(testRect, obj->bounds)) {
        return true; // Collision detected
      }
    }
  }

  return false; // No collision
}

void GameWorld::render(void *rendererPtr) {
  gl2d::Renderer2D &renderer = *static_cast<gl2d::Renderer2D *>(rendererPtr);

  // Render tile map first (background)
  tileMapManager.renderCurrentMap(rendererPtr, cameraPosition,
                                  glm::vec2(screenWidth, screenHeight));

  // Render regular game objects
  for (const auto &obj : gameObjects) {
    gl2d::Color4f color = {obj->color.r, obj->color.g, obj->color.b,
                           obj->color.a};
    renderer.renderRectangle(
        {obj->bounds.x, obj->bounds.y, obj->bounds.width, obj->bounds.height},
        color);
  }

  // Render enemies with pig texture
  if (pigTexture) {
    gl2d::Texture *texture = static_cast<gl2d::Texture *>(pigTexture);
    for (const auto &enemy : enemies) {
      renderer.renderRectangle({enemy->bounds.x, enemy->bounds.y,
                                enemy->bounds.width, enemy->bounds.height},
                               *texture);
    }
  }

  // Render pathfinding path
  renderPath(rendererPtr);

  // Render game over banner on top of everything
  renderGameOverBanner(rendererPtr);
}

void GameWorld::updateScreenSize(int width, int height) {
  screenWidth = width;
  screenHeight = height;
}

void GameWorld::renderGameOverBanner(void *rendererPtr) {
  if (!gameStateManager.isGameOver())
    return;

  gl2d::Renderer2D &renderer = *static_cast<gl2d::Renderer2D *>(rendererPtr);

  // Get camera position to render overlay in screen space
  glm::vec2 camPos = cameraPosition;

  // Calculate screen-space coordinates
  float overlayX = camPos.x;
  float overlayY = camPos.y;
  float overlayWidth = static_cast<float>(screenWidth);
  float overlayHeight = static_cast<float>(screenHeight);

  // Render semi-transparent dark overlay
  gl2d::Color4f overlayColor = {0.0f, 0.0f, 0.0f,
                                0.7f}; // Dark with transparency
  renderer.renderRectangle({overlayX, overlayY, overlayWidth, overlayHeight},
                           overlayColor);

  // Render "GAME OVER" text
  if (gameFont) {
    gl2d::Font *font = static_cast<gl2d::Font *>(gameFont);
    gl2d::Color4f textColor = {1.0f, 0.0f, 0.0f, 1.0f}; // Red text

    // Center the text on screen
    float centerX = camPos.x + screenWidth / 2.0f;
    float centerY = camPos.y + screenHeight / 2.0f;

    // Render main "GAME OVER" text (much smaller size, increased character
    // spacing)
    renderer.renderText({centerX, centerY - 80}, "GAME OVER", *font, textColor,
                        1.8f, 8, 2, true);

    // Render restart instruction (smaller size, increased character spacing)
    gl2d::Color4f instructionColor = {1.0f, 1.0f, 1.0f, 1.0f}; // White text
    renderer.renderText({centerX, centerY + 20}, "Press R to Restart", *font,
                        instructionColor, 1.0f, 5, 1, true);

    // Additional instruction (increased character spacing)
    renderer.renderText({centerX, centerY + 80}, "or click Restart button",
                        *font, instructionColor, 0.8f, 4, 1, true);
  } else {
    // Fallback: render colored rectangles to spell out "GAME OVER"
    gl2d::Color4f redColor = {1.0f, 0.0f, 0.0f, 1.0f};
    gl2d::Color4f whiteColor = {1.0f, 1.0f, 1.0f, 1.0f};

    float centerX = camPos.x + screenWidth / 2.0f - 80;
    float centerY = camPos.y + screenHeight / 2.0f - 80;

    // Simple "GAME OVER" message using a simple red rectangle
    renderer.renderRectangle({centerX, centerY, 160, 40}, redColor);

    // White text box outline to indicate "GAME OVER"
    renderer.renderRectangle({centerX + 5, centerY + 5, 150, 30}, whiteColor);
    renderer.renderRectangle({centerX + 10, centerY + 10, 140, 20}, redColor);

    // Simple "RESTART" instruction - much smaller, with much more spacing
    float instY = centerY + 100;
    renderer.renderRectangle(
        {camPos.x + screenWidth / 2.0f - 80, instY, 160, 15}, whiteColor);
    renderer.renderRectangle(
        {camPos.x + screenWidth / 2.0f - 75, instY + 3, 150, 9}, redColor);
  }
}

void GameWorld::updateCamera(float deltaTime) {
  if (!player || !cameraFollowEnabled)
    return;

  // Get player center position
  glm::vec2 playerCenter = player->getCenter();

  // Calculate target camera position (center player on screen)
  glm::vec2 targetPosition =
      playerCenter - glm::vec2(screenWidth / 2.0f, screenHeight / 2.0f);

  // Smoothly move camera towards target position
  glm::vec2 direction = targetPosition - cameraPosition;
  float distance = glm::length(direction);

  if (distance > 1.0f) // Only move if far enough away
  {
    glm::vec2 normalizedDirection = glm::normalize(direction);
    float moveDistance = cameraFollowSpeed * distance * deltaTime;

    // Clamp movement to not overshoot
    if (moveDistance > distance)
      moveDistance = distance;

    cameraPosition += normalizedDirection * moveDistance;
  }
}

bool GameWorld::initializeTileSystem() {
  // Check if we already have a current map (during reset)
  if (tileMapManager.getCurrentMap()) {
    // Reset the existing tilemap by regenerating its content
    tileMapManager.resetCurrentMap();
    std::cout << "Tile system reset successfully" << std::endl;
    return true;
  }

  // Calculate tile map dimensions to match playable world bounds
  // World: 2000x1500 pixels, Tiles: 64x64 pixels each
  int tilesWide = static_cast<int>(worldWidth / 64.0f);  // 2000/64 = 31 tiles
  int tilesHigh = static_cast<int>(worldHeight / 64.0f); // 1500/64 = 23 tiles

  // Update world bounds to exactly match tile map dimensions
  worldWidth = static_cast<float>(tilesWide * 64);  // 31 * 64 = 1,984 pixels
  worldHeight = static_cast<float>(tilesHigh * 64); // 23 * 64 = 1,472 pixels

  std::cout << "Creating tile map to match world bounds:" << std::endl;
  std::cout << "Updated world size: " << worldWidth << "x" << worldHeight
            << " pixels" << std::endl;
  std::cout << "Tile map: " << tilesWide << "x" << tilesHigh << " tiles"
            << std::endl;

  // Create a tile map that exactly matches the playable world size
  if (!tileMapManager.createDefaultGrassMap("main_world", tilesWide,
                                            tilesHigh)) {
    std::cerr << "Failed to create default grass map" << std::endl;
    return false;
  }

  // Verify map is fully preloaded by checking some random tiles
  TileMap *currentMap = tileMapManager.getCurrentMap();
  if (currentMap) {
    std::cout << "=== MAP PRELOAD VERIFICATION ===" << std::endl;
    std::cout << "Map size: " << currentMap->getMapWidth() << "x"
              << currentMap->getMapHeight() << " (Total: "
              << (currentMap->getMapWidth() * currentMap->getMapHeight())
              << " tiles)" << std::endl;
    std::cout << "World bounds: " << worldWidth << "x" << worldHeight
              << " pixels" << std::endl;
    std::cout
        << "Perfect alignment: Player movement exactly matches tile boundaries!"
        << std::endl;
    std::cout << "All tiles generated at startup - NO runtime generation!"
              << std::endl;
    std::cout << "=================================" << std::endl;
  }

  std::cout << "Tile system initialized successfully" << std::endl;
  return true;
}

void GameWorld::verifyMapPreloaded() const {
  TileMap *currentMap = tileMapManager.getCurrentMap();
  if (!currentMap) {
    std::cout << "No current map loaded!" << std::endl;
    return;
  }

  int width = currentMap->getMapWidth();
  int height = currentMap->getMapHeight();
  int totalTiles = width * height;

  std::cout << "\n=== FULL MAP PRELOAD VERIFICATION ===" << std::endl;
  std::cout << "Map dimensions: " << width << "x" << height << " = "
            << totalTiles << " total tiles" << std::endl;

  // Check every single tile to prove they all exist
  int tilesChecked = 0;
  int validTiles = 0;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int tileId = currentMap->getTile(x, y);
      tilesChecked++;
      if (tileId >= 0) { // Valid tile ID
        validTiles++;
      }
    }
  }

  std::cout << "Tiles checked: " << tilesChecked << "/" << totalTiles
            << std::endl;
  std::cout << "Valid tiles found: " << validTiles << std::endl;
  std::cout << "Verification: "
            << (validTiles == totalTiles ? "PASS - ALL TILES PRELOADED!"
                                         : "FAIL - Missing tiles!")
            << std::endl;
  std::cout << "====================================\n" << std::endl;
}

void GameWorld::handleMouseInput(const glm::vec2 &mouseScreenPos) {
  if (!player || gameStateManager.isGameOver()) {
    return;
  }

  // Convert screen position to world position
  glm::vec2 targetWorldPos = screenToWorldPosition(mouseScreenPos);

  // Get player center position
  glm::vec2 playerCenter = player->getCenter();

  // Find path from player to target
  std::vector<glm::vec2> newPath =
      Pathfinder::findPath(playerCenter, targetWorldPos, gameObjects,
                           player->bounds.width, player->bounds.height);

  if (!newPath.empty()) {
    currentPath = newPath;
    currentPathIndex = 0;
    followingPath = true;

    std::cout << "Path found with " << currentPath.size() << " waypoints"
              << std::endl;
  } else {
    std::cout << "No path found to target position" << std::endl;
    followingPath = false;
  }
}

void GameWorld::updatePathfinding(float deltaTime, float playerSpeed) {
  if (!followingPath || !player || currentPath.empty()) {
    return;
  }

  // Always move towards the final destination
  glm::vec2 destination = currentPath.back();
  glm::vec2 playerCenter = player->getCenter();

  // Calculate direction to destination
  glm::vec2 direction = destination - playerCenter;
  float distance = glm::length(direction);

  // Check if we've reached the destination
  if (distance < 15.0f) {
    followingPath = false;
    currentPath.clear();
    return;
  }

  // Move towards the destination
  glm::vec2 normalizedDirection = glm::normalize(direction);
  float moveDistance = playerSpeed * deltaTime;

  // Calculate new position (center the player on the movement)
  glm::vec2 newPlayerPos = playerCenter + normalizedDirection * moveDistance;
  newPlayerPos.x -= player->bounds.width / 2.0f;
  newPlayerPos.y -= player->bounds.height / 2.0f;

  // Check for collisions and move player
  if (!checkPlayerCollisions(newPlayerPos.x, newPlayerPos.y)) {
    player->setPosition(newPlayerPos.x, newPlayerPos.y);
  } else {
    // If collision detected, try to follow the waypoints in the path
    if (currentPath.size() > 1 && currentPathIndex < currentPath.size() - 1) {
      // Move to next waypoint in the path
      currentPathIndex++;
      glm::vec2 nextWaypoint = currentPath[currentPathIndex];
      glm::vec2 waypointDirection = nextWaypoint - playerCenter;
      float waypointDistance = glm::length(waypointDirection);

      if (waypointDistance > 1.0f) {
        glm::vec2 normalizedWaypointDir = glm::normalize(waypointDirection);
        glm::vec2 waypointPos =
            playerCenter + normalizedWaypointDir * moveDistance;
        waypointPos.x -= player->bounds.width / 2.0f;
        waypointPos.y -= player->bounds.height / 2.0f;

        if (!checkPlayerCollisions(waypointPos.x, waypointPos.y)) {
          player->setPosition(waypointPos.x, waypointPos.y);
          return;
        }
      }
    }

    // If still blocked, recalculate path
    std::vector<glm::vec2> newPath =
        Pathfinder::findPath(playerCenter, destination, gameObjects,
                             player->bounds.width, player->bounds.height);

    if (!newPath.empty()) {
      currentPath = newPath;
      currentPathIndex = 0;
    } else {
      followingPath = false;
      currentPath.clear();
    }
  }
}

void GameWorld::stopPathfinding() {
  followingPath = false;
  currentPath.clear();
  currentPathIndex = 0;
}

glm::vec2 GameWorld::screenToWorldPosition(const glm::vec2 &screenPos) const {
  return glm::vec2(screenPos.x + cameraPosition.x,
                   screenPos.y + cameraPosition.y);
}

void GameWorld::renderPath(void *rendererPtr) {
  if (!followingPath || currentPath.empty()) {
    return;
  }

  gl2d::Renderer2D &renderer = *static_cast<gl2d::Renderer2D *>(rendererPtr);
  gl2d::Color4f destinationColor = {1.0f, 1.0f, 0.0f,
                                    1.0f}; // Yellow destination

  // Only render the final destination waypoint
  if (!currentPath.empty()) {
    glm::vec2 destination = currentPath.back();
    float destinationSize = 12.0f;

    // Render destination as a larger yellow square
    renderer.renderRectangle({destination.x - destinationSize / 2,
                              destination.y - destinationSize / 2,
                              destinationSize, destinationSize},
                             destinationColor);

    // Add a smaller inner square for visual contrast
    gl2d::Color4f innerColor = {1.0f, 0.5f, 0.0f, 1.0f}; // Orange inner
    float innerSize = 6.0f;
    renderer.renderRectangle({destination.x - innerSize / 2,
                              destination.y - innerSize / 2, innerSize,
                              innerSize},
                             innerColor);
  }
}