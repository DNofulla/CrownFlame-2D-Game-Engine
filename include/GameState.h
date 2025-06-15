#pragma once

enum class GameState { PLAYING, GAME_OVER, PAUSED };

class GameStateManager {
private:
  GameState currentState;
  float gameOverTime;

public:
  GameStateManager();

  void setState(GameState newState);
  GameState getState() const { return currentState; }

  void update(float deltaTime);

  // Game over specific
  void triggerGameOver();
  float getGameOverTime() const { return gameOverTime; }
  void resetGame();

  bool isPlaying() const { return currentState == GameState::PLAYING; }
  bool isGameOver() const { return currentState == GameState::GAME_OVER; }
  bool isPaused() const { return currentState == GameState::PAUSED; }
};