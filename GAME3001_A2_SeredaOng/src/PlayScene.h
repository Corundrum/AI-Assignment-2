#pragma once
#ifndef __PLAY_SCENE__
#define __PLAY_SCENE__

#include "Car.h"
#include "Heuristic.h"
#include "Scene.h"
#include "Label.h"
#include "Target.h"
#include "Tile.h"

class PlayScene : public Scene
{
public:
	PlayScene();
	~PlayScene();

	// Scene LifeCycle Functions
	virtual void draw() override;
	virtual void update() override;
	virtual void clean() override;
	virtual void handleEvents() override;
	virtual void start() override;
private:
	// IMGUI Function
	void GUI_Function();
	std::string m_guiTitle;

	glm::vec2 m_mousePosition;

	// Game Objects
	Target* m_pParking;
	Car* m_pCar;
	Label* m_pInstructions;

	//Pathfinding objects and functions
	std::vector<Tile*> m_pGrid;
	bool m_isGridEnabled;

	glm::vec2 offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);

	//calculate shortest path and related methods
	void m_findShortestPath();
	void m_displayPathList();
	void m_resetPathfinding();
	void m_resetSimulation();

	//tiles lists for pathfinding
	std::vector<Tile*> m_pOpenList;
	std::vector<Tile*> m_ClosedList;
	std::vector<Tile*> m_pPathList;

	void m_buildGrid();
	bool m_getGridEnabled() const;
	void m_setGridEnabled(bool state);
	void m_computeTileCosts();

	//convenience functions
	Tile* m_getTile(int col, int row);
	Tile* m_getTile(glm::vec2 grid_position);

	//Heuristic
	Heuristic m_currentHeuristic;

	//Ship movement
	int moveCounter = 0;
	bool m_shipIsMoving = false;
	void m_moveShip();

	static int start_position[2];
	static int goal_position[2];
};

#endif /* defined (__PLAY_SCENE__) */