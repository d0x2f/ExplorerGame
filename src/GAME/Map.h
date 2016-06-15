#ifndef MAP_H
#define MAP_H

#include "../types.h"

class GameState;

class Map {
public:
	Map(GameState *game_state, Size size);
	~Map();
	
	void generateNewRandomMap();
	std::vector<std::vector<GLint> > getHeights();
	std::vector<Point> getBuildableSquares();
	GLfloat getHeightAt(Point p);
private:
	std::vector<std::vector<MapTile*> > map_board;
	std::vector<std::vector<Colour> > colour_grid;
	std::vector<std::vector<GLint> > height_grid;
	std::vector<Point> buildableSquares;
	GameState *game_state;
	Size size;
	
	bool inBounds(GLint x, GLint y);
	bool inBounds(GLint x, GLint y, Size s);
	QuadInts getSquaredHeightsAt(std::vector<std::vector<GLint> > *height_grid, Pointi p);
};

#endif