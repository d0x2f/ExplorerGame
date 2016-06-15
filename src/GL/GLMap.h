#ifndef GLMAP_H
#define GLMAP_H

#include <SDL2/SDL.h>

#include "../types.h"

#include "../GAME/GameState.h"
#include "GLMiniMap.h"
#include "GLShader.h"

class GLEngine;

class GLMap {
public:
	GLMap(GameState *game_state, VideoInfo *video_info, GLEngine *glEngine, Point minimap_pos);
	~GLMap();
	
	void draw();
	GLMiniMap *getMinimap();
	void initialise();
	GLint getHeightAt(int x, int y);
	GLint getHeightAt(Point2D p);
	GLfloat getHeightAt(Point p);
	Point findIntersection(Ray ray);
	GLuint getShaderProgram();
private:
	GameState *game_state;
	VideoInfo *video_info;
	GLEngine *glEngine;
	GLuint map_grid_list;
	GLuint land_texture_id;
	GLuint sand_texture_id;
	GLuint water_texture_id;
	GLuint water_edge_texture_id;
	GLuint vbo, ibo;
	GLuint index_count;
	GLuint vertex_data_size;
	GLMiniMap *minimap;
	Point minimap_pos;
	GLMapTile *empty_tile;
	GLShader *water_shader, *land_shader;
	Uint64 last_draw_time;
	Uint64 time_loop;
	
	std::vector<std::vector<GLMapTile*> > map_board;
	std::vector<std::vector<Colour> > colour_grid;
	std::vector<std::vector<GLint> > height_grid;
	std::vector<std::vector<Point> > normal_grid;
	
	bool initialised;
	
	void generateVBO();
	void loadShaders();
	bool inBounds(GLint x, GLint y);
	bool inBounds(GLint x, GLint y, Size s);
	QuadInts getSquaredHeightsAt(std::vector<std::vector<GLint> > *height_grid, Pointi p);
	GLMapTile *getTile(Pointi p);
	
};

#endif