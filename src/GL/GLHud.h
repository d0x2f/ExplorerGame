#ifndef HUD_H
#define HUD_H

#include "../GAME/GameState.h"
#include "GLQuad.h"

class GLEngine;

class GLHud {
public:
	GLHud(GameState *game_state, VideoInfo *video_info, GLEngine *glEngine);
	~GLHud();
	
	void draw();
	void initialise();
	void resize();
private:
	GameState *game_state;
	GLQuad *left_hud, *middle_hud, *right_hud;
	VideoInfo *video_info;
	GLEngine *glEngine;
	GLuint hud_list;
	Point crossheir_size;
	Point window_center;
	
	void setupQuads();
	void clean();
	void precomputeValues();
};

#endif