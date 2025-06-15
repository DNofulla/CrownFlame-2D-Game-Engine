#include "Tile.h"

Tile::Tile()
    : id(0), name("Unknown"), textureCoords(0.0f, 0.0f),
      textureSize(1.0f, 1.0f), isSolid(false), isWalkable(true),
      tintColor(1.0f, 1.0f, 1.0f, 1.0f), opacity(1.0f) {}

Tile::Tile(int tileId, const std::string &tileName, const glm::vec2 &texCoords,
           const glm::vec2 &texSize)
    : id(tileId), name(tileName), textureCoords(texCoords),
      textureSize(texSize), isSolid(false), isWalkable(true),
      tintColor(1.0f, 1.0f, 1.0f, 1.0f), opacity(1.0f) {}

void Tile::setProperties(bool solid, bool walkable) {
  isSolid = solid;
  isWalkable = walkable;
}

void Tile::setVisualProperties(const glm::vec4 &tint, float alpha) {
  tintColor = tint;
  opacity = alpha;
}

void Tile::getTextureQuad(glm::vec2 &topLeft, glm::vec2 &topRight,
                          glm::vec2 &bottomLeft, glm::vec2 &bottomRight) const {
  topLeft = textureCoords;
  topRight = glm::vec2(textureCoords.x + textureSize.x, textureCoords.y);
  bottomLeft = glm::vec2(textureCoords.x, textureCoords.y + textureSize.y);
  bottomRight = textureCoords + textureSize;
}