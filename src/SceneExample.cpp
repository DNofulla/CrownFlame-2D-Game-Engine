/*
 * SceneExample.cpp
 *
 * This file demonstrates how to use the CrownFlame 2D Scene Management System.
 * You can use this as a reference for implementing scenes in your games.
 *
 * To use this example, modify main.cpp to call these functions instead of
 * the default Application setup.
 */

#include "Application.h"
#include "SceneData.h"
#include "SceneManager.h"
#include <iostream>

// Example function to create a custom scene programmatically
SceneData::SceneDefinition createCustomScene() {
  SceneData::SceneDefinition scene("Custom Scene");
  scene.description = "A scene created programmatically";
  scene.nextScene = "level1";
  scene.transitionTrigger = "collectibles_complete";

  // Setup world
  scene.world.width = 1800.0f;
  scene.world.height = 1200.0f;
  scene.world.backgroundMusic = ""; // Could be "resources/music/background.mp3"

  // Setup camera
  scene.camera.followSpeed = 7.0f;
  scene.camera.followEnabled = true;

  // Setup player spawn
  scene.playerSpawn.x = 200.0f;
  scene.playerSpawn.y = 200.0f;

  // Add obstacles
  scene.obstacles.emplace_back(400.0f, 300.0f, 100.0f, 100.0f);
  scene.obstacles.emplace_back(600.0f, 150.0f, 80.0f, 150.0f);
  scene.obstacles.emplace_back(300.0f, 500.0f, 120.0f, 60.0f);

  // Add collectibles
  scene.collectibles.emplace_back(350.0f, 250.0f);
  scene.collectibles.emplace_back(550.0f, 100.0f);
  scene.collectibles.emplace_back(450.0f, 450.0f);
  scene.collectibles.emplace_back(700.0f, 350.0f);

  // Add enemies with different patterns
  scene.enemies.emplace_back(500.0f, 400.0f, MovementPattern::HORIZONTAL,
                             100.0f);
  scene.enemies.emplace_back(250.0f, 350.0f, MovementPattern::VERTICAL, 80.0f);
  scene.enemies.emplace_back(650.0f, 250.0f, MovementPattern::CIRCULAR, 90.0f);

  return scene;
}

// Example function showing how to set up multiple scenes
void setupExampleScenes(Application &app) {
  std::cout << "Setting up example scenes..." << std::endl;

  // Load scenes from files
  if (app.loadScene("level1", RESOURCES_PATH "scenes/level1.scene")) {
    std::cout << "✓ Loaded Level 1 from file" << std::endl;
  } else {
    std::cerr << "✗ Failed to load Level 1" << std::endl;
  }

  if (app.loadScene("level2", RESOURCES_PATH "scenes/level2.scene")) {
    std::cout << "✓ Loaded Level 2 from file" << std::endl;
  } else {
    std::cerr << "✗ Failed to load Level 2" << std::endl;
  }

  if (app.loadScene("sandbox", RESOURCES_PATH "scenes/sandbox.scene")) {
    std::cout << "✓ Loaded Sandbox from file" << std::endl;
  } else {
    std::cerr << "✗ Failed to load Sandbox" << std::endl;
  }

  // Create and load a custom scene programmatically
  auto customScene = createCustomScene();
  if (app.loadSceneFromDefinition("custom", customScene)) {
    std::cout << "✓ Created Custom Scene programmatically" << std::endl;
  } else {
    std::cerr << "✗ Failed to create Custom Scene" << std::endl;
  }

  // Set up scene transition callbacks
  auto &sceneManager = app.getSceneManager();

  sceneManager.setOnSceneChangedCallback([](const std::string &sceneName) {
    std::cout << "🎬 Scene changed to: " << sceneName << std::endl;
  });

  sceneManager.setOnSceneTransitionStartedCallback(
      [](const std::string &from, const std::string &to) {
        std::cout << "🔄 Transitioning from '" << from << "' to '" << to << "'"
                  << std::endl;
      });

  std::cout << "Scene setup complete!" << std::endl;
}

// Example function showing how to change scenes with transitions
void demonstrateSceneTransitions(Application &app) {
  auto &sceneManager = app.getSceneManager();

  std::cout << "\n=== Scene Transition Examples ===" << std::endl;

  // Instant transition to level1
  if (sceneManager.hasScene("level1")) {
    sceneManager.changeSceneInstant("level1");
    std::cout << "Changed to Level 1 (instant)" << std::endl;
  }

  // You could add timer-based transitions in a real application:
  /*
  // After some time, transition with fade effect
  SceneData::SceneTransition
  fadeTransition(SceneData::TransitionType::FADE_TO_BLACK, 2.0f);
  sceneManager.changeScene("level2", fadeTransition);

  // Or slide transition
  SceneData::SceneTransition
  slideTransition(SceneData::TransitionType::SLIDE_LEFT, 1.5f);
  sceneManager.changeScene("custom", slideTransition);
  */
}

// Example function showing how to save and create scene files
void demonstrateSceneSaving(Application &app) {
  auto &sceneManager = app.getSceneManager();

  std::cout << "\n=== Scene Saving Examples ===" << std::endl;

  // Save the custom scene to a file
  if (sceneManager.hasScene("custom")) {
    if (sceneManager.saveSceneToFile("custom", RESOURCES_PATH
                                     "scenes/custom_saved.scene")) {
      std::cout << "✓ Saved custom scene to file" << std::endl;
    } else {
      std::cerr << "✗ Failed to save custom scene" << std::endl;
    }
  }

  // Create a default scene and save it
  auto defaultScene = SceneManager::createDefaultScene("Generated Default");
  if (app.loadSceneFromDefinition("generated", defaultScene)) {
    if (sceneManager.saveSceneToFile("generated",
                                     RESOURCES_PATH "scenes/generated.scene")) {
      std::cout << "✓ Created and saved generated scene" << std::endl;
    }
  }
}

// Example usage in main.cpp:
/*
int main() {
    Application app;

    if (!app.initialize(1024, 768, "CrownFlame 2D - Scene System Demo")) {
        return -1;
    }

    // Setup all example scenes
    setupExampleScenes(app);

    // Demonstrate transitions
    demonstrateSceneTransitions(app);

    // Demonstrate saving
    demonstrateSceneSaving(app);

    // Run the application
    app.run();

    return 0;
}
*/