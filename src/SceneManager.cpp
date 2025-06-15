#include "SceneManager.h"
#include "AudioManager.h"
#include <fstream>
#include <gl2d/gl2d.h>
#include <iostream>
#include <sstream>

SceneManager::SceneManager()
    : currentScene(nullptr), nextScene(nullptr), isTransitioning(false),
      transitionProgress(0.0f), transitionTimer(0.0f), screenWidth(800),
      screenHeight(600), audioManager(nullptr) {}

SceneManager::~SceneManager() { shutdown(); }

bool SceneManager::initialize(int screenWidth, int screenHeight) {
  this->screenWidth = screenWidth;
  this->screenHeight = screenHeight;

  std::cout << "SceneManager initialized with screen size: " << screenWidth
            << "x" << screenHeight << std::endl;
  return true;
}

void SceneManager::shutdown() {
  unloadAllScenes();
  currentScene = nullptr;
  nextScene = nullptr;
  isTransitioning = false;
  audioManager = nullptr;
}

bool SceneManager::loadSceneFromDefinition(
    const std::string &sceneName,
    const SceneData::SceneDefinition &definition) {
  if (hasScene(sceneName)) {
    std::cout << "Scene '" << sceneName << "' is already loaded." << std::endl;
    return true;
  }

  if (!validateSceneDefinition(definition)) {
    std::cerr << "Invalid scene definition for '" << sceneName << "'"
              << std::endl;
    return false;
  }

  auto scene = std::make_unique<Scene>(definition);

  if (!scene->loadScene(screenWidth, screenHeight)) {
    std::cerr << "Failed to load scene '" << sceneName << "'" << std::endl;
    return false;
  }

  if (audioManager) {
    scene->setAudioManager(audioManager);
  }

  loadedScenes[sceneName] = std::move(scene);

  std::cout << "Scene '" << sceneName << "' loaded successfully." << std::endl;
  return true;
}

bool SceneManager::loadSceneFromFile(const std::string &sceneName,
                                     const std::string &filePath) {
  SceneData::SceneDefinition definition;

  if (!loadSceneDefinitionFromFile(filePath, definition)) {
    std::cerr << "Failed to load scene definition from file: " << filePath
              << std::endl;
    return false;
  }

  // Use the name from the definition, but allow override
  if (definition.name.empty()) {
    definition.name = sceneName;
  }

  return loadSceneFromDefinition(sceneName, definition);
}

void SceneManager::unloadScene(const std::string &sceneName) {
  auto it = loadedScenes.find(sceneName);
  if (it == loadedScenes.end()) {
    return;
  }

  // Don't unload if it's the current scene
  if (currentScene == it->second.get()) {
    std::cerr << "Cannot unload current scene '" << sceneName << "'"
              << std::endl;
    return;
  }

  it->second->unloadScene();
  loadedScenes.erase(it);

  std::cout << "Scene '" << sceneName << "' unloaded." << std::endl;
}

void SceneManager::unloadAllScenes() {
  for (auto &pair : loadedScenes) {
    pair.second->unloadScene();
  }
  loadedScenes.clear();
  currentScene = nullptr;
  nextScene = nullptr;
  std::cout << "All scenes unloaded." << std::endl;
}

bool SceneManager::changeScene(const std::string &sceneName,
                               const SceneData::SceneTransition &transition) {
  if (isTransitioning) {
    std::cerr << "Already transitioning between scenes!" << std::endl;
    return false;
  }

  auto it = loadedScenes.find(sceneName);
  if (it == loadedScenes.end()) {
    std::cerr << "Scene '" << sceneName << "' is not loaded!" << std::endl;
    return false;
  }

  nextScene = it->second.get();

  if (currentScene == nextScene) {
    std::cout << "Already in scene '" << sceneName << "'" << std::endl;
    return true;
  }

  // Start transition
  if (transition.type == SceneData::TransitionType::INSTANT) {
    // Instant transition
    finishTransition();
  } else {
    // Start animated transition
    isTransitioning = true;
    currentTransition = transition;
    transitionProgress = 0.0f;
    transitionTimer = 0.0f;

    std::string currentName = currentScene ? currentScene->getName() : "none";
    std::cout << "Starting transition from '" << currentName << "' to '"
              << sceneName << "'" << std::endl;

    if (onSceneTransitionStarted) {
      onSceneTransitionStarted(currentName, sceneName);
    }
  }

  return true;
}

bool SceneManager::changeSceneInstant(const std::string &sceneName) {
  return changeScene(sceneName, SceneData::SceneTransition(
                                    SceneData::TransitionType::INSTANT));
}

void SceneManager::restartCurrentScene() {
  if (!currentScene)
    return;

  std::string sceneName = currentScene->getName();
  SceneData::SceneDefinition definition = currentScene->getDefinition();

  // Reload the scene
  currentScene->unloadScene();
  currentScene->setDefinition(definition);
  currentScene->loadScene(screenWidth, screenHeight);

  if (audioManager) {
    currentScene->setAudioManager(audioManager);
  }

  currentScene->activateScene();

  std::cout << "Restarted scene '" << sceneName << "'" << std::endl;
}

void SceneManager::update(float deltaTime) {
  if (isTransitioning) {
    updateTransition(deltaTime);
  }

  // Update current scene
  if (currentScene) {
    currentScene->update(deltaTime);
  }

  // Check for auto-progression
  checkAutoProgression();
}

void SceneManager::render(void *renderer) {
  // Render current scene
  if (currentScene) {
    currentScene->render(renderer);
  }

  // Render transition effects
  if (isTransitioning) {
    renderTransition(renderer);
  }
}

const std::string &SceneManager::getCurrentSceneName() const {
  static std::string empty = "";
  return currentScene ? currentScene->getName() : empty;
}

bool SceneManager::hasScene(const std::string &sceneName) const {
  return loadedScenes.find(sceneName) != loadedScenes.end();
}

Scene *SceneManager::getScene(const std::string &sceneName) {
  auto it = loadedScenes.find(sceneName);
  return it != loadedScenes.end() ? it->second.get() : nullptr;
}

void SceneManager::setAudioManager(AudioManager *manager) {
  audioManager = manager;

  // Set for all loaded scenes
  for (auto &pair : loadedScenes) {
    pair.second->setAudioManager(manager);
  }
}

void SceneManager::updateScreenSize(int width, int height) {
  screenWidth = width;
  screenHeight = height;

  // Update all loaded scenes
  for (auto &pair : loadedScenes) {
    pair.second->updateScreenSize(width, height);
  }
}

void SceneManager::setOnSceneChangedCallback(
    std::function<void(const std::string &)> callback) {
  onSceneChanged = callback;
}

void SceneManager::setOnSceneTransitionStartedCallback(
    std::function<void(const std::string &, const std::string &)> callback) {
  onSceneTransitionStarted = callback;
}

bool SceneManager::saveSceneToFile(const std::string &sceneName,
                                   const std::string &filePath) const {
  auto it = loadedScenes.find(sceneName);
  if (it == loadedScenes.end()) {
    std::cerr << "Scene '" << sceneName << "' not found!" << std::endl;
    return false;
  }

  return saveSceneDefinitionToFile(it->second->getDefinition(), filePath);
}

SceneData::SceneDefinition
SceneManager::createDefaultScene(const std::string &name) {
  SceneData::SceneDefinition scene(name);
  scene.description = "Default scene created by SceneManager";

  // Add some default objects
  scene.obstacles.emplace_back(300.0f, 200.0f, 80.0f, 80.0f);
  scene.obstacles.emplace_back(500.0f, 300.0f, 60.0f, 120.0f);

  scene.collectibles.emplace_back(450.0f, 150.0f);
  scene.collectibles.emplace_back(150.0f, 250.0f);

  scene.enemies.emplace_back(400.0f, 300.0f, MovementPattern::HORIZONTAL);
  scene.enemies.emplace_back(700.0f, 450.0f, MovementPattern::VERTICAL);

  scene.transitionTrigger = "collectibles_complete";

  return scene;
}

void SceneManager::checkAutoProgression() {
  if (!currentScene || isTransitioning)
    return;

  if (currentScene->isSceneComplete() &&
      !currentScene->getNextScene().empty()) {
    std::string nextSceneName = currentScene->getNextScene();

    if (hasScene(nextSceneName)) {
      std::cout << "Auto-progressing to scene: " << nextSceneName << std::endl;
      changeScene(nextSceneName);
    } else {
      std::cout << "Scene completion detected, but next scene '"
                << nextSceneName << "' is not loaded." << std::endl;
    }
  }
}

bool SceneManager::shouldAutoProgress() const {
  if (!currentScene || isTransitioning)
    return false;
  return currentScene->isSceneComplete() &&
         !currentScene->getNextScene().empty();
}

void SceneManager::finishTransition() {
  if (currentScene) {
    currentScene->deactivateScene();
  }

  Scene *oldScene = currentScene;
  currentScene = nextScene;
  nextScene = nullptr;

  if (currentScene) {
    currentScene->activateScene();

    std::cout << "Scene transition completed. Current scene: "
              << currentScene->getName() << std::endl;

    if (onSceneChanged) {
      onSceneChanged(currentScene->getName());
    }
  }

  isTransitioning = false;
  transitionProgress = 0.0f;
  transitionTimer = 0.0f;
}

void SceneManager::updateTransition(float deltaTime) {
  transitionTimer += deltaTime;
  transitionProgress = transitionTimer / currentTransition.duration;

  if (transitionProgress >= 1.0f) {
    transitionProgress = 1.0f;
    finishTransition();
  }
}

void SceneManager::renderTransition(void *renderer) {
  switch (currentTransition.type) {
  case SceneData::TransitionType::FADE_TO_BLACK:
    renderFadeTransition(renderer, transitionProgress);
    break;
  case SceneData::TransitionType::SLIDE_LEFT:
  case SceneData::TransitionType::SLIDE_RIGHT:
  case SceneData::TransitionType::SLIDE_UP:
  case SceneData::TransitionType::SLIDE_DOWN:
    renderSlideTransition(renderer, transitionProgress, currentTransition.type);
    break;
  default:
    break;
  }
}

void SceneManager::renderFadeTransition(void *renderer, float progress) {
  auto *gl2dRenderer = static_cast<gl2d::Renderer2D *>(renderer);

  float alpha = 0.0f;
  if (progress < 0.5f) {
    // Fade out
    alpha = progress * 2.0f;
  } else {
    // Fade in
    alpha = (1.0f - progress) * 2.0f;
  }

  glm::vec4 fadeColor = currentTransition.fadeColor;
  fadeColor.a = alpha;

  // Render fullscreen fade overlay
  gl2dRenderer->renderRectangle({0, 0, (float)screenWidth, (float)screenHeight},
                                fadeColor);
}

void SceneManager::renderSlideTransition(void *renderer, float progress,
                                         SceneData::TransitionType type) {
  // Slide transitions would require rendering both scenes offset
  // This is a simplified implementation
  auto *gl2dRenderer = static_cast<gl2d::Renderer2D *>(renderer);

  glm::vec2 offset(0.0f, 0.0f);

  switch (type) {
  case SceneData::TransitionType::SLIDE_LEFT:
    offset.x = -screenWidth * progress;
    break;
  case SceneData::TransitionType::SLIDE_RIGHT:
    offset.x = screenWidth * progress;
    break;
  case SceneData::TransitionType::SLIDE_UP:
    offset.y = -screenHeight * progress;
    break;
  case SceneData::TransitionType::SLIDE_DOWN:
    offset.y = screenHeight * progress;
    break;
  default:
    break;
  }

  // For now, just render a simple sliding overlay
  float alpha = 0.8f * progress;
  glm::vec4 slideColor = currentTransition.fadeColor;
  slideColor.a = alpha;

  gl2dRenderer->renderRectangle(
      {offset.x, offset.y, (float)screenWidth, (float)screenHeight},
      slideColor);
}

bool SceneManager::validateSceneDefinition(
    const SceneData::SceneDefinition &definition) const {
  // Basic validation
  if (definition.name.empty()) {
    std::cerr << "Scene definition has empty name!" << std::endl;
    return false;
  }

  if (definition.world.width <= 0 || definition.world.height <= 0) {
    std::cerr << "Scene world dimensions must be positive!" << std::endl;
    return false;
  }

  return true;
}

bool SceneManager::saveSceneDefinitionToFile(
    const SceneData::SceneDefinition &definition,
    const std::string &filePath) const {
  std::ofstream file(filePath);
  if (!file.is_open()) {
    std::cerr << "Failed to open file for writing: " << filePath << std::endl;
    return false;
  }

  // Simple text-based format for now (could be JSON/XML in the future)
  file << "[SCENE]\n";
  file << "name=" << definition.name << "\n";
  file << "description=" << definition.description << "\n";
  file << "nextScene=" << definition.nextScene << "\n";
  file << "transitionTrigger=" << definition.transitionTrigger << "\n";

  file << "\n[WORLD]\n";
  file << "width=" << definition.world.width << "\n";
  file << "height=" << definition.world.height << "\n";
  file << "backgroundMusic=" << definition.world.backgroundMusic << "\n";

  file << "\n[CAMERA]\n";
  file << "followSpeed=" << definition.camera.followSpeed << "\n";
  file << "followEnabled="
       << (definition.camera.followEnabled ? "true" : "false") << "\n";

  file << "\n[PLAYER]\n";
  file << "spawnX=" << definition.playerSpawn.x << "\n";
  file << "spawnY=" << definition.playerSpawn.y << "\n";

  file << "\n[OBSTACLES]\n";
  for (const auto &obstacle : definition.obstacles) {
    file << obstacle.x << "," << obstacle.y << "," << obstacle.width << ","
         << obstacle.height << "\n";
  }

  file << "\n[COLLECTIBLES]\n";
  for (const auto &collectible : definition.collectibles) {
    file << collectible.x << "," << collectible.y << "\n";
  }

  file << "\n[ENEMIES]\n";
  for (const auto &enemy : definition.enemies) {
    file << enemy.x << "," << enemy.y << "," << static_cast<int>(enemy.pattern)
         << "," << enemy.speed << "\n";
  }

  file.close();
  std::cout << "Scene saved to file: " << filePath << std::endl;
  return true;
}

bool SceneManager::loadSceneDefinitionFromFile(
    const std::string &filePath, SceneData::SceneDefinition &definition) const {
  std::ifstream file(filePath);
  if (!file.is_open()) {
    std::cerr << "Failed to open file for reading: " << filePath << std::endl;
    return false;
  }

  std::string line;
  std::string currentSection;

  while (std::getline(file, line)) {
    // Skip empty lines and comments
    if (line.empty() || line[0] == '#')
      continue;

    // Check for section headers
    if (line[0] == '[' && line.back() == ']') {
      currentSection = line.substr(1, line.length() - 2);
      continue;
    }

    // Parse key-value pairs
    size_t equalPos = line.find('=');
    if (equalPos != std::string::npos && currentSection != "OBSTACLES" &&
        currentSection != "COLLECTIBLES" && currentSection != "ENEMIES") {
      std::string key = line.substr(0, equalPos);
      std::string value = line.substr(equalPos + 1);

      if (currentSection == "SCENE") {
        if (key == "name")
          definition.name = value;
        else if (key == "description")
          definition.description = value;
        else if (key == "nextScene")
          definition.nextScene = value;
        else if (key == "transitionTrigger")
          definition.transitionTrigger = value;
      } else if (currentSection == "WORLD") {
        if (key == "width")
          definition.world.width = std::stof(value);
        else if (key == "height")
          definition.world.height = std::stof(value);
        else if (key == "backgroundMusic")
          definition.world.backgroundMusic = value;
      } else if (currentSection == "CAMERA") {
        if (key == "followSpeed")
          definition.camera.followSpeed = std::stof(value);
        else if (key == "followEnabled")
          definition.camera.followEnabled = (value == "true");
      } else if (currentSection == "PLAYER") {
        if (key == "spawnX")
          definition.playerSpawn.x = std::stof(value);
        else if (key == "spawnY")
          definition.playerSpawn.y = std::stof(value);
      }
    }
    // Parse lists
    else if (currentSection == "OBSTACLES") {
      std::istringstream iss(line);
      std::string token;
      std::vector<float> values;

      while (std::getline(iss, token, ',')) {
        values.push_back(std::stof(token));
      }

      if (values.size() >= 4) {
        definition.obstacles.emplace_back(values[0], values[1], values[2],
                                          values[3]);
      }
    } else if (currentSection == "COLLECTIBLES") {
      std::istringstream iss(line);
      std::string token;
      std::vector<float> values;

      while (std::getline(iss, token, ',')) {
        values.push_back(std::stof(token));
      }

      if (values.size() >= 2) {
        definition.collectibles.emplace_back(values[0], values[1]);
      }
    } else if (currentSection == "ENEMIES") {
      std::istringstream iss(line);
      std::string token;
      std::vector<float> values;

      while (std::getline(iss, token, ',')) {
        values.push_back(std::stof(token));
      }

      if (values.size() >= 4) {
        MovementPattern pattern =
            static_cast<MovementPattern>(static_cast<int>(values[2]));
        definition.enemies.emplace_back(values[0], values[1], pattern,
                                        values[3]);
      }
    }
  }

  file.close();
  std::cout << "Scene loaded from file: " << filePath << std::endl;
  return true;
}