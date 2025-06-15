#pragma once
#include "AudioManager.h"
#include "FPSCounter.h"
#include "GameWorld.h"
#include "InputManager.h"
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
  GameWorld gameWorld;
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
                  const char *title = "Game with Collision Detection");
  void run();
  void shutdown();

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