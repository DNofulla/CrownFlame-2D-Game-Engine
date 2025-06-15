#include "FPSCounter.h"

FPSCounter::FPSCounter(float updateInterval)
    : fps(0.0f), fpsUpdateTimer(0.0f), fpsUpdateInterval(updateInterval) {}

void FPSCounter::update(float deltaTime) {
  fpsUpdateTimer += deltaTime;
  if (fpsUpdateTimer >= fpsUpdateInterval) {
    fps = (deltaTime > 0.0f) ? 1.0f / deltaTime : 0.0f;
    fpsUpdateTimer = 0.0f;
  }
}

void FPSCounter::reset() {
  fps = 0.0f;
  fpsUpdateTimer = 0.0f;
}