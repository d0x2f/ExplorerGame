#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <pthread.h>

#include "../types.h"

class GameEngine;
class GameState;

class GameObject {
public:
	GameObject(GameEngine *gEngine, GameState *game_state, int id, GAME_OBJECT_TYPE type, Point position);
	~GameObject();
	
	void initialise();

	void doTick();
	
	//Thread accessible
	Point getPosition();
	GLfloat getRotation();
	bool isMovable();
	bool isSelectable();
	GLfloat getMoveSpeed();
	void setPosition(Point p);
	void setMovable(bool b);
	void setSelectable(bool b);
	void setMoveSpeed(GLfloat f);
	void setRotation(GLfloat f);
	int getId();
	
	GAME_OBJECT_TYPE getType();
	void moveTo(Point p);
	Point getMoveTo();
private:
	pthread_mutex_t object_mutex;
	GameEngine *gEngine;
	GameState *game_state;
	int id;
	bool moving;
	Uint64 last_draw_time;
	
	//Thread accessible
	GAME_OBJECT_TYPE type;
	Point position;
	GLfloat rot;
	bool movable;
	bool selectable;
	GLfloat move_speed;
	Point moving_toward;
};

#endif