#include "../globals.h"

#include "GameObject.h"
#include "GameEngine.h"
#include "GameState.h"
#include "Map.h"

GameObject::GameObject(GameEngine *gEngine, GameState *game_state, int id, GAME_OBJECT_TYPE type, Point position) : gEngine(gEngine), game_state(game_state), id(id), type(type), position(position) {
	object_mutex = PTHREAD_MUTEX_INITIALIZER;

	selectable = true;
	movable = false;
	move_speed = 0;
	last_draw_time = 0;
	rot = 0.0f;
	moving = false;
}

GameObject::~GameObject() {
}

void GameObject::initialise() {

}

GAME_OBJECT_TYPE GameObject::getType() {
	pthread_mutex_lock(&object_mutex);
	GAME_OBJECT_TYPE t = type;
	pthread_mutex_unlock(&object_mutex);

	return t;
}

Point GameObject::getPosition() {
	pthread_mutex_lock(&object_mutex);
	Point p = position;
	pthread_mutex_unlock(&object_mutex);
	
	return p;
}

GLfloat GameObject::getRotation() {
	pthread_mutex_lock(&object_mutex);
	GLfloat f = rot;
	pthread_mutex_unlock(&object_mutex);
	
	return f;
}

bool GameObject::isMovable() {
	pthread_mutex_lock(&object_mutex);
	bool b = movable;
	pthread_mutex_unlock(&object_mutex);
	
	return b;
}

bool GameObject::isSelectable() {
	pthread_mutex_lock(&object_mutex);
	bool b = selectable;
	pthread_mutex_unlock(&object_mutex);
	
	return b;
}

GLfloat GameObject::getMoveSpeed() {
	pthread_mutex_lock(&object_mutex);
	GLfloat f = move_speed;
	pthread_mutex_unlock(&object_mutex);
	
	return f;
}

Point GameObject::getMoveTo() {
	pthread_mutex_lock(&object_mutex);
	Point p = moving_toward;
	pthread_mutex_unlock(&object_mutex);
	
	return p;
}

int GameObject::getId() {
	pthread_mutex_lock(&object_mutex);
	int i = id;
	pthread_mutex_unlock(&object_mutex);
	
	return i;
}

void GameObject::setMovable(bool b) {
	pthread_mutex_lock(&object_mutex);
	movable = b;
	pthread_mutex_unlock(&object_mutex);
}

void GameObject::setSelectable(bool b) {
	pthread_mutex_lock(&object_mutex);
	selectable = b;
	pthread_mutex_unlock(&object_mutex);
}

void GameObject::setMoveSpeed(GLfloat f) {
	pthread_mutex_lock(&object_mutex);
	move_speed = f;
	pthread_mutex_unlock(&object_mutex);
}

void GameObject::setPosition(Point p) {
	pthread_mutex_lock(&object_mutex);
	position = p;
	pthread_mutex_unlock(&object_mutex);
}

void GameObject::setRotation(GLfloat f) {
	pthread_mutex_lock(&object_mutex);
	rot = f;
	pthread_mutex_unlock(&object_mutex);
}

void GameObject::moveTo(Point p) {
	pthread_mutex_lock(&object_mutex);
	moving_toward = p;
	pthread_mutex_unlock(&object_mutex);
	
	moving = true;
}

void GameObject::doTick() {
	if(last_draw_time != 0) {
			
		if(moving) {
			GLfloat time_diff = ((GLfloat)(gEngine->getTimeUs() - last_draw_time));
			
			Point current_position = getPosition();
			Point move_to = getMoveTo();
			
			Point vector = move_to - current_position;
			vector.normalise();
			vector *= getMoveSpeed()*time_diff/1000000.0f;
			
			if(vector.length() >= (move_to - current_position).length() || (vector.x == 0.0f && vector.y==0.0f)) {
				setPosition(move_to);
				moving = false;
			} else {
				Point p = vector + current_position;
				p.z = game_state->getMap()->getHeightAt(p);
				
				if(p.z < -0.4f) {
					moving = false;
				} else {
					setPosition(p);
					
					if(vector.x == 0.0f) {
						setRotation(90.0f * ((vector.y > 0) - (vector.y < 0)));
					} else {
						if(vector.x >= 0)
							setRotation(fmod(atan(vector.y/vector.x) + (PI/2.0f), 2*PI));
						else
							setRotation(fmod(atan(vector.y/vector.x) + (3.0f*PI/2.0f), 2*PI));
					}
				}
			}
		}
	}
	
	last_draw_time = gEngine->getTimeUs();
	
	setRotation(rot);
}