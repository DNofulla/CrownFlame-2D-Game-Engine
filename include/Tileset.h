#pragma once
#include "Tile.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Manages a collection of tiles from individual tile images
 */
class Tileset {
private:
  std::vector<std::unique_ptr<Tile>> tiles;
  std::unordered_map<int, Tile *> tileMap;             // Quick lookup by ID
  std::unordered_map<std::string, Tile *> tileNameMap; // Quick lookup by name
  std::unordered_map<std::string, void *>
      tileTextures; // Individual tile textures

  // Tileset properties
  int tileWidth;
  int tileHeight;
  std::string name;

public:
  Tileset();
  ~Tileset();

  // Loading functions
  bool loadTileFromFile(int id, const std::string &tileName,
                        const std::string &imagePath);
  bool loadGrassTileset(); // Specialized loader for our grass tileset

  // Tile management
  Tile *addTile(int id, const std::string &tileName,
                const std::string &imagePath);
  Tile *getTile(int id) const;
  Tile *getTile(const std::string &name) const;

  // Getters
  void *getTileTexture(const std::string &tileName) const;
  int getTileWidth() const { return tileWidth; }
  int getTileHeight() const { return tileHeight; }
  const std::string &getName() const { return name; }
  size_t getTileCount() const { return tiles.size(); }

  // Utility functions
  void setName(const std::string &tilesetName) { name = tilesetName; }
  std::vector<Tile *> getAllTiles() const;
  void printTileInfo() const; // Debug function

private:
  // Helper functions
  void setupGrassTiles(); // Sets up all the grass tiles with proper names and
                          // properties
};