#include "UIManager.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include "imguiThemes.h"
#include <GLFW/glfw3.h>

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