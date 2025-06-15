#include "GameState.h"

GameStateManager::GameStateManager()
    : currentState(GameState::PLAYING), gameOverTime(0.0f) {}

void GameStateManager::setState(GameState newState) {
  currentState = newState;
  if (newState == GameState::GAME_OVER) {
    gameOverTime = 0.0f;
  }
}

void GameStateManager::update(float deltaTime) {
  if (currentState == GameState::GAME_OVER) {
    gameOverTime += deltaTime;
  }
}

void GameStateManager::triggerGameOver() { setState(GameState::GAME_OVER); }

void GameStateManager::resetGame() {
  setState(GameState::PLAYING);
  gameOverTime = 0.0f;
}