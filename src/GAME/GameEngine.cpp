#include <iostream>
#include <unistd.h>

#include "../types.h"
#include "../globals.h"

#include "../GAME/GameState.h"
#include "../GAME/GameEngine.h"

GameEngine::GameEngine(GameState *game_state, Uint32 game_event_id, Uint32 GL_event_id) : game_state(game_state), game_event_id(game_event_id), GL_event_id(GL_event_id) {
}

GameEngine::~GameEngine() {
}

void GameEngine::initialise() {
	srand (static_cast <unsigned> (time(0)));
	last_tick_time = getTimeUs();
	game_state->newGame(this);
	emit(game_event_id, GAME_THREAD_LOADED, NULL);
}

void GameEngine::run() {
	SDL_Event event;
	
	while(game_state->getVar(GameState::EXIT_GAME) != 1) {
			
		last_tick_time = getTimeUs();
		doTick();
	
		while((game_state->getVar(GameState::EXIT_GAME) != 1) && 
			SDL_PeepEvents(&event, 1, SDL_GETEVENT, GL_event_id, GL_event_id) > 0) {

			if(event.type == GL_event_id) {
				handleGLThreadEvent(&event);
			}
			
			SDL_PumpEvents();
		}
		
		Uint64 current_time = getTimeUs();
		Uint64 time_diff = current_time - last_tick_time;
		if(time_diff < TICK_RATE_USECS)
			usleep(TICK_RATE_USECS - time_diff);
	}
}

/**
 * Handle game events like menu button presses and sound activations
 */
void GameEngine::handleGLThreadEvent(SDL_Event *event) {
	switch((USEREVENTS)event->user.code) {
		case OBJECT_SELECTED:
			//std::cout << "Object Selected, id: " << (int)event->user.data1 << std::endl;
			break;
			
		default:
			break;
	}
}

void GameEngine::doTick() {
	game_state->lockObjects();
	std::map<int, GameObject *> *objects = game_state->getObjects();
	
	for(std::map<int, GameObject *>::iterator it=objects->begin(); it!=objects->end(); it++) {
		it->second->doTick();
	}
	
	game_state->unlockObjects();
}

Uint64 GameEngine::getTimeUs() {
	timeval t;
	gettimeofday(&t, NULL);
	return ((unsigned long long)t.tv_sec * 1000000) + t.tv_usec;
}