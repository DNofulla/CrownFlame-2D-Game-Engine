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
  // Save window settings before shutdown
  if (window) {
    settings.saveCurrentWindowState(window);
  }

  // Shutdown hot reload manager
  hotReloadManager.shutdown();

  // Shutdown scene manager
  sceneManager.shutdown();

  // Shutdown asset manager
  assetManager.shutdown();

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

  // Set up window close callback to save settings
  glfwSetWindowUserPointer(window, this);
  glfwSetWindowCloseCallback(window, windowCloseCallback);

  // Try to restore window to the last monitor it was on
  settings.restoreMonitorSettings(window);

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

  // Initialize asset manager
  if (!assetManager.initialize()) {
    std::cerr << "Failed to initialize asset management system!" << std::endl;
    return false;
  } else {
    std::cout << "Asset management system initialized successfully"
              << std::endl;
  }

  // Initialize audio manager
  if (!audioManager.initialize()) {
    std::cerr << "Failed to initialize audio system!" << std::endl;
    // Audio failure is not critical, continue without audio
  } else {
    // Load collectible pickup sound
    audioManager.loadSound("collectible_pickup",
                           RESOURCES_PATH "audio/collectible_pickup.mp3");
  }

  // Initialize hot reload manager
  if (!hotReloadManager.initialize(this, &sceneManager, &audioManager)) {
    std::cerr << "Failed to initialize hot reload system!" << std::endl;
    // Hot reload failure is not critical, continue without hot reloading
  } else {
    std::cout << "Hot reload system initialized successfully" << std::endl;
  }

  // Initialize UI manager
  if (!uiManager.initialize(window))
    return false;

  // Initialize scene manager
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
  if (!sceneManager.initialize(windowWidth, windowHeight)) {
    std::cerr << "Failed to initialize scene manager!" << std::endl;
    return false;
  }

  // Pass audio manager reference to scene manager
  sceneManager.setAudioManager(&audioManager);

  // Load default scene
  auto defaultScene = SceneManager::createDefaultScene("default");
  if (!sceneManager.loadSceneFromDefinition("default", defaultScene)) {
    std::cerr << "Failed to load default scene!" << std::endl;
    return false;
  }

  // Load example scenes from files (if they exist)
  sceneManager.loadSceneFromFile("level1",
                                 RESOURCES_PATH "scenes/level1.scene");
  sceneManager.loadSceneFromFile("level2",
                                 RESOURCES_PATH "scenes/level2.scene");
  sceneManager.loadSceneFromFile("sandbox",
                                 RESOURCES_PATH "scenes/sandbox.scene");

  // Register scenes for hot reloading
  hotReloadManager.registerScene("level1",
                                 RESOURCES_PATH "scenes/level1.scene");
  hotReloadManager.registerScene("level2",
                                 RESOURCES_PATH "scenes/level2.scene");
  hotReloadManager.registerScene("sandbox",
                                 RESOURCES_PATH "scenes/sandbox.scene");

  // Register audio for hot reloading
  hotReloadManager.registerAudio("collectible_pickup",
                                 RESOURCES_PATH "audio/collectible_pickup.mp3");

  // Create a custom scene programmatically
  auto customScene = SceneManager::createDefaultScene("custom");
  customScene.name = "Custom Scene";
  customScene.description = "Programmatically created scene";
  customScene.playerSpawn.x = 200.0f;
  customScene.playerSpawn.y = 200.0f;
  // Add different collectible positions
  customScene.collectibles.clear();
  customScene.collectibles.emplace_back(350.0f, 250.0f);
  customScene.collectibles.emplace_back(550.0f, 100.0f);
  customScene.collectibles.emplace_back(450.0f, 450.0f);
  sceneManager.loadSceneFromDefinition("custom", customScene);

  // Change to default scene
  if (!sceneManager.changeSceneInstant("default")) {
    std::cerr << "Failed to activate default scene!" << std::endl;
    return false;
  }

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

  // Update scene manager first
  sceneManager.update(deltaTime);

  // Get current scene for input handling
  Scene *currentScene = sceneManager.getCurrentScene();
  GameWorld *currentGameWorld =
      currentScene ? currentScene->getGameWorld() : nullptr;

  if (currentGameWorld) {
    // Cache game state to avoid repeated calls
    bool isGamePlaying = currentGameWorld->getGameStateManager().isPlaying();
    bool isGameOver = currentGameWorld->getGameStateManager().isGameOver();

    if (inputManager->isRestartPressed() && isGameOver) {
      sceneManager.restartCurrentScene();
    }

    // Update window size only if needed (cache previous values)
    static int lastWindowWidth = 0, lastWindowHeight = 0;
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
    if (windowWidth != lastWindowWidth || windowHeight != lastWindowHeight) {
      sceneManager.updateScreenSize(windowWidth, windowHeight);
      lastWindowWidth = windowWidth;
      lastWindowHeight = windowHeight;
    }

    // Handle mouse input for pathfinding
    if (inputManager->isRightMouseJustPressed() && isGamePlaying) {
      currentGameWorld->handleMouseInput(inputManager->getMousePosition());
    }

    // Only allow player movement if game is playing and not following a path
    if (isGamePlaying) {
      glm::vec2 movement = inputManager->getMovementInput();
      // Only allow keyboard movement if not following a path
      if (glm::length(movement) > 0.1f) {
        // Cancel pathfinding if player uses keyboard
        currentGameWorld->stopPathfinding();
        currentGameWorld->updatePlayer(movement.x, movement.y, playerSpeed,
                                       deltaTime);
      }
    }

    currentGameWorld->updateCamera(deltaTime);

    // Update pathfinding with actual player speed
    if (isGamePlaying) {
      currentGameWorld->updatePathfinding(deltaTime, playerSpeed);
    }
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

  // Get current scene for rendering
  Scene *currentScene = sceneManager.getCurrentScene();
  GameWorld *currentGameWorld =
      currentScene ? currentScene->getGameWorld() : nullptr;

  if (currentGameWorld) {
    glm::vec2 cameraPos = currentGameWorld->getCameraPosition();
    camera.position = cameraPos;
    renderer.setCamera(camera);

    // Render current scene
    sceneManager.render(&renderer);
    renderer.flush();

    // Render UI using current game world, scene manager, and hot reload manager
    uiManager.renderGameUI(*currentGameWorld, fpsCounter, playerSpeed,
                           sceneManager, hotReloadManager);
  } else {
    // Fallback: render default camera
    camera.position = glm::vec2(0, 0);
    renderer.setCamera(camera);
    renderer.flush();

    // Render UI with legacy game world as fallback
    uiManager.renderGameUI(gameWorld, fpsCounter, playerSpeed);
  }

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

void Application::windowCloseCallback(GLFWwindow *window) {
  // Get the Application instance from the window user pointer
  Application *app =
      static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (app) {
    // Save the current window state before closing
    app->settings.saveCurrentWindowState(window);
    std::cout << "Window closing - settings saved." << std::endl;
  }
}

// Scene management methods
bool Application::loadScene(const std::string &sceneName,
                            const std::string &filePath) {
  return sceneManager.loadSceneFromFile(sceneName, filePath);
}

bool Application::loadSceneFromDefinition(
    const std::string &sceneName,
    const SceneData::SceneDefinition &definition) {
  return sceneManager.loadSceneFromDefinition(sceneName, definition);
}

bool Application::changeScene(const std::string &sceneName) {
  return sceneManager.changeScene(sceneName);
}

void Application::restartCurrentScene() { sceneManager.restartCurrentScene(); }