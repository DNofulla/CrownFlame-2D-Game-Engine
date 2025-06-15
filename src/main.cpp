#include "Application.h"
#include <iostream>

int main() {
  Application app;

  if (!app.initialize(800, 600, "Game with Collision Detection")) {
    std::cerr << "Failed to initialize application!" << std::endl;
    return -1;
  }

  app.run();

  return 0;
}
