#include "SceneTemplates.h"
#include <algorithm>
#include <random>

std::vector<SceneTemplates::TemplateInfo>
SceneTemplates::getAvailableTemplates() {
  return {TemplateInfo("Empty Scene", "Blank scene with just a player spawn",
                       TemplateType::EMPTY),
          TemplateInfo("Tutorial",
                       "Simple scene with basic obstacles and collectibles",
                       TemplateType::TUTORIAL),
          TemplateInfo("Maze",
                       "Complex maze with walls and scattered collectibles",
                       TemplateType::MAZE),
          TemplateInfo("Arena", "Open combat area with strategic obstacles",
                       TemplateType::ARENA),
          TemplateInfo("Platformer",
                       "Platform-style layout with jumping challenges",
                       TemplateType::PLATFORMER),
          TemplateInfo("Collection Challenge",
                       "Many collectibles scattered throughout",
                       TemplateType::COLLECTION_CHALLENGE),
          TemplateInfo("Enemy Gauntlet", "Progressive enemy encounters",
                       TemplateType::ENEMY_GAUNTLET),
          TemplateInfo("Obstacle Course", "Skill-based navigation challenge",
                       TemplateType::OBSTACLE_COURSE)};
}

SceneData::SceneDefinition
SceneTemplates::createFromTemplate(TemplateType type,
                                   const std::string &sceneName) {
  std::string name = sceneName.empty() ? "New Scene" : sceneName;

  switch (type) {
  case TemplateType::EMPTY:
    return createEmpty(name);
  case TemplateType::TUTORIAL:
    return createTutorial(name);
  case TemplateType::MAZE:
    return createMaze(name);
  case TemplateType::ARENA:
    return createArena(name);
  case TemplateType::PLATFORMER:
    return createPlatformer(name);
  case TemplateType::COLLECTION_CHALLENGE:
    return createCollectionChallenge(name);
  case TemplateType::ENEMY_GAUNTLET:
    return createEnemyGauntlet(name);
  case TemplateType::OBSTACLE_COURSE:
    return createObstacleCourse(name);
  default:
    return createEmpty(name);
  }
}

SceneData::SceneDefinition
SceneTemplates::createEmpty(const std::string &name) {
  SceneData::SceneDefinition scene;
  scene.name = name;
  scene.description = "A blank scene ready for customization";
  scene.transitionTrigger = "manual";
  scene.nextScene = "";

  // World settings
  scene.world.width = 1600.0f;
  scene.world.height = 1200.0f;
  scene.world.backgroundMusic = "";

  // Camera settings
  scene.camera.followEnabled = true;
  scene.camera.followSpeed = 5.0f;

  // Player spawn at center
  scene.playerSpawn.x = scene.world.width / 2;
  scene.playerSpawn.y = scene.world.height / 2;

  return scene;
}

SceneData::SceneDefinition
SceneTemplates::createTutorial(const std::string &name) {
  SceneData::SceneDefinition scene = createEmpty(name);
  scene.description = "Tutorial level with basic obstacles and collectibles";
  scene.transitionTrigger = "collectibles_complete";

  // Add border walls
  scene.obstacles = createBorderWalls(scene.world.width, scene.world.height);

  // Add a few tutorial obstacles
  scene.obstacles.emplace_back(400, 300, 200, 50, getRandomColor());
  scene.obstacles.emplace_back(1000, 600, 50, 200, getRandomColor());
  scene.obstacles.emplace_back(600, 800, 150, 100, getRandomColor());

  // Add some collectibles
  scene.collectibles.emplace_back(300, 200, getRandomColor());
  scene.collectibles.emplace_back(1200, 300, getRandomColor());
  scene.collectibles.emplace_back(800, 700, getRandomColor());
  scene.collectibles.emplace_back(500, 1000, getRandomColor());

  // One enemy to practice with
  scene.enemies.emplace_back(1000, 400, MovementPattern::PATROL, 100.0f);
  scene.enemies.back().patrolPoint1 = glm::vec2(1000, 400);
  scene.enemies.back().patrolPoint2 = glm::vec2(1200, 400);

  // Player spawn in safe area
  scene.playerSpawn.x = 150;
  scene.playerSpawn.y = 150;

  return scene;
}

SceneData::SceneDefinition SceneTemplates::createMaze(const std::string &name) {
  SceneData::SceneDefinition scene = createEmpty(name);
  scene.description = "Navigate through a complex maze";
  scene.transitionTrigger = "collectibles_complete";
  scene.world.width = 2000.0f;
  scene.world.height = 1600.0f;

  // Create maze walls
  scene.obstacles = createMazeWalls(scene.world.width, scene.world.height);

  // Add collectibles throughout the maze
  scene.collectibles =
      createRandomCollectibles(scene.world.width, scene.world.height, 8);

  // Add a few enemies in strategic locations
  scene.enemies = createBasicEnemies(scene.world.width, scene.world.height, 3);

  // Player spawn at entrance
  scene.playerSpawn.x = 100;
  scene.playerSpawn.y = 100;

  return scene;
}

SceneData::SceneDefinition
SceneTemplates::createArena(const std::string &name) {
  SceneData::SceneDefinition scene = createEmpty(name);
  scene.description = "Combat arena with strategic cover";
  scene.transitionTrigger = "enemies_defeat";
  scene.world.width = 1800.0f;
  scene.world.height = 1400.0f;

  // Border walls
  scene.obstacles = createBorderWalls(scene.world.width, scene.world.height);

  // Add arena obstacles for cover
  auto arenaObstacles =
      createArenaObstacles(scene.world.width, scene.world.height);
  scene.obstacles.insert(scene.obstacles.end(), arenaObstacles.begin(),
                         arenaObstacles.end());

  // Add challenging enemies
  scene.enemies = createArenaEnemies(scene.world.width, scene.world.height);

  // Few collectibles for power-ups
  scene.collectibles =
      createRandomCollectibles(scene.world.width, scene.world.height, 3);

  // Player spawn in center
  scene.playerSpawn.x = scene.world.width / 2;
  scene.playerSpawn.y = scene.world.height / 2;

  return scene;
}

SceneData::SceneDefinition
SceneTemplates::createPlatformer(const std::string &name) {
  SceneData::SceneDefinition scene = createEmpty(name);
  scene.description = "Platform-style challenges and jumps";
  scene.transitionTrigger = "collectibles_complete";
  scene.world.width = 2400.0f;
  scene.world.height = 1200.0f;

  // Create platformer-style obstacles
  scene.obstacles =
      createPlatformerObstacles(scene.world.width, scene.world.height);

  // Path-based collectibles
  scene.collectibles =
      createPathCollectibles(scene.world.width, scene.world.height);

  // Mobile enemies for added challenge
  scene.enemies.emplace_back(600, 800, MovementPattern::PATROL, 80.0f);
  scene.enemies.back().patrolPoint1 = glm::vec2(500, 800);
  scene.enemies.back().patrolPoint2 = glm::vec2(700, 800);
  scene.enemies.emplace_back(1200, 600, MovementPattern::CIRCULAR, 60.0f);
  scene.enemies.back().radius = 100.0f;

  // Player spawn at start
  scene.playerSpawn.x = 100;
  scene.playerSpawn.y = 1000;

  return scene;
}

SceneData::SceneDefinition
SceneTemplates::createCollectionChallenge(const std::string &name) {
  SceneData::SceneDefinition scene = createEmpty(name);
  scene.description = "Collect all items scattered throughout";
  scene.transitionTrigger = "collectibles_complete";
  scene.world.width = 2000.0f;
  scene.world.height = 1800.0f;

  // Border walls only
  scene.obstacles = createBorderWalls(scene.world.width, scene.world.height);

  // Many collectibles in grid pattern
  scene.collectibles =
      createGridCollectibles(scene.world.width, scene.world.height, 20);

  // Roaming enemies to add pressure
  scene.enemies = createBasicEnemies(scene.world.width, scene.world.height, 5);

  scene.playerSpawn.x = scene.world.width / 2;
  scene.playerSpawn.y = scene.world.height / 2;

  return scene;
}

SceneData::SceneDefinition
SceneTemplates::createEnemyGauntlet(const std::string &name) {
  SceneData::SceneDefinition scene = createEmpty(name);
  scene.description = "Defeat waves of increasingly difficult enemies";
  scene.transitionTrigger = "enemies_defeat";
  scene.world.width = 1600.0f;
  scene.world.height = 1200.0f;

  // Border walls and some cover
  scene.obstacles = createBorderWalls(scene.world.width, scene.world.height);
  scene.obstacles.push_back({400, 300, 100, 100, getRandomColor()});
  scene.obstacles.push_back({1100, 600, 100, 100, getRandomColor()});
  scene.obstacles.push_back({700, 800, 200, 50, getRandomColor()});

  // Many enemies with different patterns
  scene.enemies = createGauntletEnemies(scene.world.width, scene.world.height);

  // Some health collectibles
  scene.collectibles =
      createRandomCollectibles(scene.world.width, scene.world.height, 4);

  scene.playerSpawn.x = 200;
  scene.playerSpawn.y = 200;

  return scene;
}

SceneData::SceneDefinition
SceneTemplates::createObstacleCourse(const std::string &name) {
  SceneData::SceneDefinition scene = createEmpty(name);
  scene.description = "Navigate through challenging obstacles";
  scene.transitionTrigger = "collectibles_complete";
  scene.world.width = 2400.0f;
  scene.world.height = 1000.0f;

  // Create obstacle course layout
  scene.obstacles = createBorderWalls(scene.world.width, scene.world.height);

  // Add course obstacles
  for (int i = 0; i < 8; i++) {
    float x = 200 + i * 250;
    float y = 200 + (i % 2) * 400;
    scene.obstacles.push_back({x, y, 80, 400, getRandomColor()});
  }

  // Checkpoint collectibles
  for (int i = 0; i < 6; i++) {
    float x = 300 + i * 350;
    float y = 500;
    scene.collectibles.push_back({x, y, getRandomColor()});
  }

  // Moving obstacles (enemies)
  scene.enemies.emplace_back(800, 400, MovementPattern::PATROL, 120.0f);
  scene.enemies.back().patrolPoint1 = glm::vec2(800, 200);
  scene.enemies.back().patrolPoint2 = glm::vec2(800, 700);
  scene.enemies.emplace_back(1400, 500, MovementPattern::CIRCULAR, 80.0f);
  scene.enemies.back().radius = 150.0f;

  scene.playerSpawn.x = 100;
  scene.playerSpawn.y = 500;

  return scene;
}

// Helper implementations
std::vector<SceneData::ObstacleData>
SceneTemplates::createBorderWalls(float worldWidth, float worldHeight,
                                  float thickness) {
  std::vector<SceneData::ObstacleData> walls;
  glm::vec4 wallColor(0.5f, 0.5f, 0.5f, 1.0f);

  // Top wall
  walls.emplace_back(0, 0, worldWidth, thickness, wallColor);
  // Bottom wall
  walls.emplace_back(0, worldHeight - thickness, worldWidth, thickness,
                     wallColor);
  // Left wall
  walls.emplace_back(0, 0, thickness, worldHeight, wallColor);
  // Right wall
  walls.emplace_back(worldWidth - thickness, 0, thickness, worldHeight,
                     wallColor);

  return walls;
}

std::vector<SceneData::ObstacleData>
SceneTemplates::createMazeWalls(float worldWidth, float worldHeight) {
  std::vector<SceneData::ObstacleData> walls =
      createBorderWalls(worldWidth, worldHeight);

  // Create a simple maze pattern
  float cellSize = 200.0f;
  int cols = static_cast<int>(worldWidth / cellSize);
  int rows = static_cast<int>(worldHeight / cellSize);

  for (int row = 1; row < rows - 1; row += 2) {
    for (int col = 1; col < cols - 1; col += 2) {
      float x = col * cellSize;
      float y = row * cellSize;

      // Add walls in a maze-like pattern
      if ((row + col) % 3 == 0) {
        walls.push_back({x, y, cellSize, 30, getRandomColor()});
      }
      if ((row + col) % 4 == 0) {
        walls.push_back({x, y, 30, cellSize, getRandomColor()});
      }
    }
  }

  return walls;
}

std::vector<SceneData::ObstacleData>
SceneTemplates::createArenaObstacles(float worldWidth, float worldHeight) {
  std::vector<SceneData::ObstacleData> obstacles;

  // Central pillar
  obstacles.push_back(
      {worldWidth / 2 - 50, worldHeight / 2 - 50, 100, 100, getRandomColor()});

  // Corner covers
  obstacles.push_back({200, 200, 150, 80, getRandomColor()});
  obstacles.push_back({worldWidth - 350, 200, 150, 80, getRandomColor()});
  obstacles.push_back({200, worldHeight - 280, 150, 80, getRandomColor()});
  obstacles.push_back(
      {worldWidth - 350, worldHeight - 280, 150, 80, getRandomColor()});

  return obstacles;
}

std::vector<SceneData::ObstacleData>
SceneTemplates::createPlatformerObstacles(float worldWidth, float worldHeight) {
  std::vector<SceneData::ObstacleData> obstacles;

  // Ground platforms
  obstacles.emplace_back(0, worldHeight - 50, worldWidth, 50,
                         glm::vec4(0.4f, 0.2f, 0.1f, 1.0f));

  // Jumping platforms
  for (int i = 1; i < 8; i++) {
    float x = i * 300;
    float y = worldHeight - 200 - (i % 3) * 150;
    obstacles.emplace_back(x, y, 200, 30, getRandomColor());
  }

  // Vertical walls for challenge
  obstacles.emplace_back(800, 400, 30, 400, getRandomColor());
  obstacles.emplace_back(1600, 200, 30, 600, getRandomColor());

  return obstacles;
}

std::vector<SceneData::CollectibleData>
SceneTemplates::createGridCollectibles(float worldWidth, float worldHeight,
                                       int count) {
  std::vector<SceneData::CollectibleData> collectibles;

  int cols = static_cast<int>(std::sqrt(count));
  int rows = (count + cols - 1) / cols;

  float spacingX = (worldWidth - 200) / (cols + 1);
  float spacingY = (worldHeight - 200) / (rows + 1);

  for (int row = 0; row < rows && collectibles.size() < count; row++) {
    for (int col = 0; col < cols && collectibles.size() < count; col++) {
      float x = 100 + (col + 1) * spacingX;
      float y = 100 + (row + 1) * spacingY;
      collectibles.push_back({x, y, getRandomColor()});
    }
  }

  return collectibles;
}

std::vector<SceneData::CollectibleData>
SceneTemplates::createRandomCollectibles(float worldWidth, float worldHeight,
                                         int count) {
  std::vector<SceneData::CollectibleData> collectibles;
  std::random_device rd;
  std::mt19937 gen(rd());

  for (int i = 0; i < count; i++) {
    float x = randomFloat(100, worldWidth - 100);
    float y = randomFloat(100, worldHeight - 100);
    collectibles.push_back({x, y, getRandomColor()});
  }

  return collectibles;
}

std::vector<SceneData::CollectibleData>
SceneTemplates::createPathCollectibles(float worldWidth, float worldHeight) {
  std::vector<SceneData::CollectibleData> collectibles;

  // Create a path of collectibles
  for (int i = 0; i < 10; i++) {
    float x = 200 + i * (worldWidth - 400) / 9;
    float y = worldHeight / 2 + std::sin(i * 0.5f) * 200;
    collectibles.push_back({x, y, getRandomColor()});
  }

  return collectibles;
}

std::vector<SceneData::EnemyData>
SceneTemplates::createBasicEnemies(float worldWidth, float worldHeight,
                                   int count) {
  std::vector<SceneData::EnemyData> enemies;

  for (int i = 0; i < count; i++) {
    float x = randomFloat(200, worldWidth - 200);
    float y = randomFloat(200, worldHeight - 200);

    MovementPattern pattern =
        (i % 2 == 0) ? MovementPattern::PATROL : MovementPattern::CIRCULAR;

    if (pattern == MovementPattern::PATROL) {
      enemies.emplace_back(x, y, pattern, 80.0f);
      enemies.back().patrolPoint1 = glm::vec2(x, y);
      enemies.back().patrolPoint2 =
          glm::vec2(x + randomFloat(-200, 200), y + randomFloat(-200, 200));
    } else {
      enemies.emplace_back(x, y, pattern, 60.0f);
      enemies.back().radius = randomFloat(80, 150);
    }
  }

  return enemies;
}

std::vector<SceneData::EnemyData>
SceneTemplates::createGauntletEnemies(float worldWidth, float worldHeight) {
  std::vector<SceneData::EnemyData> enemies;

  // Fast patrol enemies
  enemies.emplace_back(400, 300, MovementPattern::PATROL, 150.0f);
  enemies.back().patrolPoint1 = glm::vec2(300, 300);
  enemies.back().patrolPoint2 = glm::vec2(500, 300);
  enemies.emplace_back(1200, 600, MovementPattern::PATROL, 150.0f);
  enemies.back().patrolPoint1 = glm::vec2(1100, 600);
  enemies.back().patrolPoint2 = glm::vec2(1300, 600);

  // Circular enemies
  enemies.emplace_back(600, 400, MovementPattern::CIRCULAR, 100.0f);
  enemies.back().radius = 120.0f;
  enemies.emplace_back(1000, 800, MovementPattern::CIRCULAR, 120.0f);
  enemies.back().radius = 150.0f;

  // Stationary guards
  enemies.emplace_back(800, 200, MovementPattern::HORIZONTAL, 0.0f);
  enemies.emplace_back(800, 1000, MovementPattern::HORIZONTAL, 0.0f);

  return enemies;
}

std::vector<SceneData::EnemyData>
SceneTemplates::createArenaEnemies(float worldWidth, float worldHeight) {
  std::vector<SceneData::EnemyData> enemies;

  // Corner enemies
  enemies.emplace_back(200, 200, MovementPattern::HORIZONTAL, 0.0f);
  enemies.emplace_back(worldWidth - 200, 200, MovementPattern::HORIZONTAL,
                       0.0f);
  enemies.emplace_back(200, worldHeight - 200, MovementPattern::HORIZONTAL,
                       0.0f);
  enemies.emplace_back(worldWidth - 200, worldHeight - 200,
                       MovementPattern::HORIZONTAL, 0.0f);

  // Mobile threats
  enemies.emplace_back(worldWidth / 2, 200, MovementPattern::PATROL, 100.0f);
  enemies.back().patrolPoint1 = glm::vec2(300, 200);
  enemies.back().patrolPoint2 = glm::vec2(worldWidth - 300, 200);
  enemies.emplace_back(worldWidth / 2, worldHeight - 200,
                       MovementPattern::PATROL, 100.0f);
  enemies.back().patrolPoint1 = glm::vec2(300, worldHeight - 200);
  enemies.back().patrolPoint2 = glm::vec2(worldWidth - 300, worldHeight - 200);

  return enemies;
}

glm::vec4 SceneTemplates::getRandomColor() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<float> dis(0.3f, 0.9f);

  return glm::vec4(dis(gen), dis(gen), dis(gen), 1.0f);
}

float SceneTemplates::randomFloat(float min, float max) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(min, max);
  return dis(gen);
}

bool SceneTemplates::isPointFree(
    const glm::vec2 &point,
    const std::vector<SceneData::ObstacleData> &obstacles, float radius) {
  for (const auto &obstacle : obstacles) {
    if (point.x >= obstacle.x - radius &&
        point.x <= obstacle.x + obstacle.width + radius &&
        point.y >= obstacle.y - radius &&
        point.y <= obstacle.y + obstacle.height + radius) {
      return false;
    }
  }
  return true;
}