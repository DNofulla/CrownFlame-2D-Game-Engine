#pragma once
#include "SceneData.h"
#include <string>
#include <vector>


struct SceneValidationIssue {
  enum class Severity { WARNING, ERROR };

  Severity severity;
  std::string message;
  std::string location; // e.g., "obstacle[2]", "enemy[0]", "player"

  SceneValidationIssue(Severity sev, const std::string &msg,
                       const std::string &loc = "")
      : severity(sev), message(msg), location(loc) {}
};

struct SceneValidationResult {
  bool isValid;
  std::vector<SceneValidationIssue> issues;

  SceneValidationResult() : isValid(true) {}

  void addError(const std::string &message, const std::string &location = "") {
    issues.emplace_back(SceneValidationIssue::Severity::ERROR, message,
                        location);
    isValid = false;
  }

  void addWarning(const std::string &message,
                  const std::string &location = "") {
    issues.emplace_back(SceneValidationIssue::Severity::WARNING, message,
                        location);
  }

  int getErrorCount() const {
    int count = 0;
    for (const auto &issue : issues) {
      if (issue.severity == SceneValidationIssue::Severity::ERROR)
        count++;
    }
    return count;
  }

  int getWarningCount() const {
    int count = 0;
    for (const auto &issue : issues) {
      if (issue.severity == SceneValidationIssue::Severity::WARNING)
        count++;
    }
    return count;
  }
};

class SceneValidator {
public:
  static SceneValidationResult
  validate(const SceneData::SceneDefinition &scene);

private:
  static void validateBasicProperties(const SceneData::SceneDefinition &scene,
                                      SceneValidationResult &result);
  static void validateWorldSettings(const SceneData::SceneDefinition &scene,
                                    SceneValidationResult &result);
  static void validatePlayerSpawn(const SceneData::SceneDefinition &scene,
                                  SceneValidationResult &result);
  static void validateObstacles(const SceneData::SceneDefinition &scene,
                                SceneValidationResult &result);
  static void validateCollectibles(const SceneData::SceneDefinition &scene,
                                   SceneValidationResult &result);
  static void validateEnemies(const SceneData::SceneDefinition &scene,
                              SceneValidationResult &result);
  static void validateObjectOverlaps(const SceneData::SceneDefinition &scene,
                                     SceneValidationResult &result);
  static void validateReachability(const SceneData::SceneDefinition &scene,
                                   SceneValidationResult &result);

  // Helper functions
  static bool isPointInObstacle(const glm::vec2 &point,
                                const SceneData::ObstacleData &obstacle);
  static bool doObstaclesOverlap(const SceneData::ObstacleData &a,
                                 const SceneData::ObstacleData &b);
  static bool
  isCollectibleReachable(const SceneData::CollectibleData &collectible,
                         const SceneData::PlayerSpawn &playerSpawn,
                         const std::vector<SceneData::ObstacleData> &obstacles);
};