#include "PlayScene.h"
#include "Game.h"
#include "EventManager.h"
#include "Config.h"

// required for IMGUI
#include "imgui.h"
#include "imgui_sdl.h"
#include "Renderer.h"
#include "Util.h"

PlayScene::PlayScene()
{
	PlayScene::start();
}

PlayScene::~PlayScene()
= default;

void PlayScene::draw()
{
	drawDisplayList();

	SDL_SetRenderDrawColor(Renderer::Instance().getRenderer(), 255, 255, 255, 255);
}

void PlayScene::update()
{
	updateDisplayList();


}

void PlayScene::clean()
{
	removeAllChildren();
}

void PlayScene::handleEvents()
{
	EventManager::Instance().update();

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_ESCAPE))
	{
		TheGame::Instance().quit();
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_1))
	{
		TheGame::Instance().changeSceneState(START_SCENE);
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_2))
	{
		TheGame::Instance().changeSceneState(END_SCENE);
	}
}

void PlayScene::start()
{

	// Set GUI Title
	m_guiTitle = "Play Scene";

	//setup the grid
	m_buildGrid();
	auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);
	m_currentHeuristic = MANHATTAN;

	m_pParking = new Target();
	m_pParking->getTransform()->position = m_getTile(15, 11)->getTransform()->position + offset;
	m_pParking->setGridPosition(15.0f, 11.0f);
	m_getTile(15, 11)->setTileStatus(GOAL);
	addChild(m_pParking);

	m_pCar = new Car();
	m_pCar->getTransform()->position = m_getTile(1, 3)->getTransform()->position + offset;
	m_pCar->setGridPosition(1.0f, 3.0f);
	m_getTile(1, 3)->setTileStatus(START);
	addChild(m_pCar);

	SoundManager::Instance().load("../Assets/audio/yay.ogg", "yay", SOUND_SFX);
	SoundManager::Instance().load("../Assets/audio/thunder.ogg", "boom", SOUND_SFX);

	m_pInstructions = new Label("Press 'F' to find the shortest path to the target!", "Consolas");
	m_pInstructions->getTransform()->position = glm::vec2(Config::SCREEN_WIDTH * 0.5f, 460.0f);
	addChild(m_pInstructions);

	m_pInstructions = new Label("Press 'M' to move the target!", "Consolas");
	m_pInstructions->getTransform()->position = glm::vec2(Config::SCREEN_WIDTH * 0.5f, 480.0f);
	addChild(m_pInstructions);

	m_pInstructions = new Label("Press 'R' to reset the scene!", "Consolas");
	m_pInstructions->getTransform()->position = glm::vec2(Config::SCREEN_WIDTH * 0.5f, 500.0f);
	addChild(m_pInstructions);

	m_pInstructions = new Label("Press 'H' to open the debug menu!", "Consolas");
	m_pInstructions->getTransform()->position = glm::vec2(Config::SCREEN_WIDTH * 0.5f, 520.0f);
	addChild(m_pInstructions);

	m_pInstructions = new Label("'Left Click' a tile to set the start tile!", "Consolas");
	m_pInstructions->getTransform()->position = glm::vec2(Config::SCREEN_WIDTH * 0.5f, 540.0f);
	addChild(m_pInstructions);

	m_pInstructions = new Label("'Right Click' a tile to set the goal tile!", "Consolas");
	m_pInstructions->getTransform()->position = glm::vec2(Config::SCREEN_WIDTH * 0.5f, 560.0f);
	addChild(m_pInstructions);

	m_computeTileCosts();

	ImGuiWindowFrame::Instance().setGUIFunction(std::bind(&PlayScene::GUI_Function, this));
}

void PlayScene::m_buildGrid()
{
	const auto tile_size = Config::TILE_SIZE;

	//add tiles to grid
	for (int row = 0; row < Config::ROW_NUM; ++row)
	{
		for (int col = 0; col < Config::COL_NUM; ++col)
		{
			Tile* tile = new Tile();
			tile->getTransform()->position = glm::vec2(col * tile_size, row * tile_size);
			tile->setGridPosition(col, row);
			addChild(tile);
			tile->addLabels();
			tile->setEnabled(false);
			m_pGrid.push_back(tile);
		}

	}
	//create neighbor references
	for (int row = 0; row < Config::ROW_NUM; ++row)
	{
		for (int col = 0; col < Config::COL_NUM; ++col)
		{
			Tile* tile = m_getTile(col, row);

			//Top most row?
			if (row == 0)
			{
				tile->setNeighbourTile(TOP_TILE, nullptr);
			}
			else
			{
				tile->setNeighbourTile(TOP_TILE, m_getTile(col, row - 1));
			}
			//Right most col?
			if (col == Config::COL_NUM - 1)
			{
				tile->setNeighbourTile(RIGHT_TILE, nullptr);
			}
			else
			{
				tile->setNeighbourTile(RIGHT_TILE, m_getTile(col + 1, row));
			}

			//Bottom most row?
			if (row == Config::ROW_NUM - 1)
			{
				tile->setNeighbourTile(BOTTOM_TILE, nullptr);
			}
			else
			{
				tile->setNeighbourTile(BOTTOM_TILE, m_getTile(col, row + 1));
			}

			//Left most col?
			if (col == 0)
			{
				tile->setNeighbourTile(LEFT_TILE, nullptr);
			}
			else
			{
				tile->setNeighbourTile(LEFT_TILE, m_getTile(col - 1, row));
			}
		}

	}

}

bool PlayScene::m_getGridEnabled() const
{
	return m_isGridEnabled;
}

void PlayScene::m_setGridEnabled(const bool state)
{
	m_isGridEnabled = state;
	for (auto tile : m_pGrid)
	{
		tile->setEnabled(m_isGridEnabled); // enables the tile
		tile->setLabelsEnabled(m_isGridEnabled); // enables the labels
	}
}

void PlayScene::m_computeTileCosts()
{
	float distance = 0.0f;
	float dx = 0.0f;
	float dy = 0.0f;

	//loop through each tile in the grid
	for (auto tile : m_pGrid)
	{
		// f (distance estimate) = g (tile cost) + h (heuristic function)
		switch (m_currentHeuristic)
		{
		case MANHATTAN:
			dx = abs(tile->getGridPosition().x - m_pParking->getGridPosition().x);
			dy = abs(tile->getGridPosition().y - m_pParking->getGridPosition().y);
			distance = dx + dy;
			break;
		case EUCLIDEAN:
			// compute the euclidean distance for each tile to the goal
			distance = Util::distance(tile->getGridPosition(), m_pParking->getGridPosition());
			break;
		}

		tile->setTileCost(distance);
	}
}

Tile* PlayScene::m_getTile(const int col, const int row)
{
	return m_pGrid[(row * Config::COL_NUM) + col];
}

Tile* PlayScene::m_getTile(glm::vec2 grid_position)
{
	const auto col = grid_position.x;
	const auto row = grid_position.y;
	return m_pGrid[(row * Config::COL_NUM) + col];
}


void PlayScene::GUI_Function()
{
	auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);
	// Always open with a NewFrame
	ImGui::NewFrame();

	// See examples by uncommenting the following - also look at imgui_demo.cpp in the IMGUI filter
	//ImGui::ShowDemoWindow();

	ImGui::Begin("Lab  Debug Properties", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);

	ImGui::Separator();

	static bool toggleGrid = false;
	if (ImGui::Checkbox("Toggle Grid", &toggleGrid))
	{
		m_isGridEnabled = toggleGrid;
		m_setGridEnabled(m_isGridEnabled);
	}

	ImGui::Separator();

	// heuristic selection

	static int radio = m_currentHeuristic;
	ImGui::Text("Heuristic Type");
	ImGui::RadioButton("Manhattan", &radio, MANHATTAN);
	ImGui::SameLine();
	ImGui::RadioButton("Euclidean", &radio, EUCLIDEAN);
	if (m_currentHeuristic != radio)
	{
		m_currentHeuristic = static_cast<Heuristic>(radio);
		m_computeTileCosts();
	}

	ImGui::Separator();

	// target properties

	static int start_position[2] = { m_pCar->getGridPosition().x, m_pCar->getGridPosition().y };
	if (ImGui::SliderInt2("Start Position", start_position, 0.0f, Config::COL_NUM - 1))
	{
		if (start_position[1] > Config::ROW_NUM - 1)
		{
			start_position[1] = Config::ROW_NUM - 1;
		}
		m_getTile(m_pCar->getGridPosition())->setTileStatus(UNVISITED);
		m_pCar->getTransform()->position = m_getTile(start_position[0], start_position[1])->getTransform()->position + offset;
		m_pCar->setGridPosition(start_position[0], start_position[1]);
		m_getTile(m_pCar->getGridPosition())->setTileStatus(START);
	}

	static int goal_position[2] = { m_pParking->getGridPosition().x, m_pParking->getGridPosition().y };
	if (ImGui::SliderInt2("Goal Position", goal_position, 0.0f, Config::COL_NUM - 1))
	{
		if (start_position[1] > Config::ROW_NUM - 1)
		{
			start_position[1] = Config::ROW_NUM - 1;
		}
		m_getTile(m_pParking->getGridPosition())->setTileStatus(UNVISITED);
		m_pParking->getTransform()->position = m_getTile(goal_position[0], goal_position[1])->getTransform()->position + offset;
		m_pParking->setGridPosition(goal_position[0], goal_position[1]);
		m_getTile(m_pParking->getGridPosition())->setTileStatus(GOAL);
		m_computeTileCosts();
	}

	ImGui::End();
}
