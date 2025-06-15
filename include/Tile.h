#pragma once
#include <glm/glm.hpp>
#include <string>

/**
 * Represents a single tile in a tileset
 */
class Tile {
public:
  // Tile properties
  int id;                  // Unique identifier for this tile
  std::string name;        // Name/description of the tile
  glm::vec2 textureCoords; // UV coordinates in the tileset texture (top-left)
  glm::vec2
      textureSize; // Size of the tile in texture coordinates (normalized 0-1)
  bool isSolid;    // Whether this tile blocks movement
  bool isWalkable; // Whether entities can walk on this tile

  // Visual properties
  glm::vec4 tintColor; // Color tint to apply to the tile (default: white)
  float opacity;       // Opacity of the tile (0.0 - 1.0)

  // Constructors
  Tile();
  Tile(int tileId, const std::string &tileName, const glm::vec2 &texCoords,
       const glm::vec2 &texSize);

  // Utility functions
  void setProperties(bool solid, bool walkable);
  void setVisualProperties(const glm::vec4 &tint = glm::vec4(1.0f),
                           float alpha = 1.0f);

  // Get the four corners of the tile in texture coordinates
  void getTextureQuad(glm::vec2 &topLeft, glm::vec2 &topRight,
                      glm::vec2 &bottomLeft, glm::vec2 &bottomRight) const;
};