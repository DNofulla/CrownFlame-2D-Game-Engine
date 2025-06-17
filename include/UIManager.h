#pragma once
#include "FPSCounter.h"
#include "GameWorld.h"
#include <glm/glm.hpp>
#include <string>

// Forward declarations
class SceneManager;
class Scene;
class HotReloadManager;
struct SceneValidationResult;

class UIManager {
private:
  bool initialized;

  // Feature UI state
  std::string selectedTemplateName;
  int selectedTemplateIndex;
  bool showTemplateCreator;
  bool showFileBrowser;
  bool showSceneInfo;
  bool showValidationResults;
  SceneValidationResult *currentValidationResult;

  // Scene list management
  bool needsSceneListRefresh;

public:
  UIManager();
  ~UIManager();

  // Initialize/shutdown ImGui
  bool
  initialize(void *window); // Using void* to avoid GLFW dependency in header
  void shutdown();

  // Frame management
  void beginFrame();
  void endFrame();

  // UI rendering
  void renderGameUI(GameWorld &gameWorld, const FPSCounter &fpsCounter,
                    float &playerSpeed);

  // UI rendering with scene management
  void renderGameUI(GameWorld &gameWorld, const FPSCounter &fpsCounter,
                    float &playerSpeed, SceneManager &sceneManager);

  // UI rendering with hot reload management
  void renderGameUI(GameWorld &gameWorld, const FPSCounter &fpsCounter,
                    float &playerSpeed, SceneManager &sceneManager,
                    HotReloadManager &hotReloadManager);

private:
  // Helper methods for different UI sections
  void renderPlayerInfo(GameObject *player, const glm::vec2 &cameraPos);
  void renderControls();
  void renderWorldInfo(GameWorld &gameWorld);
  void renderGameState(GameWorld &gameWorld);
  void renderSceneSelector(SceneManager &sceneManager);

  // New feature UI methods
  void renderSceneInformation(SceneManager &sceneManager);
  void renderTemplateCreator(SceneManager &sceneManager);
  void renderFileBrowser(SceneManager &sceneManager);
  void renderValidationResults();
  void renderHotReloadControls(HotReloadManager &hotReloadManager);

  // Utility methods
  void openFileDialog(SceneManager &sceneManager);
  void saveFileDialog(SceneManager &sceneManager);
  void createSceneFromTemplate(SceneManager &sceneManager);
  void validateCurrentScene(SceneManager &sceneManager);
  void refreshSceneList(); // Trigger scene list refresh
};