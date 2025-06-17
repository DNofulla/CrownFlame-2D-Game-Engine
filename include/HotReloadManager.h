#pragma once
#include "FileWatcher.h"
#include <memory>
#include <unordered_map>


// Forward declarations
class Application;
class SceneManager;
class AudioManager;

class HotReloadManager {
private:
  FileWatcher fileWatcher;
  Application *application;
  SceneManager *sceneManager;
  AudioManager *audioManager;

  // Asset registry to track loaded assets and their reload paths
  std::unordered_map<std::string, std::string>
      textureRegistry; // asset_id -> file_path
  std::unordered_map<std::string, std::string>
      sceneRegistry; // scene_name -> file_path
  std::unordered_map<std::string, std::string>
      audioRegistry; // sound_id -> file_path

  bool isEnabled;

public:
  HotReloadManager();
  ~HotReloadManager();

  // Lifecycle
  bool initialize(Application *app, SceneManager *sceneMgr,
                  AudioManager *audioMgr);
  void shutdown();

  // Asset registration - call these when loading assets to enable hot reloading
  void registerTexture(const std::string &assetId, const std::string &filePath);
  void registerScene(const std::string &sceneName, const std::string &filePath);
  void registerAudio(const std::string &soundId, const std::string &filePath);

  // Asset unregistration
  void unregisterTexture(const std::string &assetId);
  void unregisterScene(const std::string &sceneName);
  void unregisterAudio(const std::string &soundId);

  // Manual reload functions
  void reloadTexture(const std::string &filePath);
  void reloadScene(const std::string &filePath);
  void reloadAudio(const std::string &filePath);
  void reloadAllAssets();

  // Control
  void enable();
  void disable();
  bool getEnabled() const { return isEnabled; }
  void setWatchInterval(int milliseconds);

  // Statistics and debugging
  size_t getWatchedFileCount() const;
  std::vector<std::string> getWatchedFiles() const;
  void printStatus() const;

  // Auto-watch directory (watches all supported files in a directory)
  void watchDirectory(const std::string &directory, bool recursive = true);
  void unwatchDirectory(const std::string &directory);

private:
  // Callback functions for FileWatcher
  void onTextureChanged(const std::string &filePath);
  void onSceneChanged(const std::string &filePath);
  void onAudioChanged(const std::string &filePath);

  // Helper functions
  std::string getAssetIdFromPath(
      const std::string &filePath,
      const std::unordered_map<std::string, std::string> &registry) const;
  void scanDirectoryForAssets(const std::string &directory, bool recursive);
  bool isValidAssetFile(const std::string &filePath) const;
};