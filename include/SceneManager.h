#pragma once
#include "Scene.h"
#include "SceneData.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

// Forward declarations
class AudioManager;

class SceneManager {
private:
  // Scene storage
  std::unordered_map<std::string, std::unique_ptr<Scene>> loadedScenes;
  Scene *currentScene;
  Scene *nextScene;

  // Transition state
  bool isTransitioning;
  SceneData::SceneTransition currentTransition;
  float transitionProgress; // 0.0 to 1.0
  float transitionTimer;

  // Screen dimensions
  int screenWidth, screenHeight;

  // Shared systems
  AudioManager *audioManager;

  // Callbacks
  std::function<void(const std::string &)> onSceneChanged;
  std::function<void(const std::string &, const std::string &)>
      onSceneTransitionStarted;

public:
  SceneManager();
  ~SceneManager();

  // Initialization
  bool initialize(int screenWidth, int screenHeight);
  void shutdown();

  // Scene management
  bool loadSceneFromFile(const std::string &sceneName,
                         const std::string &filePath);
  bool loadSceneFromDefinition(const std::string &sceneName,
                               const SceneData::SceneDefinition &definition);
  void unloadScene(const std::string &sceneName);
  void unloadAllScenes();

  // Scene transitions
  bool changeScene(const std::string &sceneName,
                   const SceneData::SceneTransition &transition =
                       SceneData::SceneTransition());
  bool changeSceneInstant(const std::string &sceneName);
  void restartCurrentScene();

  // Update and render
  void update(float deltaTime);
  void render(void *renderer);

  // Scene queries
  Scene *getCurrentScene() { return currentScene; }
  const Scene *getCurrentScene() const { return currentScene; }
  const std::string &getCurrentSceneName() const;

  bool hasScene(const std::string &sceneName) const;
  Scene *getScene(const std::string &sceneName);
  bool isTransitionInProgress() const { return isTransitioning; }

  // System management
  void setAudioManager(AudioManager *manager);
  void updateScreenSize(int width, int height);

  // Callbacks
  void
  setOnSceneChangedCallback(std::function<void(const std::string &)> callback);
  void setOnSceneTransitionStartedCallback(
      std::function<void(const std::string &, const std::string &)> callback);

  // Scene file operations
  bool saveSceneToFile(const std::string &sceneName,
                       const std::string &filePath) const;
  static SceneData::SceneDefinition createDefaultScene(const std::string &name);

  // Auto-progression (for scene completion triggers)
  void checkAutoProgression();
  bool shouldAutoProgress() const;

private:
  // Internal helpers
  void finishTransition();
  void updateTransition(float deltaTime);
  void renderTransition(void *renderer);

  // Transition effects
  void renderFadeTransition(void *renderer, float progress);
  void renderSlideTransition(void *renderer, float progress,
                             SceneData::TransitionType type);

  // Scene validation
  bool
  validateSceneDefinition(const SceneData::SceneDefinition &definition) const;

  // File I/O helpers
  bool saveSceneDefinitionToFile(const SceneData::SceneDefinition &definition,
                                 const std::string &filePath) const;
  bool
  loadSceneDefinitionFromFile(const std::string &filePath,
                              SceneData::SceneDefinition &definition) const;
};