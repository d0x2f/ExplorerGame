#ifndef GLMAPTILE_H
#define GLMAPTILE_H

#include <vector>

#include "../GAME/GameState.h"
#include "GLQuad.h"
#include "GLEngine.h"

class GLMapTile {
public:
	GLMapTile(GameState *game_state, VideoInfo *video_info, Pointi pos, std::vector< std::vector<Colour> > *colour_grid,  std::vector< std::vector<Point> > *normal_grid, QuadInts heights);
	~GLMapTile();
	
	void initialise();
	bool isVisible(GLEngine *glEngine);
	
	const std::vector<GLfloat> getData();
	QuadInts getHeights();
	Point findIntersection(Ray ray);
private:
	GameState *game_state;
	VideoInfo *video_info;
	Pointi pos;
	
	std::vector< std::vector<Colour> > *colour_grid;
	std::vector<std::vector<Point> > *normal_grid;
	
	QuadInts heights;
	
	std::vector<GLfloat> data;
	
	Point findTriangleIntersection(Ray ray, Point a, Point b, Point c);
};

#endif