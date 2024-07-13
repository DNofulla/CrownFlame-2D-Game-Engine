#define GLM_ENABLE_EXPERIMENTAL
#include "game.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "input.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include "imfilebrowser.h"
#include <gl2d/gl2d.h>
#include <tools.h>

struct GameData
{
	glm::vec2 rectPos = { 100, 100 };

}GameSave;

gl2d::Renderer2D renderer;

bool initGame()
{
	gl2d::init();
	renderer.create();
	platform::readEntireFile(RESOURCES_PATH "GameSave.data", &GameSave, sizeof(GameData));

	return true;
}

bool gameLogic(float deltaTime)
{
#pragma region Game Init
	int width = 0; int height = 0;
	width = platform::getFrameBufferSizeX(); // window width
	height = platform::getFrameBufferSizeY(); // window height
	
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT); //clear screen

	renderer.updateWindowMetrics(width, height);

#pragma endregion

	if (platform::isButtonHeld(platform::Button::Left))
	{
		GameSave.rectPos.x -= deltaTime * 100;
	}

	if (platform::isButtonHeld(platform::Button::Right))
	{
		GameSave.rectPos.x += deltaTime * 100;
	}

	if (platform::isButtonHeld(platform::Button::Up))
	{
		GameSave.rectPos.y -= deltaTime * 100;
	}

	if (platform::isButtonHeld(platform::Button::Down))
	{
		GameSave.rectPos.y += deltaTime * 100;
	}

	GameSave.rectPos = glm::clamp(GameSave.rectPos, glm::vec2{ 0,0 }, glm::vec2{ width - 100, height - 100 });

	renderer.flush();

	ImGui::Begin("RECTANGLE POSITION");
	ImGui::DragFloat2("X | Y Position", &GameSave.rectPos[0]);
	ImGui::End();

	return true;
#pragma endregion
}

void closeGame()
{
	platform::writeEntireFile(RESOURCES_PATH "GameSave.data", &GameSave, sizeof(GameData));
}
