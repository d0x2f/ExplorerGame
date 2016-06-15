#include "../globals.h"

#include "GameState.h"
#include "Map.h"

GameState::GameState() {
	vars_mutex = PTHREAD_MUTEX_INITIALIZER;
	objects_mutex = PTHREAD_MUTEX_INITIALIZER;

	setVar(EXIT_GAME, 0);
	setVar(GAME_STAGE, IN_MENU);
	setVar(MAP_SIZE, 128);
}

GameState::~GameState() {
	if(map)
		delete map;
}

void GameState::newGame(GameEngine *gEngine) {
	if(map)
		delete map;
	
	map = new Map(this, Size(getVar(MAP_SIZE), getVar(MAP_SIZE)));
	map->generateNewRandomMap();
	
	std::vector<Point> potential_spots = map->getBuildableSquares();
	
	lockObjects();
	
	GameObject *go;
	
	//Some random objects for now
	for(int i=0; i<10; i++) {
		go = new GameObject(gEngine, this, getNewId(), HOUSE, potential_spots[rand()%potential_spots.size()]);
		game_objects.insert(std::make_pair(go->getId(), go));
		
		go = new GameObject(gEngine, this, getNewId(), MALE_PERSON, potential_spots[rand()%potential_spots.size()]);
		go->setMovable(true);
		go->setMoveSpeed(PERSON_SPEED);
		go->initialise();

		game_objects.insert(std::make_pair(go->getId(), go));
	}
	
	for(int i=0; i<100; i++) {
		go = new GameObject(gEngine, this, getNewId(), TREE, potential_spots[rand()%potential_spots.size()]);
		go->setSelectable(false);
		go->initialise();

		game_objects.insert(std::make_pair(go->getId(), go));
	}
	
	unlockObjects();
}

int GameState::getVar(VarNames var_name) {
	int ret;
	
	pthread_mutex_lock(&vars_mutex);
	ret = vars[var_name];
	pthread_mutex_unlock(&vars_mutex);
	
	return ret;
}

void GameState::setVar(VarNames var_name, int value) {
	pthread_mutex_lock(&vars_mutex);
	vars[var_name] = value;
	pthread_mutex_unlock(&vars_mutex);
}

Map *GameState::getMap() {
	return map;
}

Size GameState::getMapSize() {
	int size = getVar(MAP_SIZE);
	return Size(size, size);
}

void GameState::lockObjects() {
	pthread_mutex_lock(&objects_mutex);
}

std::map<int, GameObject *> *GameState::getObjects() {
	return &game_objects;
}

void GameState::unlockObjects() {
	pthread_mutex_unlock(&objects_mutex);
}