#include "UIManager.h"
#include "SceneManager.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include "imguiThemes.h"
#include <GLFW/glfw3.h>
#include <vector>

UIManager::UIManager() : initialized(false) {}

UIManager::~UIManager() {
  if (initialized)
    shutdown();
}

bool UIManager::initialize(void *window) {
  GLFWwindow *glfwWindow = static_cast<GLFWwindow *>(window);

#if REMOVE_IMGUI == 0
  ImGui::CreateContext();
  imguiThemes::red();

  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  io.FontGlobalScale = 2.0f; // make text bigger

  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.Colors[ImGuiCol_WindowBg].w = 0.f;
    style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
  }

  ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  initialized = true;
  return true;
#else
  return false;
#endif
}

void UIManager::shutdown() {
#if REMOVE_IMGUI == 0
  if (initialized) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    initialized = false;
  }
#endif
}

void UIManager::beginFrame() {
#if REMOVE_IMGUI == 0
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
#endif
}

void UIManager::endFrame() {
#if REMOVE_IMGUI == 0
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // Handle multi-viewport
  ImGuiIO &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow *backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
#endif
}

void UIManager::renderGameUI(GameWorld &gameWorld, const FPSCounter &fpsCounter,
                             float &playerSpeed) {
#if REMOVE_IMGUI == 0
  ImGui::Begin("Game Controls");

  // Display FPS
  ImGui::Text("FPS: %.1f", fpsCounter.getFPS());
  ImGui::Separator();

  // Player and camera info
  GameObject *player = gameWorld.getPlayer();
  if (player) {
    glm::vec2 cameraPos = gameWorld.getCameraPosition();
    renderPlayerInfo(player, cameraPos);
  }

  // Controls
  renderControls();

  // Game settings
  ImGui::SliderFloat("Movement Speed", &playerSpeed, 50.0f, 500.0f);

  static float cameraSpeed = 5.0f;
  if (ImGui::SliderFloat("Camera Follow Speed", &cameraSpeed, 1.0f, 20.0f)) {
    gameWorld.setCameraFollowSpeed(cameraSpeed);
  }

  static bool cameraFollow = true;
  if (ImGui::Checkbox("Camera Follow Player", &cameraFollow)) {
    gameWorld.enableCameraFollow(cameraFollow);
  }

  if (ImGui::Button("Reset Game World")) {
    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
    gameWorld.initialize(width, height);
  }

  // World info
  renderWorldInfo(gameWorld);

  // Game state
  renderGameState(gameWorld);

  ImGui::End();
#endif
}

void UIManager::renderGameUI(GameWorld &gameWorld, const FPSCounter &fpsCounter,
                             float &playerSpeed, SceneManager &sceneManager) {
#if REMOVE_IMGUI == 0
  ImGui::Begin("Game Controls");

  // Display FPS
  ImGui::Text("FPS: %.1f", fpsCounter.getFPS());
  ImGui::Separator();

  // Scene selector
  renderSceneSelector(sceneManager);
  ImGui::Separator();

  // Player and camera info
  GameObject *player = gameWorld.getPlayer();
  if (player) {
    glm::vec2 cameraPos = gameWorld.getCameraPosition();
    renderPlayerInfo(player, cameraPos);
  }

  // Controls
  renderControls();

  // Game settings
  ImGui::SliderFloat("Movement Speed", &playerSpeed, 50.0f, 500.0f);

  static float cameraSpeed = 5.0f;
  if (ImGui::SliderFloat("Camera Follow Speed", &cameraSpeed, 1.0f, 20.0f)) {
    gameWorld.setCameraFollowSpeed(cameraSpeed);
  }

  static bool cameraFollow = true;
  if (ImGui::Checkbox("Camera Follow Player", &cameraFollow)) {
    gameWorld.enableCameraFollow(cameraFollow);
  }

  if (ImGui::Button("Restart Current Scene")) {
    sceneManager.restartCurrentScene();
  }

  // World info
  renderWorldInfo(gameWorld);

  // Game state
  renderGameState(gameWorld);

  ImGui::End();
#endif
}

void UIManager::renderPlayerInfo(GameObject *player,
                                 const glm::vec2 &cameraPos) {
  ImGui::Text("Player Position: (%.1f, %.1f)", player->bounds.x,
              player->bounds.y);
  ImGui::Text("Camera Position: (%.1f, %.1f)", cameraPos.x, cameraPos.y);
  ImGui::Text("World Size: 2000x1500 pixels");
}

void UIManager::renderControls() {
  ImGui::Text("Controls:");
  ImGui::Text("WASD or Arrow Keys - Move");
  ImGui::Text("ESC - Exit Game");
  ImGui::Separator();
}

void UIManager::renderWorldInfo(GameWorld &gameWorld) {
  ImGui::Separator();
  ImGui::Text("Objects in World:");
  ImGui::Text("Green Square - Player (you)");
  ImGui::Text("Red Rectangles - Obstacles (collision)");
  ImGui::Text("Yellow Squares - Collectibles");
  ImGui::Text("Pink Pigs - Enemies (avoid!)");
  ImGui::Text("Total Objects: %d", (int)gameWorld.getObjects().size());
  ImGui::Text("Total Enemies: %d", (int)gameWorld.getEnemies().size());
}

void UIManager::renderGameState(GameWorld &gameWorld) {
  if (gameWorld.getGameStateManager().isGameOver()) {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "GAME OVER!");
    ImGui::Text("You touched a pig!");
    ImGui::Text("Game Over Time: %.1f seconds",
                gameWorld.getGameStateManager().getGameOverTime());
    if (ImGui::Button("Restart Game")) {
      int width, height;
      glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
      gameWorld.initialize(width, height);
    }
  }
}

void UIManager::renderSceneSelector(SceneManager &sceneManager) {
#if REMOVE_IMGUI == 0
  ImGui::Text("Scene Management:");

  // Get current scene name
  std::string currentSceneName = sceneManager.getCurrentSceneName();
  if (currentSceneName.empty()) {
    currentSceneName = "No Scene";
  }

  // Create a list of available scenes
  // Note: SceneManager doesn't expose loaded scene names, so we'll use known
  // scenes
  static std::vector<std::string> availableScenes = {
      "default", "level1", "level2", "sandbox", "custom"};

  // Find current scene index
  static int currentItem = 0;
  for (int i = 0; i < availableScenes.size(); i++) {
    if (availableScenes[i] == currentSceneName) {
      currentItem = i;
      break;
    }
  }

  // Create combo box for scene selection
  if (ImGui::BeginCombo("Current Scene", currentSceneName.c_str())) {
    for (int i = 0; i < availableScenes.size(); i++) {
      const bool isSelected = (currentItem == i);

      // Check if scene is available
      bool sceneAvailable = sceneManager.hasScene(availableScenes[i]);

      // Disable unavailable scenes
      if (!sceneAvailable) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
      }

      if (ImGui::Selectable(availableScenes[i].c_str(), isSelected)) {
        if (sceneAvailable && i != currentItem) {
          // Change to selected scene
          sceneManager.changeSceneInstant(availableScenes[i]);
          currentItem = i;
        }
      }

      if (!sceneAvailable) {
        ImGui::PopStyleVar();
        // Add tooltip for unavailable scenes
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Scene not loaded");
        }
      }

      // Set initial focus
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }

  // Scene transition status
  if (sceneManager.isTransitionInProgress()) {
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Transitioning...");
  }

  // Scene loading buttons
  ImGui::Text("Load Example Scenes:");

  if (ImGui::Button("Load Level 1")) {
    // This would need access to Application to load scenes
    // For now, just show a message
    ImGui::OpenPopup("Scene Loading");
  }
  ImGui::SameLine();

  if (ImGui::Button("Load Level 2")) {
    ImGui::OpenPopup("Scene Loading");
  }
  ImGui::SameLine();

  if (ImGui::Button("Load Sandbox")) {
    ImGui::OpenPopup("Scene Loading");
  }

  // Popup for scene loading instructions
  if (ImGui::BeginPopupModal("Scene Loading", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Scene loading from UI not fully implemented yet.");
    ImGui::Text("Use Application::loadScene() to load scene files.");
    ImGui::Separator();

    if (ImGui::Button("OK", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
#endif
}