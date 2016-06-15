#ifndef MAPTILE_H
#define MAPTILE_H

#include "../types.h"

class GameState;

class MapTile {
public:
	MapTile(GameState *game_state, Pointi pos, QuadInts heights);
	~MapTile();
	
	QuadInts getHeights();
	Point findIntersection(Ray ray);
private:
	GameState *game_state;
	Pointi pos;
	QuadInts heights;
	
	Point findTriangleIntersection(Ray ray, Point a, Point b, Point c);
};

#endif