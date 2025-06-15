#pragma once

class FPSCounter {
private:
  float fps;
  float fpsUpdateTimer;
  float fpsUpdateInterval;
  int frameCount;

public:
  FPSCounter(float updateInterval = 0.5f);

  void update(float deltaTime);
  float getFPS() const { return fps; }
  void reset();
};