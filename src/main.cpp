#include <signal.h>
#include <iostream>
#include <ctime>
#include <stdlib.h>
//#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "GL/GLEngine.h"
#include "GAME/GameEngine.h"
#include "GAME/GameState.h"
#include "globals.h"

void quit(int status = 0);
void *runGameEngine(void *args);
void handler(int sig);

GameState *game_state;
GLEngine *gl_engine;

struct ThreadArgs {
	GameState *game_state;
	Uint32 game_event_id;
	Uint32 GL_event_id;
};

/**
 * This is where the magic happens.
 */
int main(int argc, char **argv) {
	pthread_t game_thread;

	srand (static_cast <unsigned> (time(0)));
	
	//Run a cleanup function on quit
	signal(SIGABRT, quit);
	signal(SIGTERM, quit);
	signal(SIGSEGV, handler);
	
	//Enable X11 multi threaded support.
	//XInitThreads();
	
	//The game state contains all the abstracted information about that game.
	//The game engine and the GL engine each get a reference to do with as needed.
	GameState *game_state = new GameState();
	
	ThreadArgs args;
	args.game_state = game_state;
	args.game_event_id = SDL_RegisterEvents(2);
	args.GL_event_id = args.game_event_id+1;
	
	//The GL engine will run in the main thread.
	//It will simply render the game state as a 3d environment.
	gl_engine = new GLEngine(game_state, args.game_event_id, args.GL_event_id);
	
	//Initialise SQL and GL scene.
	if(gl_engine->initialise()) {
		
		//The game engine will run in it's own thread.
		//It will manage the state of the game and compute positions etc.
		//(Must run after gl_engine.initialise()!!
		
		int ret = pthread_create( &game_thread, NULL, &runGameEngine, (void *)&args);
		if(ret != 0) {
			std::cout << "Couldn't create game thread, Error number: " << ret << std::endl;
			quit(EXIT_FAILURE);
			return EXIT_FAILURE;
		}
		
	
		//After the game engine has started, run the GL engine.
		gl_engine->run();
		
		//if(game_thread.joinable())
		//	game_thread.join();
		pthread_join(game_thread, NULL);
		
		quit(EXIT_SUCCESS);
	} else {
		quit(EXIT_FAILURE);
	}
	
	return EXIT_SUCCESS;
}

/**
 * Starts the GL engine in a new thread.
 */
void *runGameEngine(void *args) {
	ThreadArgs *casted_args = (ThreadArgs *)args;
	GameEngine game_engine(casted_args->game_state, casted_args->game_event_id, casted_args->GL_event_id);

	game_engine.initialise();
	game_engine.run();
	
	return NULL;
}

/**
 * Performs last minute clean up.
 */
void quit(int status) {
	delete gl_engine;
	
	#if DEBUG == 1
	printf("\nQuitting SDL...\n");
	#endif

	SDL_Quit();
	
	#if DEBUG == 1
	printf("Exiting...\n");
	#endif

	exit(status);
}

void handler(int sig) {
	//void *array[10];
	//size_t size;
	
	// get void*'s for all entries on the stack
	//size = backtrace(array, 10);
	
	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	//backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}