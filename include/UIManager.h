#pragma once
#include "FPSCounter.h"
#include "GameWorld.h"
#include <glm/glm.hpp>
#include <string>

// Forward declaration
class SceneManager;

class UIManager {
private:
  bool initialized;

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

private:
  // Helper methods for different UI sections
  void renderPlayerInfo(GameObject *player, const glm::vec2 &cameraPos);
  void renderControls();
  void renderWorldInfo(GameWorld &gameWorld);
  void renderGameState(GameWorld &gameWorld);
  void renderSceneSelector(SceneManager &sceneManager);
};