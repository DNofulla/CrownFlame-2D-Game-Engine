#pragma once

#include <functional>
#include <gl2d/gl2d.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


// Forward declarations
struct Sound;
namespace SceneData {
struct SceneDefinition;
}

class AssetManager {
public:
  // Asset type enumeration for categorization
  enum class AssetType { TEXTURE, AUDIO, SCENE, FONT };

  // Asset info structure
  struct AssetInfo {
    std::string name;
    std::string filepath;
    AssetType type;
    size_t size = 0;
    bool isLoaded = false;
  };

  AssetManager();
  ~AssetManager();

  // Asset Manager lifecycle
  bool initialize();
  void shutdown();
  bool isInitialized() const;

  // Texture management
  bool loadTexture(const std::string &name, const std::string &filepath,
                   bool pixelated = false, bool useMipMaps = true);
  void unloadTexture(const std::string &name);
  gl2d::Texture *getTexture(const std::string &name);
  bool isTextureLoaded(const std::string &name) const;

  // Audio management (integrates with AudioManager)
  bool loadAudio(const std::string &name, const std::string &filepath);
  void unloadAudio(const std::string &name);
  bool isAudioLoaded(const std::string &name) const;

  // Scene management
  bool loadScene(const std::string &name, const std::string &filepath);
  void unloadScene(const std::string &name);
  std::string *getSceneData(const std::string &name);
  bool isSceneLoaded(const std::string &name) const;

  // Font management
  bool loadFont(const std::string &name, const std::string &filepath);
  void unloadFont(const std::string &name);
  gl2d::Font *getFont(const std::string &name);
  bool isFontLoaded(const std::string &name) const;

  // Batch operations
  bool loadAssetsFromDirectory(const std::string &directory, AssetType type,
                               bool recursive = false);
  bool loadAssetManifest(const std::string &manifestPath);
  void unloadAllAssets();
  void unloadAssetsByType(AssetType type);

  // Asset discovery and scanning
  std::vector<std::string> scanForAssets(const std::string &directory,
                                         AssetType type,
                                         bool recursive = false);
  void autoDiscoverAssets(); // Scans resources/ folder automatically

  // Asset information and statistics
  std::vector<AssetInfo> getLoadedAssets() const;
  std::vector<AssetInfo> getAssetsByType(AssetType type) const;
  size_t getTotalMemoryUsage() const;
  size_t getAssetCount() const;
  size_t getAssetCountByType(AssetType type) const;

  // Asset validation and integrity
  bool validateAsset(const std::string &name);
  bool validateAllAssets();
  std::vector<std::string> getMissingAssets() const;

  // Callbacks for asset loading events
  using AssetLoadCallback = std::function<void(const std::string &name,
                                               AssetType type, bool success)>;
  void setAssetLoadCallback(AssetLoadCallback callback);

  // Hot reload support
  bool reloadAsset(const std::string &name);
  void enableHotReload(bool enable);
  void checkForAssetChanges(); // Call this in your update loop if hot reload is
                               // enabled

  // Asset preloading and streaming
  void preloadAssets(const std::vector<std::string> &assetNames);
  void setAssetPriority(const std::string &name, int priority);

  // Utility functions
  static std::string getAssetTypeString(AssetType type);
  static AssetType getAssetTypeFromExtension(const std::string &extension);
  static std::string getFileExtension(const std::string &filepath);

private:
  // Internal asset containers
  std::unordered_map<std::string, std::unique_ptr<gl2d::Texture>> m_textures;
  std::unordered_map<std::string, std::unique_ptr<Sound>> m_audioAssets;
  std::unordered_map<std::string, std::string> m_sceneData;
  std::unordered_map<std::string, std::unique_ptr<gl2d::Font>> m_fonts;

  // Asset metadata
  std::unordered_map<std::string, AssetInfo> m_assetRegistry;

  // Manager state
  bool m_initialized;
  AssetLoadCallback m_loadCallback;
  bool m_hotReloadEnabled;

  // Internal helper methods
  bool loadTextureInternal(const std::string &name, const std::string &filepath,
                           bool pixelated, bool useMipMaps);
  bool loadSceneInternal(const std::string &name, const std::string &filepath);
  bool loadFontInternal(const std::string &name, const std::string &filepath);

  std::string readFileToString(const std::string &filepath);
  bool fileExists(const std::string &filepath) const;
  size_t getFileSize(const std::string &filepath) const;

  void registerAsset(const std::string &name, const std::string &filepath,
                     AssetType type);
  void unregisterAsset(const std::string &name);

  // Directory scanning helpers
  void scanDirectoryRecursive(const std::string &directory, AssetType type,
                              std::vector<std::string> &results);
  bool isValidAssetFile(const std::string &filepath, AssetType type) const;

  // Disable copy constructor and assignment operator
  AssetManager(const AssetManager &) = delete;
  AssetManager &operator=(const AssetManager &) = delete;
};