#pragma once
#include "AudioManager.h"
#include "FPSCounter.h"
#include "GameWorld.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "UIManager.h"
#include <GLFW/glfw3.h>
#include <gl2d/gl2d.h>

class Application {
private:
  // Window and graphics
  GLFWwindow *window;
  gl2d::Renderer2D renderer;
  int windowWidth, windowHeight;

  // Game systems
  GameWorld gameWorld; // Keep for backward compatibility, but SceneManager will
                       // take over
  SceneManager sceneManager;
  FPSCounter fpsCounter;
  InputManager *inputManager;
  UIManager uiManager;
  AudioManager audioManager;

  // Game settings
  float playerSpeed;

  // Timing
  double lastTime;
  bool isRunning;

public:
  Application();
  ~Application();

  // Application lifecycle
  bool initialize(int width = 800, int height = 600,
                  const char *title = "CrownFlame 2D Game Engine");
  void run();
  void shutdown();

  // Scene management
  bool loadScene(const std::string &sceneName, const std::string &filePath);
  bool loadSceneFromDefinition(const std::string &sceneName,
                               const SceneData::SceneDefinition &definition);
  bool changeScene(const std::string &sceneName);
  void restartCurrentScene();
  SceneManager &getSceneManager() { return sceneManager; }

private:
  // Initialization helpers
  bool initializeWindow(int width, int height, const char *title);
  bool initializeOpenGL();
  bool initializeGame();

  // Main loop
  void update(float deltaTime);
  void render();
  void handleEvents();

  // Event callbacks
  static void errorCallback(int error, const char *description);
};