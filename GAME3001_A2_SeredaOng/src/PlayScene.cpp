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
	TextureManager::Instance().draw("citymap", 0, 0, 0, 255);
	drawDisplayList();

	SDL_SetRenderDrawColor(Renderer::Instance().getRenderer(), 255, 255, 255, 255);
}

void PlayScene::update()
{
	updateDisplayList();
	if (m_carIsMoving)
	{
		m_moveShip();
	}
}

void PlayScene::clean()
{
	removeAllChildren();
}

void PlayScene::handleEvents()
{
	EventManager::Instance().update();

	//QUIT [KEY = ESC]
	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_ESCAPE))
	{
		TheGame::Instance().quit();
	}
	
	//DISPLAY DEBUG [KEY = H]
	if (EventManager::Instance().keyPressed(SDL_SCANCODE_H) && m_getGridEnabled() == 0)
	{
		m_setGridEnabled(1);
	}
	else if (EventManager::Instance().keyPressed(SDL_SCANCODE_H) && m_getGridEnabled() == 1)
	{
		m_setGridEnabled(0);
	}

	//RESET SCENE [KEY = R]
	if (EventManager::Instance().keyPressed(SDL_SCANCODE_R))
	{
		m_resetSimulation();
	}

	//MOVE PLAYER [KEY = M]
	if (EventManager::Instance().keyPressed(SDL_SCANCODE_M))
	{
		if (!m_carIsMoving)
		{
			m_carIsMoving = true;
		}
	}

	//CALCULATE DISTANCE [KEY = F]

	if (EventManager::Instance().keyPressed(SDL_SCANCODE_F))
	{
		m_findShortestPath();
	}

	//SELECT START TILE [LEFT CLICK]
	if (EventManager::Instance().getMouseButton(LEFT) && m_getGridEnabled() == 1 && m_pParking->getGridPosition() != glm::vec2(int(EventManager::Instance().getMousePosition().x / 40),int(EventManager::Instance().getMousePosition().y / Config::TILE_SIZE))
		&& m_getTile(int(EventManager::Instance().getMousePosition().x / Config::TILE_SIZE), int(EventManager::Instance().getMousePosition().y / Config::TILE_SIZE))->getTileStatus() != IMPASSABLE)
	{
		m_getTile(m_pCar->getGridPosition())->setTileStatus(UNVISITED);
		m_pCar->getTransform()->position = m_getTile(int(EventManager::Instance().getMousePosition().x / Config::TILE_SIZE), int(EventManager::Instance().getMousePosition().y / Config::TILE_SIZE))->getTransform()->position + offset;
		m_pCar->setGridPosition(int(EventManager::Instance().getMousePosition().x / Config::TILE_SIZE), int(EventManager::Instance().getMousePosition().y / Config::TILE_SIZE));
		m_getTile(int(EventManager::Instance().getMousePosition().x / Config::TILE_SIZE), int(EventManager::Instance().getMousePosition().y / Config::TILE_SIZE))->setTileStatus(START);
	}

	//SELECT GOAL TILE [RIGHT CLICK]
	if (EventManager::Instance().getMouseButton(RIGHT) && m_getGridEnabled() == 1 && m_pCar->getGridPosition() != glm::vec2(int(EventManager::Instance().getMousePosition().x / Config::TILE_SIZE), int(EventManager::Instance().getMousePosition().y / Config::TILE_SIZE))
		&& m_getTile(int(EventManager::Instance().getMousePosition().x / Config::TILE_SIZE), int(EventManager::Instance().getMousePosition().y / Config::TILE_SIZE))->getTileStatus() != IMPASSABLE)
	{
		m_getTile(m_pParking->getGridPosition())->setTileStatus(UNVISITED);
		m_pParking->getTransform()->position = m_getTile(int(EventManager::Instance().getMousePosition().x / Config::TILE_SIZE), int(EventManager::Instance().getMousePosition().y / Config::TILE_SIZE))->getTransform()->position + offset;
		m_pParking->setGridPosition(int(EventManager::Instance().getMousePosition().x / Config::TILE_SIZE), int(EventManager::Instance().getMousePosition().y / Config::TILE_SIZE));
		m_getTile(m_pParking->getGridPosition())->setTileStatus(GOAL);
		m_computeTileCosts();
	}

}

void PlayScene::start()
{

	// Set GUI Title
	m_guiTitle = "Play Scene";

	TextureManager::Instance().load("../Assets/Textures/citymap.jpg", "citymap");

	SoundManager::Instance().load("../Assets/audio/citymusic.mp3", "citymusic", SOUND_MUSIC);
	SoundManager::Instance().playMusic("citymusic", -1, 0);
	SoundManager::Instance().setMusicVolume(5);
	//setup the grid
	m_buildGrid();
	m_currentHeuristic = MANHATTAN;

	m_pParking = new Target();
	m_pParking->setGridPosition(15, 8);
	m_pParking->getTransform()->position = m_getTile(m_pParking->getGridPosition().x, m_pParking->getGridPosition().y)->getTransform()->position + offset;
	m_getTile(m_pParking->getGridPosition().x, m_pParking->getGridPosition().y)->setTileStatus(GOAL);
	addChild(m_pParking);

	m_pCar = new Car();
	m_pCar->setGridPosition(1, 3);
	m_pCar->getTransform()->position = m_getTile(m_pCar->getGridPosition().x, m_pCar->getGridPosition().y)->getTransform()->position + offset;
	m_getTile(m_pCar->getGridPosition().x, m_pCar->getGridPosition().y)->setTileStatus(START);
	addChild(m_pCar);

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

void PlayScene::m_findShortestPath()
{
	// check if pathList is empty
	if (m_pPathList.empty())
	{
		// Step 1. Add Start Position
		Tile* start_tile = m_getTile(m_pCar->getGridPosition());
		start_tile->setTileStatus(OPEN);
		m_pOpenList.push_back(start_tile);

		bool goal_found = false;

		// Step 2. Loop until the OpenList is empty or the Goal is found
		while (!m_pOpenList.empty() && !goal_found)
		{
			// initialization
			auto min = INFINITY;
			Tile* min_tile;
			int min_tile_index = 0;
			int count = 0;
			std::vector<Tile*> neighbour_list;

			// Step 2.a - Get together the neighbours to check
			// loop through each neighbour in right-winding order (Top - Right - Bottom - Left)
			for (int index = 0; index < NUM_OF_NEIGHBOUR_TILES; ++index)
			{
				const auto neighbour = m_pOpenList[0]->getNeighbourTile(static_cast<NeighbourTile>(index));
				if (neighbour == nullptr || neighbour->getTileStatus() == IMPASSABLE)
				{
					continue; // ignore neighbours that are inappropriate
				}
				neighbour_list.push_back(neighbour);
			}

			// Step 2.b - for every neighbour in the neighbour list
			for (auto neighbour : neighbour_list)
			{
				// Step 2.b1 - check if the neighbour is not the goal
				if (neighbour->getTileStatus() != GOAL)
				{
					// check if neighbour tile cost is less than minimum found so far...
					if (neighbour->getTileCost() < min)
					{
						min = neighbour->getTileCost();
						min_tile = neighbour;
						min_tile_index = count;
					}
					count++;
				}
				else // neighbour is the goal tile
				{
					min_tile = neighbour;
					m_pPathList.push_back(min_tile);
					goal_found = true;
					break;
				}
			}

			// Step 2.c - remove the reference of the current tile in the open list
			m_pPathList.push_back(m_pOpenList[0]); // add the top of the open list to the path_list
			m_pOpenList.pop_back(); // empties the open list

			// Step 2.d - add the min_tile to the openList
			m_pOpenList.push_back(min_tile);
			min_tile->setTileStatus(OPEN);
			neighbour_list.erase(neighbour_list.begin() + min_tile_index); // remove the min_tile from the neighbour list

			// Step 2.e - push all remaining neighbours onto the closed list
			for (auto neighbour : neighbour_list)
			{
				if (neighbour->getTileStatus() == UNVISITED)
				{
					neighbour->setTileStatus(CLOSED);
					m_ClosedList.push_back(neighbour);
				}
			}
		}

		// Alex's hack - to correct the algorithm
		Tile* goal = m_pPathList.at(m_pPathList.size() - 2);
		m_pPathList.erase(m_pPathList.end() - 2);
		m_pPathList.push_back(goal);

		m_displayPathList();
	}
}

void PlayScene::m_displayPathList()
{
	for (auto tile : m_pPathList)
	{
		std::cout << "(" << tile->getGridPosition().x << ", " << tile->getGridPosition().y << ")" << std::endl;
	}
	std::cout << "Path Length" << m_pPathList.size() << std::endl;
}

void PlayScene::m_resetPathfinding()
{
	//clear the tile vectors
	m_pPathList.clear();
	m_pPathList.shrink_to_fit();
	m_pOpenList.clear();
	m_pOpenList.shrink_to_fit();
	m_ClosedList.clear();
	m_ClosedList.shrink_to_fit();

	//reset tile statuses
	for (auto tile : m_pGrid)
	{
		tile->setTileStatus(UNVISITED);
	}

	m_getTile(m_pParking->getGridPosition())->setTileStatus(GOAL);
	goal_position[0] = m_pParking->getGridPosition().x;
	goal_position[1] = m_pParking->getGridPosition().y;
	m_getTile(m_pCar->getGridPosition())->setTileStatus(START);
	start_position[0] = m_pCar->getGridPosition().x;
	start_position[1] = m_pCar->getGridPosition().y;
	moveCounter = 0;
	m_carIsMoving = false;

}

void PlayScene::m_resetSimulation()
{
	auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);
	// clear current status of ship and target tiles
	m_getTile(m_pParking->getGridPosition())->setTileStatus(UNVISITED);
	m_getTile(m_pCar->getGridPosition())->setTileStatus(UNVISITED);

	// move target back to starting location
	m_pParking->getTransform()->position = m_getTile(15, 8)->getTransform()->position + offset;
	m_pParking->setGridPosition(15.0f, 8.0f);
	m_getTile(15, 8)->setTileStatus(GOAL);
	goal_position[0] = m_pParking->getGridPosition().x;
	goal_position[1] = m_pParking->getGridPosition().y;

	// move spaceship back to starting location
	m_pCar->getTransform()->position = m_getTile(1, 3)->getTransform()->position + offset;
	m_pCar->setGridPosition(1.0f, 3.0f);
	m_getTile(1, 3)->setTileStatus(START);
	start_position[0] = m_pCar->getGridPosition().x;
	start_position[1] = m_pCar->getGridPosition().y;
	m_resetPathfinding();
}

void PlayScene::m_moveShip()
{
	auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);
	if (moveCounter < m_pPathList.size())
	{
		auto curret_path_tile_position = m_pPathList[moveCounter]->getGridPosition();
		m_pCar->getTransform()->position = m_getTile(curret_path_tile_position)->getTransform()->position + offset;
		m_pCar->setGridPosition(curret_path_tile_position.x, curret_path_tile_position.y);
		if (Game::Instance().getFrames() % 20 == 0)
		{
			moveCounter++;
		}
	}
	else
	{
		m_carIsMoving = false;
	}
}


void PlayScene::GUI_Function()
{
	// Always open with a NewFrame
	ImGui::NewFrame();

	// See examples by uncommenting the following - also look at imgui_demo.cpp in the IMGUI filter
	//ImGui::ShowDemoWindow();

	ImGui::Begin("Lab  Debug Properties", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);

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

	start_position[0] = m_pCar->getGridPosition().x;
	start_position[1] = m_pCar->getGridPosition().y;
	if (ImGui::SliderInt2("Start Position", start_position, 0, Config::COL_NUM - 1))
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

	goal_position[0] = m_pParking->getGridPosition().x;
	goal_position[1] = m_pParking->getGridPosition().y;
	if (ImGui::SliderInt2("Goal Position", goal_position, 0, Config::COL_NUM - 1))
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

int PlayScene::start_position[2];
int PlayScene::goal_position[2];