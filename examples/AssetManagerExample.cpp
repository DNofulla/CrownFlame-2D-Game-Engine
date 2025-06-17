// AssetManagerExample.cpp
// Example demonstrating how to use the AssetManager in CrownFlame 2D Game
// Engine

#include "Application.h"
#include "AssetManager.h"
#include <iostream>

void demonstrateAssetManager() {
  // Get the asset manager from the application
  Application app;
  if (!app.initialize()) {
    std::cerr << "Failed to initialize application!" << std::endl;
    return;
  }

  AssetManager &assetManager = app.getAssetManager();

  std::cout << "\n=== AssetManager Demo ===" << std::endl;

  // === TEXTURE LOADING ===
  std::cout << "\n--- Loading Textures ---" << std::endl;

  // Load textures (pixelated for pixel art games)
  if (assetManager.loadTexture(
          "pig_sprite", RESOURCES_PATH "textures/sprites/pig.png", true)) {
    std::cout << "✓ Loaded pig sprite" << std::endl;
  }

  // Load tile textures
  if (assetManager.loadTexture("grass_basic",
                               RESOURCES_PATH "textures/tiles/grass_basic.png",
                               true)) {
    std::cout << "✓ Loaded grass basic tile" << std::endl;
  }

  if (assetManager.loadTexture(
          "grass_flower", RESOURCES_PATH "textures/tiles/grass_flowers.png",
          true)) {
    std::cout << "✓ Loaded grass flower tile" << std::endl;
  }

  // === BATCH LOADING ===
  std::cout << "\n--- Batch Loading Tiles ---" << std::endl;

  // Load all tiles at once
  if (assetManager.loadAssetsFromDirectory(RESOURCES_PATH "textures/tiles/",
                                           AssetManager::AssetType::TEXTURE,
                                           false)) {
    std::cout << "✓ Batch loaded all tile textures" << std::endl;
  }

  // === AUDIO LOADING ===
  std::cout << "\n--- Loading Audio ---" << std::endl;

  if (assetManager.loadAudio("pickup_sound",
                             RESOURCES_PATH "audio/collectible_pickup.mp3")) {
    std::cout << "✓ Loaded pickup sound" << std::endl;
  }

  // === SCENE LOADING ===
  std::cout << "\n--- Loading Scenes ---" << std::endl;

  if (assetManager.loadScene("level1", RESOURCES_PATH "scenes/level1.scene")) {
    std::cout << "✓ Loaded level1 scene" << std::endl;
  }

  if (assetManager.loadScene("level2", RESOURCES_PATH "scenes/level2.scene")) {
    std::cout << "✓ Loaded level2 scene" << std::endl;
  }

  // === GETTING LOADED ASSETS ===
  std::cout << "\n--- Asset Information ---" << std::endl;

  // Get texture for rendering
  gl2d::Texture *pigTexture = assetManager.getTexture("pig_sprite");
  if (pigTexture) {
    auto size = pigTexture->GetSize();
    std::cout << "✓ Pig sprite size: " << size.x << "x" << size.y << std::endl;
  }

  // Check if assets are loaded
  std::cout << "Pig sprite loaded: "
            << (assetManager.isTextureLoaded("pig_sprite") ? "Yes" : "No")
            << std::endl;
  std::cout << "Pickup sound loaded: "
            << (assetManager.isAudioLoaded("pickup_sound") ? "Yes" : "No")
            << std::endl;
  std::cout << "Level1 scene loaded: "
            << (assetManager.isSceneLoaded("level1") ? "Yes" : "No")
            << std::endl;

  // === ASSET STATISTICS ===
  std::cout << "\n--- Asset Statistics ---" << std::endl;

  std::cout << "Total assets: " << assetManager.getAssetCount() << std::endl;
  std::cout << "Textures loaded: "
            << assetManager.getAssetCountByType(
                   AssetManager::AssetType::TEXTURE)
            << std::endl;
  std::cout << "Audio files loaded: "
            << assetManager.getAssetCountByType(AssetManager::AssetType::AUDIO)
            << std::endl;
  std::cout << "Scenes loaded: "
            << assetManager.getAssetCountByType(AssetManager::AssetType::SCENE)
            << std::endl;
  std::cout << "Total memory usage: " << assetManager.getTotalMemoryUsage()
            << " bytes" << std::endl;

  // === LIST ALL LOADED ASSETS ===
  std::cout << "\n--- Loaded Assets List ---" << std::endl;

  auto loadedAssets = assetManager.getLoadedAssets();
  for (const auto &asset : loadedAssets) {
    std::cout << "  " << AssetManager::getAssetTypeString(asset.type) << ": "
              << asset.name << " (" << asset.size << " bytes)" << std::endl;
  }

  // === ASSET VALIDATION ===
  std::cout << "\n--- Asset Validation ---" << std::endl;

  if (assetManager.validateAllAssets()) {
    std::cout << "✓ All assets are valid" << std::endl;
  } else {
    std::cout << "⚠ Some assets are missing or invalid" << std::endl;
    auto missingAssets = assetManager.getMissingAssets();
    for (const auto &missing : missingAssets) {
      std::cout << "  Missing: " << missing << std::endl;
    }
  }

  // === USING TEXTURES IN RENDERING ===
  std::cout << "\n--- Example Rendering Usage ---" << std::endl;

  gl2d::Texture *grassTexture = assetManager.getTexture("grass_basic");
  if (grassTexture) {
    std::cout << "✓ Ready to render grass texture" << std::endl;
    // In your render loop you would do:
    // renderer.renderRectangle({x, y, width, height}, *grassTexture);
  }

  // === CALLBACK EXAMPLE ===
  std::cout << "\n--- Setting up Callbacks ---" << std::endl;

  assetManager.setAssetLoadCallback(
      [](const std::string &name, AssetManager::AssetType type, bool success) {
        if (success) {
          std::cout << "✓ Asset loaded: " << name << " ("
                    << AssetManager::getAssetTypeString(type) << ")"
                    << std::endl;
        } else {
          std::cout << "✗ Failed to load asset: " << name << std::endl;
        }
      });

  // === HOT RELOAD EXAMPLE ===
  std::cout << "\n--- Hot Reload Setup ---" << std::endl;

  assetManager.enableHotReload(true);
  std::cout << "✓ Hot reload enabled for assets" << std::endl;

  // In your game loop you would call:
  // assetManager.checkForAssetChanges();

  std::cout << "\n=== AssetManager Demo Complete ===" << std::endl;

  app.shutdown();
}

// Example of integrating AssetManager into a simple game loop
void gameLoopExample() {
  Application app;
  if (!app.initialize()) {
    return;
  }

  AssetManager &assetManager = app.getAssetManager();

  // Preload essential assets
  std::vector<std::string> essentialAssets = {"pig_sprite", "grass_basic",
                                              "pickup_sound"};
  assetManager.preloadAssets(essentialAssets);

  // Example game loop (simplified)
  /*
  while (gameRunning) {
      // Check for asset changes if hot reload is enabled
      assetManager.checkForAssetChanges();

      // Get textures for rendering
      gl2d::Texture* playerTexture = assetManager.getTexture("pig_sprite");
      gl2d::Texture* groundTexture = assetManager.getTexture("grass_basic");

      // Use textures in rendering
      if (playerTexture) {
          renderer.renderRectangle(playerPosition, *playerTexture);
      }

      if (groundTexture) {
          for (auto& tile : groundTiles) {
              renderer.renderRectangle(tile.position, *groundTexture);
          }
      }

      // ... rest of game loop
  }
  */

  app.shutdown();
}

int main() {
  std::cout << "CrownFlame 2D Game Engine - AssetManager Example" << std::endl;

  // Run the asset manager demonstration
  demonstrateAssetManager();

  return 0;
}