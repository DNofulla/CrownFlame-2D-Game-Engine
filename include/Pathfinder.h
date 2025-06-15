#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>

// Forward declarations
class GameObject;

struct Node {
  glm::vec2 position;
  float gCost; // Distance from start
  float hCost; // Distance to end
  float fCost() const { return gCost + hCost; }
  Node *parent;

  Node(const glm::vec2 &pos)
      : position(pos), gCost(0), hCost(0), parent(nullptr) {}
};

class Pathfinder {
private:
  static const float GRID_SIZE; // Size of each grid cell for pathfinding
  static const float DIAGONAL_COST;

public:
  // Find a path from start to end, avoiding obstacles
  static std::vector<glm::vec2>
  findPath(const glm::vec2 &start, const glm::vec2 &end,
           const std::vector<std::unique_ptr<GameObject>> &obstacles,
           float characterWidth = 50.0f, float characterHeight = 50.0f);

private:
  // Helper functions
  static float getDistance(const glm::vec2 &a, const glm::vec2 &b);
  static bool
  isPositionBlocked(const glm::vec2 &position,
                    const std::vector<std::unique_ptr<GameObject>> &obstacles,
                    float characterWidth, float characterHeight);
  static bool
  hasDirectPath(const glm::vec2 &start, const glm::vec2 &end,
                const std::vector<std::unique_ptr<GameObject>> &obstacles,
                float characterWidth, float characterHeight);
  static std::vector<glm::vec2> getNeighbors(const glm::vec2 &position);
  static std::vector<glm::vec2> reconstructPath(Node *endNode);
  static glm::vec2 snapToGrid(const glm::vec2 &position);
};