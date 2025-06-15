#include "TileMapManager.h"
#include <iostream>

TileMapManager::TileMapManager() : currentMap(nullptr) {}

TileMapManager::~TileMapManager() { clearAll(); }

bool TileMapManager::loadTileset(const std::string &name,
                                 const std::string &imagePath, int tileWidth,
                                 int tileHeight) {
  // This method is now deprecated since we load individual tile images
  // For backward compatibility, we'll just create an empty tileset
  auto tileset = std::make_unique<Tileset>();
  tileset->setName(name);
  tilesets[name] = std::move(tileset);
  std::cout << "Created empty tileset: " << name
            << " (individual tile loading mode)" << std::endl;
  return true;
}

bool TileMapManager::loadGrassTileset() {
  auto tileset = std::make_unique<Tileset>();
  if (!tileset->loadGrassTileset()) {
    std::cerr << "Failed to load grass tileset" << std::endl;
    return false;
  }

  tilesets["grass"] = std::move(tileset);
  std::cout << "Loaded grass tileset with " << tilesets["grass"]->getTileCount()
            << " tiles" << std::endl;
  return true;
}

Tileset *TileMapManager::getTileset(const std::string &name) {
  auto it = tilesets.find(name);
  return (it != tilesets.end()) ? it->second.get() : nullptr;
}

TileMap *TileMapManager::createTileMap(const std::string &name, int width,
                                       int height,
                                       const std::string &tilesetName) {
  Tileset *tileset = getTileset(tilesetName);
  if (!tileset) {
    std::cerr << "Cannot create tile map '" << name << "': tileset '"
              << tilesetName << "' not found" << std::endl;
    return nullptr;
  }

  auto tileMap = std::make_unique<TileMap>(width, height, tileset);
  tileMap->setName(name);

  TileMap *mapPtr = tileMap.get();
  tileMaps[name] = std::move(tileMap);

  std::cout << "Created tile map: " << name << " (" << width << "x" << height
            << ")" << std::endl;
  return mapPtr;
}

TileMap *TileMapManager::getTileMap(const std::string &name) {
  auto it = tileMaps.find(name);
  return (it != tileMaps.end()) ? it->second.get() : nullptr;
}

bool TileMapManager::setCurrentMap(const std::string &name) {
  TileMap *map = getTileMap(name);
  if (!map) {
    std::cerr << "Cannot set current map: '" << name << "' not found"
              << std::endl;
    return false;
  }

  currentMap = map;
  std::cout << "Set current map to: " << name << std::endl;
  return true;
}

bool TileMapManager::createDefaultGrassMap(const std::string &mapName,
                                           int width, int height) {
  // Ensure grass tileset is loaded
  if (!getTileset("grass")) {
    if (!loadGrassTileset()) {
      return false;
    }
  }

  // Create the map
  TileMap *map = createTileMap(mapName, width, height, "grass");
  if (!map) {
    return false;
  }

  // Generate grass terrain
  map->createGrassMap();

  // Set as current map if it's the first one
  if (!currentMap) {
    currentMap = map;
  }

  std::cout << "Created default grass map: " << mapName << std::endl;
  return true;
}

void TileMapManager::renderCurrentMap(void *renderer,
                                      const glm::vec2 &cameraPos,
                                      const glm::vec2 &screenSize) {
  if (currentMap) {
    currentMap->render(renderer, cameraPos, screenSize);
  }
}

void TileMapManager::renderMap(const std::string &mapName, void *renderer,
                               const glm::vec2 &cameraPos,
                               const glm::vec2 &screenSize) {
  TileMap *map = getTileMap(mapName);
  if (map) {
    map->render(renderer, cameraPos, screenSize);
  }
}

bool TileMapManager::isPositionBlocked(const glm::vec2 &worldPos) const {
  if (currentMap) {
    return currentMap->isPositionBlocked(worldPos);
  }
  return false;
}

bool TileMapManager::isTileSolid(const glm::vec2 &worldPos) const {
  if (currentMap) {
    glm::ivec2 tilePos = currentMap->worldToTilePosition(worldPos);
    return currentMap->isTileSolid(tilePos.x, tilePos.y);
  }
  return false;
}

void TileMapManager::clearAll() {
  currentMap = nullptr;
  tileMaps.clear();
  tilesets.clear();
  std::cout << "Cleared all tilesets and tile maps" << std::endl;
}

std::vector<std::string> TileMapManager::getTilesetNames() const {
  std::vector<std::string> names;
  names.reserve(tilesets.size());
  for (const auto &pair : tilesets) {
    names.push_back(pair.first);
  }
  return names;
}

std::vector<std::string> TileMapManager::getTileMapNames() const {
  std::vector<std::string> names;
  names.reserve(tileMaps.size());
  for (const auto &pair : tileMaps) {
    names.push_back(pair.first);
  }
  return names;
}

void TileMapManager::printTilesetInfo(const std::string &name) const {
  Tileset *tileset = const_cast<TileMapManager *>(this)->getTileset(name);
  if (tileset) {
    tileset->printTileInfo();
  } else {
    std::cout << "Tileset '" << name << "' not found" << std::endl;
  }
}

void TileMapManager::printMapInfo(const std::string &name) const {
  TileMap *map = const_cast<TileMapManager *>(this)->getTileMap(name);
  if (map) {
    map->printMap();
  } else {
    std::cout << "Tile map '" << name << "' not found" << std::endl;
  }
}

void TileMapManager::printAllInfo() const {
  std::cout << "=== TileMapManager Info ===" << std::endl;
  std::cout << "Loaded Tilesets (" << tilesets.size() << "):" << std::endl;
  for (const auto &name : getTilesetNames()) {
    std::cout << " - " << name << std::endl;
  }

  std::cout << "Loaded Tile Maps (" << tileMaps.size() << "):" << std::endl;
  for (const auto &name : getTileMapNames()) {
    std::cout << " - " << name << std::endl;
  }

  std::cout << "Current Map: " << (currentMap ? currentMap->getName() : "None")
            << std::endl;
}