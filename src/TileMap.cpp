#include "TileMap.h"
#include <algorithm>
#include <gl2d/gl2d.h>
#include <iomanip>
#include <iostream>
#include <random>

TileMap::TileMap()
    : tileset(nullptr), mapWidth(0), mapHeight(0), tilePixelWidth(64),
      tilePixelHeight(64), worldPosition(0.0f, 0.0f) {}

TileMap::TileMap(int width, int height, Tileset *tilesetPtr)
    : tileset(tilesetPtr), mapWidth(width), mapHeight(height),
      tilePixelWidth(64), tilePixelHeight(64), worldPosition(0.0f, 0.0f) {
  if (tileset) {
    tilePixelWidth = tileset->getTileWidth();
    tilePixelHeight = tileset->getTileHeight();
  }

  // Initialize tile data
  tileData.resize(mapHeight);
  for (int y = 0; y < mapHeight; ++y) {
    tileData[y].resize(mapWidth, 0); // Default to tile ID 0
  }
}

TileMap::~TileMap() { clear(); }

bool TileMap::initialize(int width, int height, Tileset *tilesetPtr) {
  if (!tilesetPtr) {
    std::cerr << "Cannot initialize TileMap with null tileset" << std::endl;
    return false;
  }

  tileset = tilesetPtr;
  mapWidth = width;
  mapHeight = height;
  tilePixelWidth = tileset->getTileWidth();
  tilePixelHeight = tileset->getTileHeight();

  // Initialize tile data
  tileData.resize(mapHeight);
  for (int y = 0; y < mapHeight; ++y) {
    tileData[y].resize(mapWidth, 0);
  }

  return true;
}

void TileMap::clear() {
  tileData.clear();
  mapWidth = 0;
  mapHeight = 0;
}

void TileMap::setTile(int x, int y, int tileId) {
  if (isValidCoordinate(x, y)) {
    tileData[y][x] = tileId;
  }
}

int TileMap::getTile(int x, int y) const {
  if (isValidCoordinate(x, y)) {
    return tileData[y][x];
  }
  return -1; // Invalid tile
}

Tile *TileMap::getTileObject(int x, int y) const {
  if (!tileset || !isValidCoordinate(x, y)) {
    return nullptr;
  }

  int tileId = tileData[y][x];
  return tileset->getTile(tileId);
}

void TileMap::fill(int tileId) {
  for (int y = 0; y < mapHeight; ++y) {
    for (int x = 0; x < mapWidth; ++x) {
      tileData[y][x] = tileId;
    }
  }
}

void TileMap::fillRect(int x, int y, int width, int height, int tileId) {
  int endX = std::min(x + width, mapWidth);
  int endY = std::min(y + height, mapHeight);

  for (int ty = y; ty < endY; ++ty) {
    for (int tx = x; tx < endX; ++tx) {
      if (isValidCoordinate(tx, ty)) {
        tileData[ty][tx] = tileId;
      }
    }
  }
}

void TileMap::createGrassMap() {
  if (!tileset) {
    std::cerr << "Cannot create grass map without tileset" << std::endl;
    return;
  }

  // Random number generator for variety
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> grassVariants(
      0, 5); // Basic grass variants (IDs 0-5)
  std::uniform_int_distribution<> specialChance(1, 100);
  std::uniform_int_distribution<> specialTypes(20,
                                               25); // Special tile IDs (20-25)

  // Fill the map with varied grass
  for (int y = 0; y < mapHeight; ++y) {
    for (int x = 0; x < mapWidth; ++x) {
      // 85% chance for basic grass variants, 15% chance for special tiles
      if (specialChance(gen) <= 85) {
        // Use basic grass variants
        tileData[y][x] = grassVariants(gen);
      } else {
        // Use special tiles (stone paths, flower patches, etc.)
        tileData[y][x] = specialTypes(gen);
      }
    }
  }

  // Add some stone paths for variety
  int numPaths = (mapWidth * mapHeight) / 200; // Roughly 1 path per 200 tiles
  std::uniform_int_distribution<> pathX(1, mapWidth - 2);
  std::uniform_int_distribution<> pathY(1, mapHeight - 2);
  std::uniform_int_distribution<> pathLength(3, 8);
  std::uniform_int_distribution<> pathDirection(
      0, 3); // 0=horizontal, 1=vertical, 2=diagonal1, 3=diagonal2

  for (int i = 0; i < numPaths; ++i) {
    int startX = pathX(gen);
    int startY = pathY(gen);
    int length = pathLength(gen);
    int direction = pathDirection(gen);

    for (int j = 0; j < length; ++j) {
      int px = startX, py = startY;

      switch (direction) {
      case 0:
        px = startX + j;
        break; // Horizontal
      case 1:
        py = startY + j;
        break; // Vertical
      case 2:
        px = startX + j;
        py = startY + j;
        break; // Diagonal down-right
      case 3:
        px = startX + j;
        py = startY - j;
        break; // Diagonal up-right
      }

      if (isValidCoordinate(px, py)) {
        tileData[py][px] = 20; // Stone path tile ID
      }
    }
  }

  std::cout << "Created grass map of size " << mapWidth << "x" << mapHeight
            << " with " << numPaths << " stone paths" << std::endl;
}

bool TileMap::isValidCoordinate(int x, int y) const {
  return x >= 0 && x < mapWidth && y >= 0 && y < mapHeight;
}

void TileMap::render(void *renderer, const glm::vec2 &cameraPos,
                     const glm::vec2 &screenSize) {
  if (!tileset || !renderer) {
    return;
  }

  gl2d::Renderer2D *r = static_cast<gl2d::Renderer2D *>(renderer);

  // Render ALL tiles - full map always visible
  for (int y = 0; y < mapHeight; ++y) {
    for (int x = 0; x < mapWidth; ++x) {
      glm::vec2 renderPos = tileToWorldPosition(x, y);
      renderTile(renderer, x, y, renderPos);
    }
  }
}

void TileMap::renderTile(void *renderer, int x, int y,
                         const glm::vec2 &renderPos) {
  if (!tileset || !renderer || !isValidCoordinate(x, y)) {
    return;
  }

  gl2d::Renderer2D *r = static_cast<gl2d::Renderer2D *>(renderer);
  int tileId = tileData[y][x];
  Tile *tile = tileset->getTile(tileId);

  if (!tile) {
    return; // Skip invalid tiles
  }

  // Get texture coordinates for this tile
  glm::vec2 topLeft, topRight, bottomLeft, bottomRight;
  tile->getTextureQuad(topLeft, topRight, bottomLeft, bottomRight);

  // Create texture coordinates vector (left, top, right, bottom)
  glm::vec4 texCoords(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);

  // Render the tile
  gl2d::Rect tileRect = {renderPos.x, renderPos.y,
                         static_cast<float>(tilePixelWidth),
                         static_cast<float>(tilePixelHeight)};

  // Apply tile tint color and opacity
  gl2d::Color4f tileColor = {tile->tintColor.r, tile->tintColor.g,
                             tile->tintColor.b,
                             tile->tintColor.a * tile->opacity};

  // Get the specific texture for this tile
  void *tileTexturePtr = tileset->getTileTexture(tile->name);
  if (tileTexturePtr) {
    r->renderRectangle(tileRect, *static_cast<gl2d::Texture *>(tileTexturePtr),
                       tileColor, {0, 0}, 0, texCoords);
  }
}

glm::vec2 TileMap::tileToWorldPosition(int tileX, int tileY) const {
  return worldPosition +
         glm::vec2(tileX * tilePixelWidth, tileY * tilePixelHeight);
}

glm::ivec2 TileMap::worldToTilePosition(const glm::vec2 &worldPos) const {
  glm::vec2 relativePos = worldPos - worldPosition;
  return glm::ivec2(static_cast<int>(relativePos.x / tilePixelWidth),
                    static_cast<int>(relativePos.y / tilePixelHeight));
}

bool TileMap::isTileSolid(int x, int y) const {
  Tile *tile = getTileObject(x, y);
  return tile ? tile->isSolid : false;
}

bool TileMap::isPositionBlocked(const glm::vec2 &worldPos) const {
  glm::ivec2 tilePos = worldToTilePosition(worldPos);
  return isTileSolid(tilePos.x, tilePos.y);
}

void TileMap::printMap() const {
  std::cout << "=== TileMap: " << name << " ===" << std::endl;
  std::cout << "Size: " << mapWidth << "x" << mapHeight << std::endl;
  std::cout << "Tile size: " << tilePixelWidth << "x" << tilePixelHeight
            << std::endl;
  std::cout << "World position: (" << worldPosition.x << ", " << worldPosition.y
            << ")" << std::endl;

  for (int y = 0; y < std::min(mapHeight, 20);
       ++y) { // Limit output for readability
    for (int x = 0; x < std::min(mapWidth, 40); ++x) {
      std::cout << std::setfill('0') << std::setw(2) << tileData[y][x] << " ";
    }
    std::cout << std::endl;
  }

  if (mapHeight > 20 || mapWidth > 40) {
    std::cout << "... (map truncated for display)" << std::endl;
  }
}

void TileMap::printTileInfo(int x, int y) const {
  if (!isValidCoordinate(x, y)) {
    std::cout << "Invalid tile coordinate: (" << x << ", " << y << ")"
              << std::endl;
    return;
  }

  int tileId = tileData[y][x];
  Tile *tile = getTileObject(x, y);

  std::cout << "Tile at (" << x << ", " << y << "):" << std::endl;
  std::cout << "  ID: " << tileId << std::endl;

  if (tile) {
    std::cout << "  Name: " << tile->name << std::endl;
    std::cout << "  Solid: " << (tile->isSolid ? "Yes" : "No") << std::endl;
    std::cout << "  Walkable: " << (tile->isWalkable ? "Yes" : "No")
              << std::endl;
    std::cout << "  Texture coords: (" << tile->textureCoords.x << ", "
              << tile->textureCoords.y << ")" << std::endl;
  } else {
    std::cout << "  Tile object not found!" << std::endl;
  }
}

void TileMap::calculateVisibleTileRange(const glm::vec2 &cameraPos,
                                        const glm::vec2 &screenSize,
                                        int &minTileX, int &minTileY,
                                        int &maxTileX, int &maxTileY) const {
  // Calculate camera view bounds
  glm::vec2 viewMin = cameraPos - screenSize * 0.5f;
  glm::vec2 viewMax = cameraPos + screenSize * 0.5f;

  // Convert to tile coordinates with some padding for safety
  glm::ivec2 minTile = worldToTilePosition(viewMin) - glm::ivec2(1, 1);
  glm::ivec2 maxTile = worldToTilePosition(viewMax) + glm::ivec2(1, 1);

  // Clamp to map bounds
  minTileX = std::max(0, minTile.x);
  minTileY = std::max(0, minTile.y);
  maxTileX = std::min(mapWidth - 1, maxTile.x);
  maxTileY = std::min(mapHeight - 1, maxTile.y);
}