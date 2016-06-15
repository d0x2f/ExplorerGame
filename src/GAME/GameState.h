#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <pthread.h>
#include <map>

#include "../types.h"

#include "../GAME/GameObject.h"

class Map;

class GameState {
public:
	enum VarNames {
		EXIT_GAME,
		GAME_STAGE,
		MAP_SIZE
	};
	
	enum GameStage {
		IN_MENU,
		IN_GAME
	};
	
	GameState();
	~GameState();
	
	int getVar(VarNames var_name);
	void setVar(VarNames var_name, int value);
	void newGame(GameEngine *gEngine);
	Size getMapSize();
	
	void lockObjects();
	std::map<int, GameObject *> *getObjects();
	void unlockObjects();
	
	Map *getMap();
private:
	pthread_mutex_t vars_mutex, objects_mutex, map_mutex;
	
	std::map<VarNames, int> vars;
	std::map<int, GameObject *> game_objects;
	
	Map *map;
};

#endif