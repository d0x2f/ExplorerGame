#ifndef GLGAMEOBJECT_H
#define GLGAMEOBJECT_H

#include <SDL2/SDL.h>

#include "../GAME/GameState.h"
#include "GLMeshInstance.h"

class GLEngine;

class GLGameObject {
public:
	GLGameObject(GameState *game_state, int id, GameObject *go, GLMesh *mesh, Point pos, GLEngine *glEngine);
	~GLGameObject();
	
	void initialise();
	bool isVisible();
	int getId();
	void draw(bool selected = false);
	GLfloat getSelectionRadius();
	bool intersects(Ray r);
	bool isSelectable();
	bool isMovable();
	GLMeshInstance *getMeshInstance();
	void moveTo(Point p);
private:
	GameState *game_state;
	int id;
	GameObject *go;
	GLMesh *mesh;
	GLMeshInstance *mesh_instance;
	GLEngine *glEngine;
};

#endif