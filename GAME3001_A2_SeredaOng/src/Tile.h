#pragma once
#ifndef __TILE__
#define __TILE__

#include "Label.h"
#include "NavigationObject.h"
#include "NeighbourTile.h"
#include "TileStatus.h"

class Tile : public NavigationObject
{
public:
	//constructor
	Tile();
	//destructor
	~Tile();
	//life-cycle functions
	void draw() override;
	void update() override;
	void clean() override;

	//get
	Tile* getNeighbourTile(NeighbourTile position);
	float getTileCost() const;
	TileStatus getTileStatus() const;

	//set
	void setNeighbourTile(NeighbourTile position, Tile* tile);
	void setTileCost(float cost);
	void setTileStatus(TileStatus status);
	void addLabels();
	void setLabelsEnabled(bool state);

private:
	float m_cost;
	TileStatus m_status;

	Label* m_costLabel;
	Label* m_statusLabel;

	Tile* m_neighbours[NUM_OF_NEIGHBOUR_TILES];
};

#endif // defined (__TILE__)
