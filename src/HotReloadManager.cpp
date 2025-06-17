#include "HotReloadManager.h"
#include "Application.h"
#include "AudioManager.h"
#include "SceneManager.h"
#include <algorithm>
#include <filesystem>
#include <iostream>


HotReloadManager::HotReloadManager()
    : application(nullptr), sceneManager(nullptr), audioManager(nullptr),
      isEnabled(false) {}

HotReloadManager::~HotReloadManager() { shutdown(); }

bool HotReloadManager::initialize(Application *app, SceneManager *sceneMgr,
                                  AudioManager *audioMgr) {
  if (!app || !sceneMgr || !audioMgr) {
    std::cerr << "HotReloadManager: Null pointer passed to initialize!"
              << std::endl;
    return false;
  }

  application = app;
  sceneManager = sceneMgr;
  audioManager = audioMgr;

  if (!fileWatcher.initialize(app)) {
    std::cerr << "HotReloadManager: Failed to initialize file watcher!"
              << std::endl;
    return false;
  }

  isEnabled = true;

  std::cout << "HotReloadManager: Initialized successfully" << std::endl;
  return true;
}

void HotReloadManager::shutdown() {
  disable();
  fileWatcher.shutdown();

  textureRegistry.clear();
  sceneRegistry.clear();
  audioRegistry.clear();

  application = nullptr;
  sceneManager = nullptr;
  audioManager = nullptr;

  std::cout << "HotReloadManager: Shutdown complete" << std::endl;
}

void HotReloadManager::registerScene(const std::string &sceneName,
                                     const std::string &filePath) {
  if (!isEnabled)
    return;

  sceneRegistry[sceneName] = filePath;

  fileWatcher.watchFile(
      filePath, FileType::SCENE,
      [this](const std::string &path) { onSceneChanged(path); });

  std::cout << "HotReloadManager: Registered scene " << sceneName << " -> "
            << filePath << std::endl;
}

void HotReloadManager::registerAudio(const std::string &soundId,
                                     const std::string &filePath) {
  if (!isEnabled)
    return;

  audioRegistry[soundId] = filePath;

  fileWatcher.watchFile(
      filePath, FileType::AUDIO,
      [this](const std::string &path) { onAudioChanged(path); });

  std::cout << "HotReloadManager: Registered audio " << soundId << " -> "
            << filePath << std::endl;
}

void HotReloadManager::reloadScene(const std::string &filePath) {
  std::cout << "HotReloadManager: Reloading scene " << filePath << std::endl;

  // Find the scene name for this file path
  std::string sceneName = getAssetIdFromPath(filePath, sceneRegistry);
  if (sceneName.empty()) {
    std::cout << "HotReloadManager: No registered scene found for path "
              << filePath << std::endl;
    return;
  }

  // Check if this is the currently active scene
  if (sceneManager->getCurrentScene() &&
      sceneManager->getCurrentScene()->getName() == sceneName) {
    std::cout << "HotReloadManager: Reloading current scene " << sceneName
              << std::endl;

    // Reload the scene from file
    if (sceneManager->loadSceneFromFile(sceneName, filePath)) {
      // If successful, restart the current scene to apply changes
      sceneManager->restartCurrentScene();
      std::cout << "HotReloadManager: Successfully reloaded current scene "
                << sceneName << std::endl;
    } else {
      std::cerr << "HotReloadManager: Failed to reload scene " << sceneName
                << std::endl;
    }
  } else {
    // Scene is not currently active, just reload it in memory
    std::cout << "HotReloadManager: Reloading inactive scene " << sceneName
              << std::endl;
    sceneManager->unloadScene(sceneName);
    if (sceneManager->loadSceneFromFile(sceneName, filePath)) {
      std::cout << "HotReloadManager: Successfully reloaded inactive scene "
                << sceneName << std::endl;
    } else {
      std::cerr << "HotReloadManager: Failed to reload scene " << sceneName
                << std::endl;
    }
  }
}

void HotReloadManager::reloadAudio(const std::string &filePath) {
  std::cout << "HotReloadManager: Reloading audio " << filePath << std::endl;

  // Find the sound ID for this file path
  std::string soundId = getAssetIdFromPath(filePath, audioRegistry);
  if (soundId.empty()) {
    std::cout << "HotReloadManager: No registered audio found for path "
              << filePath << std::endl;
    return;
  }

  // Reload the audio file
  if (audioManager->loadSound(soundId, filePath)) {
    std::cout << "HotReloadManager: Successfully reloaded audio " << soundId
              << std::endl;
  } else {
    std::cerr << "HotReloadManager: Failed to reload audio " << soundId
              << std::endl;
  }
}

void HotReloadManager::enable() {
  isEnabled = true;
  fileWatcher.setEnabled(true);
  std::cout << "HotReloadManager: Enabled" << std::endl;
}

void HotReloadManager::disable() {
  isEnabled = false;
  fileWatcher.setEnabled(false);
  std::cout << "HotReloadManager: Disabled" << std::endl;
}

size_t HotReloadManager::getWatchedFileCount() const {
  return fileWatcher.getWatchedFileCount();
}

std::vector<std::string> HotReloadManager::getWatchedFiles() const {
  return fileWatcher.getWatchedFiles();
}

void HotReloadManager::onSceneChanged(const std::string &filePath) {
  std::cout << "HotReloadManager: Scene file changed: " << filePath
            << std::endl;
  reloadScene(filePath);
}

void HotReloadManager::onAudioChanged(const std::string &filePath) {
  std::cout << "HotReloadManager: Audio file changed: " << filePath
            << std::endl;
  reloadAudio(filePath);
}

std::string HotReloadManager::getAssetIdFromPath(
    const std::string &filePath,
    const std::unordered_map<std::string, std::string> &registry) const {
  for (const auto &[id, path] : registry) {
    if (path == filePath) {
      return id;
    }
  }
  return "";
}

// Stub implementations for remaining methods
void HotReloadManager::registerTexture(const std::string &assetId,
                                       const std::string &filePath) {
  std::cout << "HotReloadManager: Texture hot reloading not yet implemented"
            << std::endl;
}

void HotReloadManager::unregisterTexture(const std::string &assetId) {}
void HotReloadManager::unregisterScene(const std::string &sceneName) {}
void HotReloadManager::unregisterAudio(const std::string &soundId) {}
void HotReloadManager::reloadTexture(const std::string &filePath) {}
void HotReloadManager::reloadAllAssets() {}
void HotReloadManager::setWatchInterval(int milliseconds) {}
void HotReloadManager::printStatus() const {}
void HotReloadManager::watchDirectory(const std::string &directory,
                                      bool recursive) {}
void HotReloadManager::unwatchDirectory(const std::string &directory) {}
void HotReloadManager::onTextureChanged(const std::string &filePath) {}
void HotReloadManager::scanDirectoryForAssets(const std::string &directory,
                                              bool recursive) {}
bool HotReloadManager::isValidAssetFile(const std::string &filePath) const {
  return false;
}