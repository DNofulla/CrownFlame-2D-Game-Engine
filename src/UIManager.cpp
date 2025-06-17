#include "UIManager.h"
#include "FileBrowser.h"
#include "HotReloadManager.h"
#include "Scene.h"
#include "SceneManager.h"
#include "SceneTemplates.h"
#include "SceneValidator.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include "imguiThemes.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

UIManager::UIManager()
    : initialized(false), selectedTemplateIndex(0), showTemplateCreator(false),
      showFileBrowser(false), showSceneInfo(false),
      showValidationResults(false), currentValidationResult(nullptr),
      needsSceneListRefresh(true) {}

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

  // Scene selector and new features
  renderSceneSelector(sceneManager);
  renderSceneInformation(sceneManager);
  ImGui::Separator();

  // Scene management buttons
  if (ImGui::Button("📁 File Browser")) {
    showFileBrowser = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("📋 Templates")) {
    showTemplateCreator = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("ℹ️ Scene Info")) {
    showSceneInfo = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("✅ Validate")) {
    validateCurrentScene(sceneManager);
  }

  // Render feature windows
  if (showFileBrowser) {
    renderFileBrowser(sceneManager);
  }
  if (showTemplateCreator) {
    renderTemplateCreator(sceneManager);
  }
  if (showValidationResults) {
    renderValidationResults();
  }

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

  // Debug output (can be removed later)
  static std::string lastSceneName = "";
  if (lastSceneName != currentSceneName) {
    std::cout << "Current scene changed to: " << currentSceneName << std::endl;
    lastSceneName = currentSceneName;
  }

  // Dynamically get available scene files
  static std::vector<std::string> availableScenes;

  // Refresh scene list button
  if (ImGui::Button("🔄 Refresh")) {
    needsSceneListRefresh = true;
  }
  ImGui::SameLine();

  // Refresh scene list when needed
  if (needsSceneListRefresh) {
    availableScenes.clear();

    // Get scene files from the scenes directory
    std::string sceneDir = FileBrowser::getSceneDirectory();
    auto sceneFiles = FileBrowser::listSceneFiles(sceneDir);

    for (const auto &fileInfo : sceneFiles) {
      // Extract scene name without extension
      std::string sceneName = fileInfo.filename;
      size_t dotPos = sceneName.find_last_of('.');
      if (dotPos != std::string::npos) {
        sceneName = sceneName.substr(0, dotPos);
      }
      availableScenes.push_back(sceneName);

      // Auto-load scene if not already loaded
      if (!sceneManager.hasScene(sceneName)) {
        std::cout << "Auto-loading scene: " << sceneName << " from "
                  << fileInfo.fullPath << std::endl;
        sceneManager.loadSceneFromFile(sceneName, fileInfo.fullPath);
      }
    }

    // Add default scenes if they don't exist in files
    std::vector<std::string> defaultScenes = {"default", "level1", "level2",
                                              "sandbox"};
    for (const auto &defaultScene : defaultScenes) {
      bool found = false;
      for (const auto &existing : availableScenes) {
        if (existing == defaultScene) {
          found = true;
          break;
        }
      }
      if (!found) {
        availableScenes.push_back(defaultScene);
      }
    }

    needsSceneListRefresh = false;
  }

  // Find current scene index
  static int currentItem = 0;
  bool foundCurrentScene = false;
  for (int i = 0; i < availableScenes.size(); i++) {
    if (availableScenes[i] == currentSceneName) {
      currentItem = i;
      foundCurrentScene = true;
      break;
    }
  }

  // If current scene not found in list, reset to 0
  if (!foundCurrentScene && !availableScenes.empty()) {
    currentItem = 0;
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
        if (i != currentItem) {
          if (sceneAvailable) {
            // Change to selected scene
            std::cout << "Switching to scene: " << availableScenes[i]
                      << std::endl;
            sceneManager.changeSceneInstant(availableScenes[i]);
            currentItem = i;
          } else {
            // Try to load the scene first, then switch
            std::cout << "Scene not loaded, attempting to load: "
                      << availableScenes[i] << std::endl;
            std::string sceneDir = FileBrowser::getSceneDirectory();
            std::string sceneFile =
                FileBrowser::joinPaths(sceneDir, availableScenes[i] + ".scene");

            if (FileBrowser::fileExists(sceneFile)) {
              std::cout << "Loading scene file: " << sceneFile << std::endl;
              sceneManager.loadSceneFromFile(availableScenes[i], sceneFile);
              sceneManager.changeSceneInstant(availableScenes[i]);
              currentItem = i;
            } else {
              std::cout << "Scene file not found: " << sceneFile << std::endl;
            }
          }
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

// ============================================================================
// NEW FEATURE IMPLEMENTATIONS
// ============================================================================

void UIManager::renderSceneInformation(SceneManager &sceneManager) {
#if REMOVE_IMGUI == 0
  if (!showSceneInfo)
    return;

  Scene *currentScene = sceneManager.getCurrentScene();
  if (!currentScene)
    return;

  ImGui::Begin("Scene Information", &showSceneInfo);

  ImGui::Text("📋 Scene Details");
  ImGui::Separator();

  ImGui::Text("Name: %s", currentScene->getName().c_str());
  ImGui::Text("Description: %s", currentScene->getDescription().c_str());
  ImGui::Text("World Size: %.0fx%.0f", currentScene->getWorldWidth(),
              currentScene->getWorldHeight());
  ImGui::Text("Transition: %s", currentScene->getTransitionTrigger().c_str());

  ImGui::Separator();
  ImGui::Text("📊 Object Count");
  ImGui::Text("Total Objects: %d", currentScene->getObjectCount());
  ImGui::Text("Obstacles: %d", currentScene->getObstacleCount());
  ImGui::Text("Collectibles: %d", currentScene->getCollectibleCount());
  ImGui::Text("Enemies: %d", currentScene->getEnemyCount());

  ImGui::Separator();
  if (ImGui::Button("Validate Scene")) {
    validateCurrentScene(sceneManager);
  }

  ImGui::End();
#endif
}

void UIManager::renderTemplateCreator(SceneManager &sceneManager) {
#if REMOVE_IMGUI == 0
  if (!showTemplateCreator)
    return;

  ImGui::Begin("Scene Templates", &showTemplateCreator);

  ImGui::Text("🎨 Create Scene from Template");
  ImGui::Separator();

  // Get available templates
  static auto templates = SceneTemplates::getAvailableTemplates();
  static char sceneName[256] = "New Scene";

  ImGui::InputText("Scene Name", sceneName, sizeof(sceneName));
  ImGui::Separator();

  // Template selection
  ImGui::Text("Select Template:");

  for (int i = 0; i < templates.size(); i++) {
    if (ImGui::Selectable(templates[i].name.c_str(),
                          selectedTemplateIndex == i)) {
      selectedTemplateIndex = i;
      selectedTemplateName = templates[i].name;
    }

    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("%s", templates[i].description.c_str());
    }
  }

  ImGui::Separator();

  if (selectedTemplateIndex < templates.size()) {
    ImGui::Text("Selected: %s", templates[selectedTemplateIndex].name.c_str());
    ImGui::TextWrapped("%s",
                       templates[selectedTemplateIndex].description.c_str());

    ImGui::Separator();

    if (ImGui::Button("Create Scene")) {
      createSceneFromTemplate(sceneManager);
    }
    ImGui::SameLine();
    if (ImGui::Button("Create & Save")) {
      createSceneFromTemplate(sceneManager);
      saveFileDialog(sceneManager);
    }
  }

  ImGui::End();
#endif
}

void UIManager::renderFileBrowser(SceneManager &sceneManager) {
#if REMOVE_IMGUI == 0
  if (!showFileBrowser)
    return;

  ImGui::Begin("File Browser", &showFileBrowser);

  ImGui::Text("📁 Scene File Browser");
  ImGui::Separator();

  // Quick actions
  if (ImGui::Button("📂 Open Scene...")) {
    openFileDialog(sceneManager);
  }
  ImGui::SameLine();
  if (ImGui::Button("💾 Save Scene As...")) {
    saveFileDialog(sceneManager);
  }

  ImGui::Separator();

  // Scene directory browser
  ImGui::Text("📂 Scenes Directory:");
  std::string sceneDir = FileBrowser::getSceneDirectory();
  ImGui::Text("%s", sceneDir.c_str());

  ImGui::Separator();

  // List scene files
  static auto sceneFiles = FileBrowser::listSceneFiles(sceneDir);
  static float lastRefresh = 0.0f;

  // Refresh button and auto-refresh
  if (ImGui::Button("🔄 Refresh") || ImGui::GetTime() - lastRefresh > 5.0f) {
    sceneFiles = FileBrowser::listSceneFiles(sceneDir);
    lastRefresh = ImGui::GetTime();
  }

  ImGui::SameLine();
  ImGui::Text("(%d scene files)", (int)sceneFiles.size());

  ImGui::Separator();

  // File list
  if (sceneFiles.empty()) {
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No scene files found");
  } else {
    ImGui::Text("Scene Files:");
    for (const auto &file : sceneFiles) {
      ImGui::PushID(file.filename.c_str());

      if (ImGui::Selectable(file.filename.c_str())) {
        // Load the selected scene
        std::string sceneName = FileBrowser::getFileName(file.fullPath);
        // Remove .scene extension for scene name
        if (sceneName.length() > 6 &&
            sceneName.substr(sceneName.length() - 6) == ".scene") {
          sceneName = sceneName.substr(0, sceneName.length() - 6);
        }
        sceneManager.loadSceneFromFile(sceneName, file.fullPath);
        sceneManager.changeSceneInstant(
            sceneName);     // Switch to the loaded scene
        refreshSceneList(); // Refresh the scene list after loading
      }

      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Path: %s\nSize: %s\nClick to load",
                          file.fullPath.c_str(),
                          FileBrowser::formatFileSize(file.fileSize).c_str());
      }

      ImGui::PopID();
    }
  }

  ImGui::End();
#endif
}

void UIManager::renderValidationResults() {
#if REMOVE_IMGUI == 0
  if (!showValidationResults || !currentValidationResult)
    return;

  ImGui::Begin("Scene Validation Results", &showValidationResults);

  // Summary
  if (currentValidationResult->isValid) {
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✅ Scene is valid!");
  } else {
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "❌ Scene has issues");
  }

  ImGui::Text("Errors: %d", currentValidationResult->getErrorCount());
  ImGui::Text("Warnings: %d", currentValidationResult->getWarningCount());

  ImGui::Separator();

  // Issue list
  if (!currentValidationResult->issues.empty()) {
    ImGui::Text("Issues Found:");

    for (const auto &issue : currentValidationResult->issues) {
      ImVec4 color = (issue.severity == SceneValidationIssue::Severity::ERROR)
                         ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f)
                         : ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

      const char *icon =
          (issue.severity == SceneValidationIssue::Severity::ERROR) ? "❌"
                                                                    : "⚠️";

      ImGui::TextColored(color, "%s %s", icon, issue.message.c_str());

      if (!issue.location.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(%s)",
                           issue.location.c_str());
      }
    }
  }

  ImGui::Separator();

  if (ImGui::Button("Re-validate")) {
    // The current scene will be re-validated, but we need the scene manager
    // This is a bit of a limitation of the current design
    showValidationResults = false;
  }

  ImGui::End();
#endif
}

// ============================================================================
// UTILITY METHODS
// ============================================================================

void UIManager::openFileDialog(SceneManager &sceneManager) {
  std::string sceneDir = FileBrowser::getSceneDirectory();
  std::string filepath =
      FileBrowser::openFileDialog("Open Scene File", sceneDir);

  if (!filepath.empty()) {
    if (FileBrowser::fileExists(filepath)) {
      std::string sceneName = FileBrowser::getFileName(filepath);
      // Remove .scene extension for scene name
      if (sceneName.length() > 6 &&
          sceneName.substr(sceneName.length() - 6) == ".scene") {
        sceneName = sceneName.substr(0, sceneName.length() - 6);
      }
      sceneManager.loadSceneFromFile(sceneName, filepath);
      sceneManager.changeSceneInstant(sceneName); // Switch to the loaded scene
      refreshSceneList(); // Refresh the scene list after loading
      std::cout << "Loading scene: " << filepath << std::endl;
    } else {
      std::cerr << "File does not exist: " << filepath << std::endl;
    }
  }
}

void UIManager::saveFileDialog(SceneManager &sceneManager) {
  Scene *currentScene = sceneManager.getCurrentScene();
  if (!currentScene) {
    std::cerr << "No current scene to save" << std::endl;
    return;
  }

  std::string sceneDir = FileBrowser::getSceneDirectory();
  std::string defaultName = currentScene->getName() + ".scene";
  std::string filepath =
      FileBrowser::saveFileDialog("Save Scene As", sceneDir, defaultName);

  if (!filepath.empty()) {
    // Ensure .scene extension
    if (FileBrowser::getFileExtension(filepath) != "scene") {
      filepath += ".scene";
    }

    sceneManager.saveSceneToFile(currentScene->getName(), filepath);
    refreshSceneList(); // Refresh the scene list after saving
    std::cout << "Saving scene: " << filepath << std::endl;
  }
}

void UIManager::createSceneFromTemplate(SceneManager &sceneManager) {
  if (selectedTemplateIndex >= SceneTemplates::getAvailableTemplates().size()) {
    return;
  }

  auto templates = SceneTemplates::getAvailableTemplates();
  auto templateType = templates[selectedTemplateIndex].type;

  // Create scene from template
  auto sceneDefinition =
      SceneTemplates::createFromTemplate(templateType, selectedTemplateName);

  // Load the scene into the scene manager
  sceneManager.loadSceneFromDefinition("template_scene", sceneDefinition);
  sceneManager.changeSceneInstant("template_scene");
  refreshSceneList(); // Refresh the scene list after creating from template

  std::cout << "Created scene from template: " << selectedTemplateName
            << std::endl;
}

void UIManager::validateCurrentScene(SceneManager &sceneManager) {
  Scene *currentScene = sceneManager.getCurrentScene();
  if (!currentScene) {
    std::cerr << "No current scene to validate" << std::endl;
    return;
  }

  // Get scene definition (this would need to be exposed by Scene class)
  // For now, we'll create a basic validation
  // This is a limitation - we'd need Scene to expose its definition

  // Create a temporary validation result
  static SceneValidationResult validationResult;
  validationResult = SceneValidationResult(); // Reset

  // Basic validation checks we can do with current Scene interface
  if (currentScene->getName().empty()) {
    validationResult.addError("Scene name is empty");
  }

  if (currentScene->getObjectCount() == 0) {
    validationResult.addWarning("Scene has no objects");
  }

  if (currentScene->getCollectibleCount() == 0 &&
      currentScene->getTransitionTrigger() == "collectibles_complete") {
    validationResult.addError(
        "Scene completion requires collectibles but none are found");
  }

  if (currentScene->getEnemyCount() == 0 &&
      currentScene->getTransitionTrigger() == "enemies_defeat") {
    validationResult.addError(
        "Scene completion requires defeating enemies but none are found");
  }

  // Store result and show UI
  currentValidationResult = &validationResult;
  showValidationResults = true;

  std::cout << "Scene validation completed: "
            << (validationResult.isValid ? "VALID" : "INVALID") << std::endl;
}

void UIManager::refreshSceneList() { needsSceneListRefresh = true; }

// New overload with hot reload manager
void UIManager::renderGameUI(GameWorld &gameWorld, const FPSCounter &fpsCounter,
                             float &playerSpeed, SceneManager &sceneManager,
                             HotReloadManager &hotReloadManager) {
#if REMOVE_IMGUI == 0
  ImGui::Begin("Game Controls");

  // Display FPS
  ImGui::Text("FPS: %.1f", fpsCounter.getFPS());
  ImGui::Separator();

  // Hot Reload Controls
  renderHotReloadControls(hotReloadManager);
  ImGui::Separator();

  // Scene selector and new features
  renderSceneSelector(sceneManager);
  renderSceneInformation(sceneManager);
  ImGui::Separator();

  // Scene management buttons
  if (ImGui::Button("📁 File Browser")) {
    showFileBrowser = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("📋 Templates")) {
    showTemplateCreator = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("ℹ️ Scene Info")) {
    showSceneInfo = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("✅ Validate")) {
    validateCurrentScene(sceneManager);
  }

  // Render feature windows
  if (showFileBrowser) {
    renderFileBrowser(sceneManager);
  }
  if (showTemplateCreator) {
    renderTemplateCreator(sceneManager);
  }
  if (showValidationResults) {
    renderValidationResults();
  }

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

void UIManager::renderHotReloadControls(HotReloadManager &hotReloadManager) {
#if REMOVE_IMGUI == 0
  if (ImGui::CollapsingHeader("🔥 Hot Reload",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    bool isEnabled = hotReloadManager.getEnabled();
    if (ImGui::Checkbox("Enable Hot Reloading", &isEnabled)) {
      if (isEnabled) {
        hotReloadManager.enable();
      } else {
        hotReloadManager.disable();
      }
    }

    if (isEnabled) {
      ImGui::Text("Watched Files: %zu", hotReloadManager.getWatchedFileCount());

      if (ImGui::Button("🔄 Reload All Assets")) {
        hotReloadManager.reloadAllAssets();
      }

      ImGui::SameLine();
      if (ImGui::Button("📊 Show Status")) {
        hotReloadManager.printStatus();
      }

      // Show watched files in a scrollable list
      if (ImGui::TreeNode("Watched Files")) {
        auto watchedFiles = hotReloadManager.getWatchedFiles();
        if (watchedFiles.empty()) {
          ImGui::Text("No files being watched");
        } else {
          ImGui::BeginChild("WatchedFilesList", ImVec2(0, 100), true);
          for (const auto &file : watchedFiles) {
            // Extract just the filename for display
            size_t lastSlash = file.find_last_of("/\\");
            std::string filename = (lastSlash != std::string::npos)
                                       ? file.substr(lastSlash + 1)
                                       : file;

            ImGui::Text("📄 %s", filename.c_str());
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("Full path: %s", file.c_str());
            }
          }
          ImGui::EndChild();
        }
        ImGui::TreePop();
      }

      ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                         "Note: Files are monitored for changes every 500ms");
    } else {
      ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                         "Hot reloading is disabled");
    }
  }
#endif
}