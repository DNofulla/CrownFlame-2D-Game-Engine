#pragma once
#include "TileMap.h"
#include "Tileset.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Manages multiple tilesets and tile maps for the game
 */
class TileMapManager {
private:
  std::unordered_map<std::string, std::unique_ptr<Tileset>> tilesets;
  std::unordered_map<std::string, std::unique_ptr<TileMap>> tileMaps;
  TileMap *currentMap; // The currently active map

public:
  TileMapManager();
  ~TileMapManager();

  // Tileset management
  bool loadTileset(const std::string &name, const std::string &imagePath,
                   int tileWidth, int tileHeight);
  bool loadGrassTileset(); // Convenience method for our grass tileset
  Tileset *getTileset(const std::string &name);

  // TileMap management
  TileMap *createTileMap(const std::string &name, int width, int height,
                         const std::string &tilesetName);
  TileMap *getTileMap(const std::string &name);
  bool setCurrentMap(const std::string &name);
  TileMap *getCurrentMap() const { return currentMap; }

  // Map generation
  bool createDefaultGrassMap(const std::string &mapName, int width = 50,
                             int height = 50);

  // Rendering
  void renderCurrentMap(void *renderer, const glm::vec2 &cameraPos,
                        const glm::vec2 &screenSize);
  void renderMap(const std::string &mapName, void *renderer,
                 const glm::vec2 &cameraPos, const glm::vec2 &screenSize);

  // Collision detection
  bool isPositionBlocked(const glm::vec2 &worldPos) const;
  bool isTileSolid(const glm::vec2 &worldPos) const;

  // Utility functions
  void clearAll();
  void
  resetCurrentMap(); // Reset the current map's content without destroying it
  bool resetMap(const std::string &mapName); // Reset a specific map's content
  std::vector<std::string> getTilesetNames() const;
  std::vector<std::string> getTileMapNames() const;

  // Debug functions
  void printTilesetInfo(const std::string &name) const;
  void printMapInfo(const std::string &name) const;
  void printAllInfo() const;
};