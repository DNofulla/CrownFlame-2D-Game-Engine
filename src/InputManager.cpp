#include "InputManager.h"
#include <stdexcept>

InputManager::InputManager(GLFWwindow *window)
    : window(window), rightMousePressed(false), rightMouseJustPressed(false),
      mousePosition(0.0f, 0.0f) {
  if (!window) {
    throw std::runtime_error(
        "InputManager: Cannot initialize with null window pointer");
  }
}

glm::vec2 InputManager::getMovementInput() const {
  if (!window)
    return glm::vec2(0.0f, 0.0f);

  glm::vec2 movement(0.0f, 0.0f);

  if (isKeyPressed(GLFW_KEY_W) || isKeyPressed(GLFW_KEY_UP))
    movement.y -= 1.0f;
  if (isKeyPressed(GLFW_KEY_S) || isKeyPressed(GLFW_KEY_DOWN))
    movement.y += 1.0f;
  if (isKeyPressed(GLFW_KEY_A) || isKeyPressed(GLFW_KEY_LEFT))
    movement.x -= 1.0f;
  if (isKeyPressed(GLFW_KEY_D) || isKeyPressed(GLFW_KEY_RIGHT))
    movement.x += 1.0f;

  return movement;
}

bool InputManager::isExitPressed() const {
  return window ? isKeyPressed(GLFW_KEY_ESCAPE) : false;
}

bool InputManager::isRestartPressed() const {
  return window ? isKeyPressed(GLFW_KEY_R) : false;
}

bool InputManager::isKeyPressed(int key) const {
  return window ? (glfwGetKey(window, key) == GLFW_PRESS) : false;
}

void InputManager::update() {
  if (!window)
    return;

  // Update mouse position
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  mousePosition = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));

  // Check right mouse button state
  bool currentRightPressed =
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
  rightMouseJustPressed = currentRightPressed && !rightMousePressed;
  rightMousePressed = currentRightPressed;
}