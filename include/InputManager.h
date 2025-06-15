#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class InputManager {
private:
  GLFWwindow *window;

  // Mouse state
  bool rightMousePressed;
  bool rightMouseJustPressed;
  glm::vec2 mousePosition;

public:
  InputManager(GLFWwindow *window);

  // Movement input - returns normalized direction vector
  glm::vec2 getMovementInput() const;

  // Action keys
  bool isExitPressed() const;
  bool isRestartPressed() const;

  // Mouse input
  bool isRightMousePressed() const { return rightMousePressed; }
  bool isRightMouseJustPressed() const { return rightMouseJustPressed; }
  glm::vec2 getMousePosition() const { return mousePosition; }

  // General key checking
  bool isKeyPressed(int key) const;

  // Update method (for tracking mouse state changes)
  void update();
};