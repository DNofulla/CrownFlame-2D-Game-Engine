#include "FPSCounter.h"

FPSCounter::FPSCounter(float updateInterval)
    : fps(0.0f), fpsUpdateTimer(0.0f), fpsUpdateInterval(updateInterval),
      frameCount(0) {}

void FPSCounter::update(float deltaTime) {
  fpsUpdateTimer += deltaTime;
  frameCount++;

  if (fpsUpdateTimer >= fpsUpdateInterval) {
    fps = (fpsUpdateTimer > 0.0f) ? frameCount / fpsUpdateTimer : 0.0f;
    fpsUpdateTimer = 0.0f;
    frameCount = 0;
  }
}

void FPSCounter::reset() {
  fps = 0.0f;
  fpsUpdateTimer = 0.0f;
  frameCount = 0;
}