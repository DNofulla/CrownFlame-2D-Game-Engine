#include "Tileset.h"
#include <gl2d/gl2d.h>
#include <iostream>

Tileset::Tileset() : tileWidth(64), tileHeight(64) {}

Tileset::~Tileset() {
  // Clean up all tile textures
  for (auto &pair : tileTextures) {
    if (pair.second) {
      static_cast<gl2d::Texture *>(pair.second)->cleanup();
      delete static_cast<gl2d::Texture *>(pair.second);
    }
  }
  tileTextures.clear();
  tiles.clear();
  tileMap.clear();
  tileNameMap.clear();
}

bool Tileset::loadTileFromFile(int id, const std::string &tileName,
                               const std::string &imagePath) {
  // Check if tile ID already exists
  if (tileMap.find(id) != tileMap.end()) {
    std::cerr << "Warning: Tile ID " << id << " already exists, overwriting."
              << std::endl;
  }

  // Load the texture
  auto texture = new gl2d::Texture();
  texture->loadFromFile(imagePath.c_str());

  // Check if texture was loaded successfully
  if (texture->id == 0) {
    std::cerr << "Failed to load tile image: " << imagePath << std::endl;
    delete texture;
    return false;
  }

  // Create tile with full texture coordinates (0,0 to 1,1 since each tile is
  // its own texture)
  auto tile = std::make_unique<Tile>(id, tileName, glm::vec2(0.0f, 0.0f),
                                     glm::vec2(1.0f, 1.0f));
  Tile *tilePtr = tile.get();

  // Store everything
  tiles.push_back(std::move(tile));
  tileMap[id] = tilePtr;
  tileNameMap[tileName] = tilePtr;
  tileTextures[tileName] = texture;

  std::cout << "Loaded tile: " << tileName << " (ID: " << id << ") from "
            << imagePath << std::endl;
  return true;
}

bool Tileset::loadGrassTileset() {
  setName("Grass Tileset");
  setupGrassTiles();
  return true;
}

Tile *Tileset::addTile(int id, const std::string &tileName,
                       const std::string &imagePath) {
  if (loadTileFromFile(id, tileName, imagePath)) {
    return getTile(id);
  }
  return nullptr;
}

Tile *Tileset::getTile(int id) const {
  auto it = tileMap.find(id);
  return (it != tileMap.end()) ? it->second : nullptr;
}

Tile *Tileset::getTile(const std::string &name) const {
  auto it = tileNameMap.find(name);
  return (it != tileNameMap.end()) ? it->second : nullptr;
}

void *Tileset::getTileTexture(const std::string &tileName) const {
  auto it = tileTextures.find(tileName);
  return (it != tileTextures.end()) ? it->second : nullptr;
}

std::vector<Tile *> Tileset::getAllTiles() const {
  std::vector<Tile *> result;
  result.reserve(tiles.size());
  for (const auto &tile : tiles) {
    result.push_back(tile.get());
  }
  return result;
}

void Tileset::printTileInfo() const {
  std::cout << "=== Tileset: " << name << " ===" << std::endl;
  std::cout << "Total tiles: " << tiles.size() << std::endl;
  for (const auto &tile : tiles) {
    std::cout << "ID: " << tile->id << ", Name: " << tile->name << std::endl;
  }
}

void Tileset::setupGrassTiles() {
  // Clear existing tiles
  tiles.clear();
  tileMap.clear();
  tileNameMap.clear();
  tileTextures.clear();

  // Define tile mapping based on the PNG files we found
  struct TileInfo {
    int id;
    std::string name;
    std::string filename;
    bool isSolid;
  };

  std::vector<TileInfo> tileInfos = {
      // Basic grass variants
      {0, "Basic Grass", "grass_basic.png", false},
      {1, "Dense Grass", "grass_dense.png", false},
      {2, "Tall Grass", "grass_tall.png", false},
      {3, "Grass with Flowers", "grass_flowers.png", false},
      {4, "Dark Grass", "grass_dark.png", false},
      {5, "Light Grass", "grass_light.png", false},

      // Edge tiles
      {10, "Top Edge", "grass_edge_top.png", false},
      {11, "Right Edge", "grass_edge_right.png", false},
      {12, "Bottom Edge", "grass_edge_bottom.png", false},
      {13, "Left Edge", "grass_edge_left.png", false},
      {14, "Top-Left Corner", "grass_corner_top_left.png", false},
      {15, "Top-Right Corner", "grass_corner_top_right.png", false},

      // Special tiles
      {20, "Stone Path", "grass_stone_path.png", false},
      {21, "Dirt Patches", "grass_dirt_patches.png", false},
      {22, "Worn Grass", "grass_worn.png", false},
      {23, "Grass Transition", "grass_transition.png", false},
      {24, "Rocky Grass", "grass_rocky.png", false},
      {25, "Flower Patch", "grass_flower_patch.png", false}};

  // Load all tiles
  for (const auto &info : tileInfos) {
    std::string fullPath = RESOURCES_PATH "tiles/" + info.filename;
    if (loadTileFromFile(info.id, info.name, fullPath)) {
      Tile *tile = getTile(info.id);
      if (tile) {
        tile->setProperties(info.isSolid, true); // All grass tiles are walkable
      }
    }
  }

  std::cout << "Set up " << tiles.size()
            << " grass tiles from individual PNG files" << std::endl;
}