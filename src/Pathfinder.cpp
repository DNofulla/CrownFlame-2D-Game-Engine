#include "Pathfinder.h"
#include "GameObject.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>

const float Pathfinder::GRID_SIZE = 25.0f;      // Grid size for pathfinding
const float Pathfinder::DIAGONAL_COST = 1.414f; // sqrt(2) for diagonal movement

std::vector<glm::vec2>
Pathfinder::findPath(const glm::vec2 &start, const glm::vec2 &end,
                     const std::vector<std::unique_ptr<GameObject>> &obstacles,
                     float characterWidth, float characterHeight) {
  std::vector<glm::vec2> path;

  // First, try direct path
  if (hasDirectPath(start, end, obstacles, characterWidth, characterHeight)) {
    path.push_back(end);
    return path;
  }

  // Snap positions to grid for A* pathfinding
  glm::vec2 gridStart = snapToGrid(start);
  glm::vec2 gridEnd = snapToGrid(end);

  // Early exit if start equals end
  if (gridStart == gridEnd) {
    path.push_back(end);
    return path;
  }

  // Check if end position is blocked
  if (isPositionBlocked(gridEnd, obstacles, characterWidth, characterHeight)) {
    // Try to find nearby unblocked position
    std::vector<glm::vec2> nearbyPositions = getNeighbors(gridEnd);
    gridEnd = gridEnd; // Keep original if no alternative found
    for (const auto &pos : nearbyPositions) {
      if (!isPositionBlocked(pos, obstacles, characterWidth, characterHeight)) {
        gridEnd = pos;
        break;
      }
    }
    // If still blocked, return empty path
    if (isPositionBlocked(gridEnd, obstacles, characterWidth,
                          characterHeight)) {
      return path;
    }
  }

  // A* algorithm implementation
  std::vector<std::unique_ptr<Node>> allNodes;
  std::vector<Node *> openSet;
  std::unordered_set<size_t> openSetHashes;
  std::unordered_set<size_t> closedSet;

  // Create start node
  auto startNode = std::make_unique<Node>(gridStart);
  startNode->gCost = 0;
  startNode->hCost = getDistance(gridStart, gridEnd);
  Node *currentStart = startNode.get();
  allNodes.push_back(std::move(startNode));

  openSet.push_back(currentStart);
  auto hashPos = [](const glm::vec2 &pos) -> size_t {
    return std::hash<float>{}(pos.x) ^ (std::hash<float>{}(pos.y) << 1);
  };
  openSetHashes.insert(hashPos(gridStart));

  Node *endNode = nullptr;
  int maxIterations = 1000; // Prevent infinite loops
  int iterations = 0;

  while (!openSet.empty() && iterations < maxIterations) {
    iterations++;

    // Find node with lowest fCost
    auto it =
        std::min_element(openSet.begin(), openSet.end(), [](Node *a, Node *b) {
          return a->fCost() < b->fCost();
        });

    Node *current = *it;
    openSet.erase(it);
    openSetHashes.erase(hashPos(current->position));
    closedSet.insert(hashPos(current->position));

    // Check if we reached the end
    if (current->position == gridEnd) {
      endNode = current;
      break;
    }

    // Check all neighbors
    std::vector<glm::vec2> neighbors = getNeighbors(current->position);
    for (const glm::vec2 &neighborPos : neighbors) {
      size_t neighborHash = hashPos(neighborPos);

      // Skip if in closed set or blocked
      if (closedSet.count(neighborHash) ||
          isPositionBlocked(neighborPos, obstacles, characterWidth,
                            characterHeight)) {
        continue;
      }

      // Calculate costs
      float moveCost = (abs(neighborPos.x - current->position.x) > 0.1f &&
                        abs(neighborPos.y - current->position.y) > 0.1f)
                           ? DIAGONAL_COST
                           : 1.0f;
      float tentativeGCost = current->gCost + moveCost;

      // Check if this neighbor is already in open set
      Node *existingNeighbor = nullptr;
      for (Node *node : openSet) {
        if (node->position == neighborPos) {
          existingNeighbor = node;
          break;
        }
      }

      if (existingNeighbor == nullptr) {
        // Add new node to open set
        auto neighborNode = std::make_unique<Node>(neighborPos);
        neighborNode->gCost = tentativeGCost;
        neighborNode->hCost = getDistance(neighborPos, gridEnd);
        neighborNode->parent = current;

        Node *neighborPtr = neighborNode.get();
        allNodes.push_back(std::move(neighborNode));
        openSet.push_back(neighborPtr);
        openSetHashes.insert(neighborHash);
      } else if (tentativeGCost < existingNeighbor->gCost) {
        // Update existing node if we found a better path
        existingNeighbor->gCost = tentativeGCost;
        existingNeighbor->parent = current;
      }
    }
  }

  // Reconstruct path if we found the end
  if (endNode != nullptr) {
    path = reconstructPath(endNode);
    // Add the actual end position (not grid-snapped)
    if (!path.empty()) {
      path.back() = end;
    }
  }

  return path;
}

float Pathfinder::getDistance(const glm::vec2 &a, const glm::vec2 &b) {
  float dx = abs(a.x - b.x);
  float dy = abs(a.y - b.y);

  // Manhattan distance with diagonal movement consideration
  if (dx > dy) {
    return DIAGONAL_COST * dy + (dx - dy);
  } else {
    return DIAGONAL_COST * dx + (dy - dx);
  }
}

bool Pathfinder::isPositionBlocked(
    const glm::vec2 &position,
    const std::vector<std::unique_ptr<GameObject>> &obstacles,
    float characterWidth, float characterHeight) {
  // Add extra spacing around the character to ensure it doesn't get stuck
  float extraSpacing = 10.0f;
  Rectangle characterRect(position.x - (characterWidth / 2 + extraSpacing),
                          position.y - (characterHeight / 2 + extraSpacing),
                          characterWidth + extraSpacing * 2,
                          characterHeight + extraSpacing * 2);

  for (const auto &obstacle : obstacles) {
    if (obstacle->type == ObjectType::OBSTACLE) {
      if (CollisionDetection::checkRectangleCollision(characterRect,
                                                      obstacle->bounds)) {
        return true;
      }
    }
  }
  return false;
}

std::vector<glm::vec2> Pathfinder::getNeighbors(const glm::vec2 &position) {
  std::vector<glm::vec2> neighbors;

  // 8-directional movement
  for (int dx = -1; dx <= 1; dx++) {
    for (int dy = -1; dy <= 1; dy++) {
      if (dx == 0 && dy == 0)
        continue;

      glm::vec2 neighbor(position.x + dx * GRID_SIZE,
                         position.y + dy * GRID_SIZE);
      neighbors.push_back(neighbor);
    }
  }

  return neighbors;
}

std::vector<glm::vec2> Pathfinder::reconstructPath(Node *endNode) {
  std::vector<glm::vec2> path;
  Node *current = endNode;

  while (current != nullptr) {
    path.push_back(current->position);
    current = current->parent;
  }

  // Reverse path so it goes from start to end
  std::reverse(path.begin(), path.end());
  return path;
}

bool Pathfinder::hasDirectPath(
    const glm::vec2 &start, const glm::vec2 &end,
    const std::vector<std::unique_ptr<GameObject>> &obstacles,
    float characterWidth, float characterHeight) {
  // Check multiple points along the line from start to end
  glm::vec2 direction = end - start;
  float distance = glm::length(direction);

  if (distance < 1.0f) {
    return true; // Very close, consider it direct
  }

  glm::vec2 normalizedDirection = direction / distance;
  float stepSize =
      std::min(10.0f, distance / 10.0f); // Check every 10 pixels or 10 steps

  for (float d = 0; d <= distance; d += stepSize) {
    glm::vec2 checkPoint = start + normalizedDirection * d;
    if (isPositionBlocked(checkPoint, obstacles, characterWidth,
                          characterHeight)) {
      return false;
    }
  }

  // Also check the exact end position
  return !isPositionBlocked(end, obstacles, characterWidth, characterHeight);
}

glm::vec2 Pathfinder::snapToGrid(const glm::vec2 &position) {
  return glm::vec2(round(position.x / GRID_SIZE) * GRID_SIZE,
                   round(position.y / GRID_SIZE) * GRID_SIZE);
}