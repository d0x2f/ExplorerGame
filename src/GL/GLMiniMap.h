#ifndef GLMINIMAP_H
#define GLMINIMAP_H

#include "../types.h"

#include "GLQuad.h"
#include "../GAME/GameState.h"

class GLMiniMap {
public:
	GLMiniMap(Size size, Point pos, VideoInfo *video_info, GameState *game_state, std::vector< std::vector<GLint> > *height_grid);
	~GLMiniMap();
	
	void draw(Point p);
	void initialise();
	Point checkClick(Point p);
private:
	Size size;
	Point pos;
	VideoInfo *video_info;
	GameState *game_state;
	std::vector< std::vector<GLint> > *height_grid;
	GLuint texture_id;
	GLQuad *minimap_quad;
	
	struct _Colour {
		GLfloat r, g, b;
	};
	
	void loadVBO();
};

#endif