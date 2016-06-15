#include "../globals.h"

#include "../GAME/MapTile.h"
#include "../GAME/GameState.h"

MapTile::MapTile(GameState *game_state, Pointi pos, QuadInts heights) : game_state(game_state), pos(pos), heights(heights) {
}

MapTile::~MapTile() {
}

QuadInts MapTile::getHeights() {
	return heights;
}

/**
 * Return value of (0,0,-100) signals a negative result.
 */
Point MapTile::findIntersection(Ray ray) {
	Point p = Point((GLfloat)pos.x, (GLfloat)pos.y);
	
	Point a = Point(p.x*TILE_SIZE, p.y*TILE_SIZE, (GLfloat)heights.a);
	Point b = Point(p.x*TILE_SIZE, p.y*TILE_SIZE+TILE_SIZE, (GLfloat)heights.b);
	Point c = Point(p.x*TILE_SIZE+TILE_SIZE, p.y*TILE_SIZE+TILE_SIZE, (GLfloat)heights.c);
	Point d = Point(p.x*TILE_SIZE+TILE_SIZE, p.y*TILE_SIZE, (GLfloat)heights.d);
	Point e = Point(p.x*TILE_SIZE+TILE_SIZE/2.0f, p.y*TILE_SIZE+TILE_SIZE/2.0f, heights.middle());
	
	p = findTriangleIntersection(ray, a, d, e);
	if(p.z != -100.0f)
		return p;
	
	p = findTriangleIntersection(ray, d, c, e);
	if(p.z != -100.0f)
		return p;
	
	p = findTriangleIntersection(ray, c, b, e);
	if(p.z != -100.0f)
		return p;
	
	p = findTriangleIntersection(ray, b, a, e);
	return p;
}

/**
 * Return value of (0,0,-100) signals a negative result.
 */
Point MapTile::findTriangleIntersection(Ray ray, Point a, Point b, Point c) {
	Point v_ab = b-a;
	Point v_ac = c-a;
	
	Point cross = ray.dir.cross(v_ac);
	GLfloat dot = v_ab.dot(cross);
	
	//If dot == 0 allowing for float inaccuracy
	if(dot > -0.00001 && dot < 0.00001)
		return Point(0.0f, 0.0f, -100.0f);
	
	GLfloat invdot = 1.0f/dot;
	Point dir_a_r = ray.a - a;
	GLfloat u = dir_a_r.dot(cross) * invdot;
	
	if(u < 0.0f || u > 1.0f)
		return Point(0.0f, 0.0f, -100.0f);
	
	cross = dir_a_r.cross(v_ab);
	GLfloat v = ray.dir.dot(cross) * invdot;
	
	if(v < 0.0f || u+v > 1.0f)
		return Point(0.0f, 0.0f, -100.0f);
	
	GLfloat t = v_ac.dot(cross) * invdot;
	
	//Not interested in intersections behind the ray
	if(t < 0.0f)
		return Point(0.0f, 0.0f, -100.0f);
	
	return ray.a + ray.dir*t;
}