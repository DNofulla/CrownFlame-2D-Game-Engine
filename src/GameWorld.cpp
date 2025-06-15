#include "GameWorld.h"
#include "AudioManager.h"
#include <algorithm>
#include <gl2d/gl2d.h>
#include <iostream>

GameWorld::GameWorld()
    : player(nullptr), screenWidth(640), screenHeight(480),
      cameraPosition(0.0f, 0.0f), cameraFollowSpeed(5.0f),
      cameraFollowEnabled(true), worldWidth(2000.0f), worldHeight(1500.0f),
      pigTexture(nullptr), gameFont(nullptr), audioManager(nullptr) {}

GameWorld::~GameWorld() {
  gameObjects.clear();
  enemies.clear();

  // Clean up pig texture
  if (pigTexture) {
    static_cast<gl2d::Texture *>(pigTexture)->cleanup();
    delete static_cast<gl2d::Texture *>(pigTexture);
  }

  // Clean up font
  if (gameFont) {
    static_cast<gl2d::Font *>(gameFont)->cleanup();
    delete static_cast<gl2d::Font *>(gameFont);
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
        ->loadFromFile(RESOURCES_PATH "pig.png");
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
    // Update non-static objects if needed
    for (auto &obj : gameObjects) {
      if (!obj->isStatic && obj.get() != player) {
        // Only constrain non-player objects to screen bounds
        obj->constrainToBounds(screenWidth, screenHeight);
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
  // Create a default grass map
  if (!tileMapManager.createDefaultGrassMap("main_world", 100, 75)) {
    std::cerr << "Failed to create default grass map" << std::endl;
    return false;
  }

  std::cout << "Tile system initialized successfully" << std::endl;
  return true;
}