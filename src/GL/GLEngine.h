#ifndef GLENGINE_H
#define GLENGINE_H

#include <SDL2/SDL_image.h>
#include <FTGL/ftgl.h>
#include <map>

#include "../GAME/GameState.h"
#include "../types.h"
#include "../globals.h"
#include "GLHud.h"
#include "GLMap.h"
#include "GLMesh.h"
#include "GLGameObject.h"
#include "GLMeshGroup.h"

#define KEY_UP		1
#define KEY_LEFT 	2
#define KEY_DOWN	4
#define KEY_RIGHT	8
#define KEY_SHIFT	16
#define KEY_CTRL	32

class FTGLPixmapFont;

class GLEngine {
public:
	GameState *game_state;

	GLEngine(GameState *game_state, Uint32 game_event_id, Uint32 GL_event_id);
	~GLEngine();
	
	bool initialise();
	void run();
	
	GLuint loadTexture(const char *filename);
	static GLfloat randomFloat();
	bool isVisible(Point p);
	bool isVisible(Point p, GLfloat radius);
	GLMap *getMap();
	
	Uint64 getTimeUs();
	
	GLfloat *getModelViewMatrix();
	GLfloat *getProjectionMatrix();
	Point getCameraPosition();
	GLShader *getGeneralShader();
private:
	bool exit_loop;
	const Uint32 game_event_id, GL_event_id;
	Uint64 last_frame_time;
	FTGLPixmapFont *font;
	Size lastWindowSize;
	bool inFullscreen;
	bool minimapFollowMouse;
	bool initialised;
	bool loading;
	
	//SDL contexts
	SDL_Window *window;
	SDL_GLContext gl_context;
	SDL_Renderer *renderer; 
	
	//OpenGL related variables
	float fps;
	GLfloat frustum[6][4];
	GLfloat projection[16];
	GLfloat modelview[16];
	GLuint uniform_buffer_id, matrices_block_index;
	GLShader *general_shader;
	
	GLdouble modelMatrix[16];
	GLdouble projectionMatrix[16];

	//On screen game components
	GLHud *hud;
	GLMap *map;
	Point camera_position;
	Uint8 keys;
	Point minimap_pos;
	GLMiniMap *minimap;
	std::map<int, GLGameObject *> game_objects;
	std::map<GAME_OBJECT_TYPE, GLMeshGroup *> mesh_groups;
	int selected_object;
	
#if DEBUG == 1
	Ray click_ray;
#endif
	
	//Testing
	std::map<GAME_OBJECT_TYPE, GLMesh *> meshes;
	
	GLuint cursor_texture;
	GLQuad *cursor;
	Point cursor_position;
	
	//Texture cache
	std::map<std::string, GLuint> textures;
	
	//Pre computed values
	VideoInfo *video_info;
	GLfloat camera_move_factor;
	
	void handleKeypress(SDL_Event *event);
	void handleKeyrelease(SDL_Event *event);
	void handleMouseWheel(SDL_Event *event);
	void handleMouseButtonDown(SDL_Event *event);
	void handleMouseButtonUp(SDL_Event *event);
	void handleMouseMotion(SDL_Event *event);
	void handleGameThreadEvent(SDL_Event *event);
	
	bool initSDL();
	bool initGL();
	bool initGame();
	void setPerspective();
	void updateMatrices();
	void loadShader();
	void precomputeValues();
	void drawFrame();
	void drawLoadingScreen();
	void drawHUD();
	void drawMap();
	void moveCameraPosition();
	void lookAt(Point p);
	void windowResize(Size s);
	void exitFullscreen();
	void enterFullscreen();
	void extractFrustum();
	Ray getClickedRay(Point2D p);
	Point getClickedPosition(Point2D p);
	void clickSelect(Point2D p);
};

#endif