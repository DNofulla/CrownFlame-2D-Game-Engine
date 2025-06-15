#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class InputManager {
private:
  GLFWwindow *window;

public:
  InputManager(GLFWwindow *window);

  // Movement input - returns normalized direction vector
  glm::vec2 getMovementInput() const;

  // Action keys
  bool isExitPressed() const;
  bool isRestartPressed() const;

  // General key checking
  bool isKeyPressed(int key) const;

  // Update method (for future key state tracking if needed)
  void update();
};