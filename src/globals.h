#ifndef GLOBALS_H
#define GLOBALS_H

#include "./GL/gl_core_3_1.h"
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <iostream>

#define DEBUG 1
#define TILE_SIZE 1
#define WINDOW_SPEED 1
#define PERSON_SPEED 1
#define PI 3.1415926535897f
#define TICK_RATE 100
#define TICK_RATE_USECS (1000000/TICK_RATE)

extern bool r_debug;

enum USEREVENTS {
	OBJECT_SELECTED,
	GAME_THREAD_LOADED
};

enum GAME_OBJECT_TYPE {
	MALE_PERSON,
	HOUSE,
	TREE
};

void checkSDLError(int line = -1);
void checkOGLError(const char *file, int line = -1);

void emit(Uint32 event_id, USEREVENTS event_type, void *data);
#endif