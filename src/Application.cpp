#include "Application.h"
#include <glad/glad.h>
#include <iostream>
#include <openglErrorReporting.h>

Application::Application()
    : window(nullptr), windowWidth(800), windowHeight(600), playerSpeed(200.0f),
      lastTime(0.0), isRunning(false), inputManager(nullptr) {}

Application::~Application() { shutdown(); }

bool Application::initialize(int width, int height, const char *title) {
  if (!initializeWindow(width, height, title))
    return false;

  if (!initializeOpenGL())
    return false;

  if (!initializeGame())
    return false;

  isRunning = true;
  return true;
}

void Application::run() {
  if (!isRunning)
    return;

  lastTime = glfwGetTime();

  while (!glfwWindowShouldClose(window) && isRunning) {
    // Calculate delta time
    double currentTime = glfwGetTime();
    float deltaTime = static_cast<float>(currentTime - lastTime);
    lastTime = currentTime;

    // Handle events
    handleEvents();

    // Update
    update(deltaTime);

    // Render
    render();

    // Swap buffers and poll events
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void Application::shutdown() {
  // Shutdown audio system
  audioManager.shutdown();

  uiManager.shutdown();

  if (inputManager) {
    delete inputManager;
    inputManager = nullptr;
  }

  if (window) {
    glfwDestroyWindow(window);
    window = nullptr;
  }

  glfwTerminate();
  isRunning = false;
}

bool Application::initializeWindow(int width, int height, const char *title) {
  glfwSetErrorCallback(errorCallback);

  if (!glfwInit())
    return false;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

  window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (!window) {
    glfwTerminate();
    return false;
  }

  windowWidth = width;
  windowHeight = height;

  glfwMakeContextCurrent(window);
  return true;
}

bool Application::initializeOpenGL() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    return false;
  }

  enableReportGlErrors();

  // Initialize gl2d
  gl2d::init();
  renderer.create();

  return true;
}

bool Application::initializeGame() {
  // Initialize input manager
  inputManager = new InputManager(window);

  // Initialize audio manager
  if (!audioManager.initialize()) {
    std::cerr << "Failed to initialize audio system!" << std::endl;
    // Audio failure is not critical, continue without audio
  } else {
    // Load collectible pickup sound
    audioManager.loadSound("collectible_pickup",
                           RESOURCES_PATH "collectible_pickup.mp3");
  }

  // Initialize UI manager
  if (!uiManager.initialize(window))
    return false;

  // Initialize game world
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
  gameWorld.initialize(windowWidth, windowHeight);

  // Pass audio manager reference to game world
  gameWorld.setAudioManager(&audioManager);

  return true;
}

void Application::update(float deltaTime) {
  // Update FPS counter
  fpsCounter.update(deltaTime);

  // Update input
  inputManager->update();

  // Handle input
  if (inputManager->isExitPressed()) {
    isRunning = false;
    return;
  }

  // Cache game state to avoid repeated calls
  bool isGamePlaying = gameWorld.getGameStateManager().isPlaying();
  bool isGameOver = gameWorld.getGameStateManager().isGameOver();

  if (inputManager->isRestartPressed() && isGameOver) {
    gameWorld.initialize(windowWidth, windowHeight);
  }

  // Update window size only if needed (cache previous values)
  static int lastWindowWidth = 0, lastWindowHeight = 0;
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
  if (windowWidth != lastWindowWidth || windowHeight != lastWindowHeight) {
    gameWorld.updateScreenSize(windowWidth, windowHeight);
    lastWindowWidth = windowWidth;
    lastWindowHeight = windowHeight;
  }

  // Handle mouse input for pathfinding
  if (inputManager->isRightMouseJustPressed() && isGamePlaying) {
    gameWorld.handleMouseInput(inputManager->getMousePosition());
  }

  // Only allow player movement if game is playing and not following a path
  if (isGamePlaying) {
    glm::vec2 movement = inputManager->getMovementInput();
    // Only allow keyboard movement if not following a path
    if (glm::length(movement) > 0.1f) {
      // Cancel pathfinding if player uses keyboard
      gameWorld.stopPathfinding();
      gameWorld.updatePlayer(movement.x, movement.y, playerSpeed, deltaTime);
    }
  }

  gameWorld.updateCamera(deltaTime);
  gameWorld.update(deltaTime);

  // Update pathfinding with actual player speed
  if (isGamePlaying) {
    gameWorld.updatePathfinding(deltaTime, playerSpeed);
  }
}

void Application::render() {
  glViewport(0, 0, windowWidth, windowHeight);
  glClear(GL_COLOR_BUFFER_BIT);

  // Begin UI frame
  uiManager.beginFrame();

  // Set up camera for following player
  renderer.updateWindowMetrics(windowWidth, windowHeight);
  gl2d::Camera camera;
  glm::vec2 cameraPos = gameWorld.getCameraPosition();
  camera.position = cameraPos;
  renderer.setCamera(camera);

  // Render game world
  gameWorld.render(&renderer);
  renderer.flush();

  // Render UI
  uiManager.renderGameUI(gameWorld, fpsCounter, playerSpeed);

  // End UI frame
  uiManager.endFrame();
}

void Application::handleEvents() {
  // Additional event handling can be added here
  // For now, GLFW handles events through polling
}

void Application::errorCallback(int error, const char *description) {
  std::cout << "GLFW Error (" << error << "): " << description << std::endl;
}