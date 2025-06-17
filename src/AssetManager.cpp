#include "AssetManager.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <raudio.h>
#include <sstream>


#ifdef _WIN32
#include <windows.h>
#endif

AssetManager::AssetManager()
    : m_initialized(false), m_hotReloadEnabled(false) {}

AssetManager::~AssetManager() { shutdown(); }

bool AssetManager::initialize() {
  if (m_initialized) {
    std::cout << "AssetManager: Already initialized." << std::endl;
    return true;
  }

  std::cout << "AssetManager: Initializing asset management system..."
            << std::endl;

  // Initialize GL2D if not already done
  if (!m_initialized) {
    m_initialized = true;
    std::cout << "AssetManager: Successfully initialized." << std::endl;

    // Auto-discover assets in the resources folder
    autoDiscoverAssets();

    return true;
  }

  return false;
}

void AssetManager::shutdown() {
  if (!m_initialized) {
    return;
  }

  std::cout << "AssetManager: Shutting down asset management system..."
            << std::endl;

  // Unload all assets
  unloadAllAssets();

  m_initialized = false;
  std::cout << "AssetManager: Asset management system shut down." << std::endl;
}

bool AssetManager::isInitialized() const { return m_initialized; }

// Texture Management
bool AssetManager::loadTexture(const std::string &name,
                               const std::string &filepath, bool pixelated,
                               bool useMipMaps) {
  if (!m_initialized) {
    std::cerr
        << "AssetManager: Cannot load texture - asset manager not initialized."
        << std::endl;
    return false;
  }

  return loadTextureInternal(name, filepath, pixelated, useMipMaps);
}

void AssetManager::unloadTexture(const std::string &name) {
  auto it = m_textures.find(name);
  if (it != m_textures.end()) {
    it->second->cleanup();
    m_textures.erase(it);
    unregisterAsset(name);
    std::cout << "AssetManager: Unloaded texture '" << name << "'."
              << std::endl;
  }
}

gl2d::Texture *AssetManager::getTexture(const std::string &name) {
  auto it = m_textures.find(name);
  if (it != m_textures.end()) {
    return it->second.get();
  }
  return nullptr;
}

bool AssetManager::isTextureLoaded(const std::string &name) const {
  return m_textures.find(name) != m_textures.end();
}

// Audio Management
bool AssetManager::loadAudio(const std::string &name,
                             const std::string &filepath) {
  if (!m_initialized) {
    std::cerr
        << "AssetManager: Cannot load audio - asset manager not initialized."
        << std::endl;
    return false;
  }

  // Check if audio already exists
  if (m_audioAssets.find(name) != m_audioAssets.end()) {
    std::cout << "AssetManager: Audio '" << name << "' already loaded."
              << std::endl;
    return true;
  }

  // Check if file exists
  if (!fileExists(filepath)) {
    std::cerr << "AssetManager: Audio file '" << filepath << "' does not exist."
              << std::endl;
    return false;
  }

  // Load the sound
  Sound loadedSound = LoadSound(filepath.c_str());

  if (loadedSound.sampleCount == 0) {
    std::cerr << "AssetManager: Failed to load audio '" << name << "' from '"
              << filepath << "'." << std::endl;
    return false;
  }

  // Store the sound
  auto soundPtr = std::make_unique<Sound>(loadedSound);
  m_audioAssets[name] = std::move(soundPtr);

  registerAsset(name, filepath, AssetType::AUDIO);

  std::cout << "AssetManager: Successfully loaded audio '" << name << "' from '"
            << filepath << "'." << std::endl;

  if (m_loadCallback) {
    m_loadCallback(name, AssetType::AUDIO, true);
  }

  return true;
}

void AssetManager::unloadAudio(const std::string &name) {
  auto it = m_audioAssets.find(name);
  if (it != m_audioAssets.end()) {
    UnloadSound(*(it->second));
    m_audioAssets.erase(it);
    unregisterAsset(name);
    std::cout << "AssetManager: Unloaded audio '" << name << "'." << std::endl;
  }
}

bool AssetManager::isAudioLoaded(const std::string &name) const {
  return m_audioAssets.find(name) != m_audioAssets.end();
}

// Scene Management
bool AssetManager::loadScene(const std::string &name,
                             const std::string &filepath) {
  if (!m_initialized) {
    std::cerr
        << "AssetManager: Cannot load scene - asset manager not initialized."
        << std::endl;
    return false;
  }

  return loadSceneInternal(name, filepath);
}

void AssetManager::unloadScene(const std::string &name) {
  auto it = m_sceneData.find(name);
  if (it != m_sceneData.end()) {
    m_sceneData.erase(it);
    unregisterAsset(name);
    std::cout << "AssetManager: Unloaded scene '" << name << "'." << std::endl;
  }
}

std::string *AssetManager::getSceneData(const std::string &name) {
  auto it = m_sceneData.find(name);
  if (it != m_sceneData.end()) {
    return &it->second;
  }
  return nullptr;
}

bool AssetManager::isSceneLoaded(const std::string &name) const {
  return m_sceneData.find(name) != m_sceneData.end();
}

// Font Management
bool AssetManager::loadFont(const std::string &name,
                            const std::string &filepath) {
  if (!m_initialized) {
    std::cerr
        << "AssetManager: Cannot load font - asset manager not initialized."
        << std::endl;
    return false;
  }

  return loadFontInternal(name, filepath);
}

void AssetManager::unloadFont(const std::string &name) {
  auto it = m_fonts.find(name);
  if (it != m_fonts.end()) {
    it->second->cleanup();
    m_fonts.erase(it);
    unregisterAsset(name);
    std::cout << "AssetManager: Unloaded font '" << name << "'." << std::endl;
  }
}

gl2d::Font *AssetManager::getFont(const std::string &name) {
  auto it = m_fonts.find(name);
  if (it != m_fonts.end()) {
    return it->second.get();
  }
  return nullptr;
}

bool AssetManager::isFontLoaded(const std::string &name) const {
  return m_fonts.find(name) != m_fonts.end();
}

// Batch Operations
bool AssetManager::loadAssetsFromDirectory(const std::string &directory,
                                           AssetType type, bool recursive) {
  if (!m_initialized) {
    std::cerr
        << "AssetManager: Cannot load assets - asset manager not initialized."
        << std::endl;
    return false;
  }

  auto assets = scanForAssets(directory, type, recursive);
  bool allSuccess = true;

  for (const auto &assetPath : assets) {
    std::string name = std::filesystem::path(assetPath).stem().string();

    switch (type) {
    case AssetType::TEXTURE:
      if (!loadTexture(name, assetPath)) {
        allSuccess = false;
      }
      break;
    case AssetType::AUDIO:
      if (!loadAudio(name, assetPath)) {
        allSuccess = false;
      }
      break;
    case AssetType::SCENE:
      if (!loadScene(name, assetPath)) {
        allSuccess = false;
      }
      break;
    case AssetType::FONT:
      if (!loadFont(name, assetPath)) {
        allSuccess = false;
      }
      break;
    }
  }

  return allSuccess;
}

void AssetManager::unloadAllAssets() {
  // Unload textures
  for (auto &pair : m_textures) {
    pair.second->cleanup();
  }
  m_textures.clear();

  // Unload audio
  for (auto &pair : m_audioAssets) {
    UnloadSound(*(pair.second));
  }
  m_audioAssets.clear();

  // Unload scenes
  m_sceneData.clear();

  // Unload fonts
  for (auto &pair : m_fonts) {
    pair.second->cleanup();
  }
  m_fonts.clear();

  // Clear registry
  m_assetRegistry.clear();

  std::cout << "AssetManager: Unloaded all assets." << std::endl;
}

void AssetManager::unloadAssetsByType(AssetType type) {
  std::vector<std::string> assetsToRemove;

  // Collect assets of the specified type
  for (const auto &pair : m_assetRegistry) {
    if (pair.second.type == type) {
      assetsToRemove.push_back(pair.first);
    }
  }

  // Unload them
  for (const std::string &name : assetsToRemove) {
    switch (type) {
    case AssetType::TEXTURE:
      unloadTexture(name);
      break;
    case AssetType::AUDIO:
      unloadAudio(name);
      break;
    case AssetType::SCENE:
      unloadScene(name);
      break;
    case AssetType::FONT:
      unloadFont(name);
      break;
    }
  }
}

// Asset Discovery
std::vector<std::string>
AssetManager::scanForAssets(const std::string &directory, AssetType type,
                            bool recursive) {
  std::vector<std::string> results;

  if (!std::filesystem::exists(directory)) {
    std::cerr << "AssetManager: Directory '" << directory << "' does not exist."
              << std::endl;
    return results;
  }

  if (recursive) {
    scanDirectoryRecursive(directory, type, results);
  } else {
    for (const auto &entry : std::filesystem::directory_iterator(directory)) {
      if (entry.is_regular_file() &&
          isValidAssetFile(entry.path().string(), type)) {
        results.push_back(entry.path().string());
      }
    }
  }

  return results;
}

void AssetManager::autoDiscoverAssets() {
  std::cout << "AssetManager: Auto-discovering assets in resources/ folder..."
            << std::endl;

  std::string resourcesPath = RESOURCES_PATH;

  // Scan for textures
  auto textures =
      scanForAssets(resourcesPath + "textures/", AssetType::TEXTURE, true);
  std::cout << "AssetManager: Found " << textures.size() << " texture files."
            << std::endl;

  // Scan for audio files
  auto audioFiles =
      scanForAssets(resourcesPath + "audio/", AssetType::AUDIO, true);
  std::cout << "AssetManager: Found " << audioFiles.size() << " audio files."
            << std::endl;

  // Scan for scene files
  auto sceneFiles =
      scanForAssets(resourcesPath + "scenes/", AssetType::SCENE, true);
  std::cout << "AssetManager: Found " << sceneFiles.size() << " scene files."
            << std::endl;

  // Register all discovered assets but don't load them yet
  for (const auto &texturePath : textures) {
    std::string name = std::filesystem::path(texturePath).stem().string();
    registerAsset(name, texturePath, AssetType::TEXTURE);
  }

  for (const auto &audioPath : audioFiles) {
    std::string name = std::filesystem::path(audioPath).stem().string();
    registerAsset(name, audioPath, AssetType::AUDIO);
  }

  for (const auto &scenePath : sceneFiles) {
    std::string name = std::filesystem::path(scenePath).stem().string();
    registerAsset(name, scenePath, AssetType::SCENE);
  }
}

// Asset Information
std::vector<AssetManager::AssetInfo> AssetManager::getLoadedAssets() const {
  std::vector<AssetInfo> result;
  for (const auto &pair : m_assetRegistry) {
    if (pair.second.isLoaded) {
      result.push_back(pair.second);
    }
  }
  return result;
}

std::vector<AssetManager::AssetInfo>
AssetManager::getAssetsByType(AssetType type) const {
  std::vector<AssetInfo> result;
  for (const auto &pair : m_assetRegistry) {
    if (pair.second.type == type) {
      result.push_back(pair.second);
    }
  }
  return result;
}

size_t AssetManager::getTotalMemoryUsage() const {
  size_t total = 0;
  for (const auto &pair : m_assetRegistry) {
    if (pair.second.isLoaded) {
      total += pair.second.size;
    }
  }
  return total;
}

size_t AssetManager::getAssetCount() const { return m_assetRegistry.size(); }

size_t AssetManager::getAssetCountByType(AssetType type) const {
  size_t count = 0;
  for (const auto &pair : m_assetRegistry) {
    if (pair.second.type == type) {
      count++;
    }
  }
  return count;
}

// Asset Validation
bool AssetManager::validateAsset(const std::string &name) {
  auto it = m_assetRegistry.find(name);
  if (it == m_assetRegistry.end()) {
    return false;
  }

  return fileExists(it->second.filepath);
}

bool AssetManager::validateAllAssets() {
  bool allValid = true;
  for (const auto &pair : m_assetRegistry) {
    if (!fileExists(pair.second.filepath)) {
      std::cerr << "AssetManager: Asset '" << pair.first
                << "' file missing: " << pair.second.filepath << std::endl;
      allValid = false;
    }
  }
  return allValid;
}

std::vector<std::string> AssetManager::getMissingAssets() const {
  std::vector<std::string> missing;
  for (const auto &pair : m_assetRegistry) {
    if (!fileExists(pair.second.filepath)) {
      missing.push_back(pair.first);
    }
  }
  return missing;
}

// Callbacks
void AssetManager::setAssetLoadCallback(AssetLoadCallback callback) {
  m_loadCallback = callback;
}

// Hot Reload Support
bool AssetManager::reloadAsset(const std::string &name) {
  auto it = m_assetRegistry.find(name);
  if (it == m_assetRegistry.end()) {
    std::cerr << "AssetManager: Asset '" << name << "' not found for reload."
              << std::endl;
    return false;
  }

  const AssetInfo &info = it->second;

  // Unload current asset
  switch (info.type) {
  case AssetType::TEXTURE:
    unloadTexture(name);
    return loadTexture(name, info.filepath);
  case AssetType::AUDIO:
    unloadAudio(name);
    return loadAudio(name, info.filepath);
  case AssetType::SCENE:
    unloadScene(name);
    return loadScene(name, info.filepath);
  case AssetType::FONT:
    unloadFont(name);
    return loadFont(name, info.filepath);
  }

  return false;
}

void AssetManager::enableHotReload(bool enable) {
  m_hotReloadEnabled = enable;
  std::cout << "AssetManager: Hot reload " << (enable ? "enabled" : "disabled")
            << "." << std::endl;
}

void AssetManager::checkForAssetChanges() {
  if (!m_hotReloadEnabled) {
    return;
  }

  // This would need to be implemented with file watching
  // For now, just a placeholder
}

// Preloading
void AssetManager::preloadAssets(const std::vector<std::string> &assetNames) {
  for (const std::string &name : assetNames) {
    auto it = m_assetRegistry.find(name);
    if (it != m_assetRegistry.end() && !it->second.isLoaded) {
      const AssetInfo &info = it->second;

      switch (info.type) {
      case AssetType::TEXTURE:
        loadTexture(name, info.filepath);
        break;
      case AssetType::AUDIO:
        loadAudio(name, info.filepath);
        break;
      case AssetType::SCENE:
        loadScene(name, info.filepath);
        break;
      case AssetType::FONT:
        loadFont(name, info.filepath);
        break;
      }
    }
  }
}

void AssetManager::setAssetPriority(const std::string &name, int priority) {
  // Placeholder for future priority system
  // Could be used for loading order optimization
}

// Utility Functions
std::string AssetManager::getAssetTypeString(AssetType type) {
  switch (type) {
  case AssetType::TEXTURE:
    return "Texture";
  case AssetType::AUDIO:
    return "Audio";
  case AssetType::SCENE:
    return "Scene";
  case AssetType::FONT:
    return "Font";
  default:
    return "Unknown";
  }
}

AssetManager::AssetType
AssetManager::getAssetTypeFromExtension(const std::string &extension) {
  std::string ext = extension;
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" ||
      ext == ".tga") {
    return AssetType::TEXTURE;
  } else if (ext == ".mp3" || ext == ".wav" || ext == ".ogg" ||
             ext == ".flac") {
    return AssetType::AUDIO;
  } else if (ext == ".scene") {
    return AssetType::SCENE;
  } else if (ext == ".ttf" || ext == ".otf") {
    return AssetType::FONT;
  }

  return AssetType::TEXTURE; // Default fallback
}

std::string AssetManager::getFileExtension(const std::string &filepath) {
  return std::filesystem::path(filepath).extension().string();
}

// Private Internal Methods
bool AssetManager::loadTextureInternal(const std::string &name,
                                       const std::string &filepath,
                                       bool pixelated, bool useMipMaps) {
  // Check if texture already exists
  if (m_textures.find(name) != m_textures.end()) {
    std::cout << "AssetManager: Texture '" << name << "' already loaded."
              << std::endl;
    return true;
  }

  // Check if file exists
  if (!fileExists(filepath)) {
    std::cerr << "AssetManager: Texture file '" << filepath
              << "' does not exist." << std::endl;
    return false;
  }

  // Create and load texture
  auto texture = std::make_unique<gl2d::Texture>();
  texture->loadFromFile(filepath.c_str(), pixelated, useMipMaps);

  if (texture->id == 0) {
    std::cerr << "AssetManager: Failed to load texture '" << name << "' from '"
              << filepath << "'." << std::endl;
    return false;
  }

  // Store the texture
  m_textures[name] = std::move(texture);

  registerAsset(name, filepath, AssetType::TEXTURE);

  std::cout << "AssetManager: Successfully loaded texture '" << name
            << "' from '" << filepath << "'." << std::endl;

  if (m_loadCallback) {
    m_loadCallback(name, AssetType::TEXTURE, true);
  }

  return true;
}

bool AssetManager::loadSceneInternal(const std::string &name,
                                     const std::string &filepath) {
  // Check if scene already exists
  if (m_sceneData.find(name) != m_sceneData.end()) {
    std::cout << "AssetManager: Scene '" << name << "' already loaded."
              << std::endl;
    return true;
  }

  // Read scene file
  std::string sceneContent = readFileToString(filepath);
  if (sceneContent.empty()) {
    std::cerr << "AssetManager: Failed to load scene '" << name << "' from '"
              << filepath << "'." << std::endl;
    return false;
  }

  // Store the scene data
  m_sceneData[name] = sceneContent;

  registerAsset(name, filepath, AssetType::SCENE);

  std::cout << "AssetManager: Successfully loaded scene '" << name << "' from '"
            << filepath << "'." << std::endl;

  if (m_loadCallback) {
    m_loadCallback(name, AssetType::SCENE, true);
  }

  return true;
}

bool AssetManager::loadFontInternal(const std::string &name,
                                    const std::string &filepath) {
  // Check if font already exists
  if (m_fonts.find(name) != m_fonts.end()) {
    std::cout << "AssetManager: Font '" << name << "' already loaded."
              << std::endl;
    return true;
  }

  // Check if file exists
  if (!fileExists(filepath)) {
    std::cerr << "AssetManager: Font file '" << filepath << "' does not exist."
              << std::endl;
    return false;
  }

  // Create and load font
  auto font = std::make_unique<gl2d::Font>();
  font->createFromFile(filepath.c_str());

  if (font->texture.id == 0) {
    std::cerr << "AssetManager: Failed to load font '" << name << "' from '"
              << filepath << "'." << std::endl;
    return false;
  }

  // Store the font
  m_fonts[name] = std::move(font);

  registerAsset(name, filepath, AssetType::FONT);

  std::cout << "AssetManager: Successfully loaded font '" << name << "' from '"
            << filepath << "'." << std::endl;

  if (m_loadCallback) {
    m_loadCallback(name, AssetType::FONT, true);
  }

  return true;
}

std::string AssetManager::readFileToString(const std::string &filepath) {
  std::ifstream file(filepath);
  if (!file.is_open()) {
    return "";
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

bool AssetManager::fileExists(const std::string &filepath) const {
  return std::filesystem::exists(filepath);
}

size_t AssetManager::getFileSize(const std::string &filepath) const {
  if (!fileExists(filepath)) {
    return 0;
  }
  return std::filesystem::file_size(filepath);
}

void AssetManager::registerAsset(const std::string &name,
                                 const std::string &filepath, AssetType type) {
  AssetInfo info;
  info.name = name;
  info.filepath = filepath;
  info.type = type;
  info.size = getFileSize(filepath);
  info.isLoaded = true;

  m_assetRegistry[name] = info;
}

void AssetManager::unregisterAsset(const std::string &name) {
  m_assetRegistry.erase(name);
}

void AssetManager::scanDirectoryRecursive(const std::string &directory,
                                          AssetType type,
                                          std::vector<std::string> &results) {
  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(directory)) {
    if (entry.is_regular_file() &&
        isValidAssetFile(entry.path().string(), type)) {
      results.push_back(entry.path().string());
    }
  }
}

bool AssetManager::isValidAssetFile(const std::string &filepath,
                                    AssetType type) const {
  std::string extension = getFileExtension(filepath);
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 ::tolower);

  switch (type) {
  case AssetType::TEXTURE:
    return extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
           extension == ".bmp" || extension == ".tga";
  case AssetType::AUDIO:
    return extension == ".mp3" || extension == ".wav" || extension == ".ogg" ||
           extension == ".flac";
  case AssetType::SCENE:
    return extension == ".scene";
  case AssetType::FONT:
    return extension == ".ttf" || extension == ".otf";
  }

  return false;
}

bool AssetManager::loadAssetManifest(const std::string &manifestPath) {
  // Placeholder for loading from a manifest file (JSON/XML)
  // This would define which assets to load at startup
  std::cout << "AssetManager: Asset manifest loading not yet implemented."
            << std::endl;
  return false;
}