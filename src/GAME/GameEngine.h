#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <SDL2/SDL.h>
#include <sys/time.h>

class GameState;

class GameEngine {
public:
	GameState *game_state;
	
	GameEngine(GameState *game_state, Uint32 game_event_id, Uint32 GL_event_id);
	~GameEngine();
	
	void initialise();
	void run();
	
	Uint64 getTimeUs();
private:
	Uint64 last_tick_time;
	const Uint32 game_event_id, GL_event_id;
	
	void handleGLThreadEvent(SDL_Event *event);
	void doTick();
};

#endif