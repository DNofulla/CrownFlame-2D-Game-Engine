#pragma once
#include "Tileset.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>


/**
 * Represents a 2D grid of tiles that forms a game map
 */
class TileMap {
private:
  std::vector<std::vector<int>> tileData; // 2D grid storing tile IDs
  Tileset *tileset;                       // Reference to the tileset to use
  int mapWidth;                           // Width of the map in tiles
  int mapHeight;                          // Height of the map in tiles
  int tilePixelWidth;                     // Width of each tile in pixels
  int tilePixelHeight;                    // Height of each tile in pixels

  // Map properties
  std::string name;
  glm::vec2 worldPosition; // Position of the map in world coordinates

public:
  TileMap();
  TileMap(int width, int height, Tileset *tilesetPtr);
  ~TileMap();

  // Initialization
  bool initialize(int width, int height, Tileset *tilesetPtr);
  void clear();

  // Tile manipulation
  void setTile(int x, int y, int tileId);
  int getTile(int x, int y) const;
  Tile *getTileObject(int x, int y) const;

  // Map generation
  void fill(int tileId);
  void fillRect(int x, int y, int width, int height, int tileId);
  void createGrassMap(); // Creates a default grass map with variety

  // Bounds checking
  bool isValidCoordinate(int x, int y) const;

  // Rendering
  void render(void *renderer, const glm::vec2 &cameraPos,
              const glm::vec2 &screenSize);
  void renderTile(void *renderer, int x, int y, const glm::vec2 &renderPos);

  // Coordinate conversion
  glm::vec2 tileToWorldPosition(int tileX, int tileY) const;
  glm::ivec2 worldToTilePosition(const glm::vec2 &worldPos) const;

  // Collision detection
  bool isTileSolid(int x, int y) const;
  bool isPositionBlocked(const glm::vec2 &worldPos) const;

  // Getters
  int getMapWidth() const { return mapWidth; }
  int getMapHeight() const { return mapHeight; }
  int getTilePixelWidth() const { return tilePixelWidth; }
  int getTilePixelHeight() const { return tilePixelHeight; }
  Tileset *getTileset() const { return tileset; }
  const std::string &getName() const { return name; }
  const glm::vec2 &getWorldPosition() const { return worldPosition; }

  // Setters
  void setName(const std::string &mapName) { name = mapName; }
  void setWorldPosition(const glm::vec2 &pos) { worldPosition = pos; }

  // Debug functions
  void printMap() const;
  void printTileInfo(int x, int y) const;

private:
  // Helper functions for rendering optimization
  void calculateVisibleTileRange(const glm::vec2 &cameraPos,
                                 const glm::vec2 &screenSize, int &minTileX,
                                 int &minTileY, int &maxTileX,
                                 int &maxTileY) const;
};