#include <stdlib.h>
#include <algorithm>
#include <iostream>

#include "../types.h"
#include "../globals.h"

#include "GLMapTile.h"

using std::max;

GLMapTile::GLMapTile(GameState *game_state, 
					 VideoInfo *video_info, 
					 Pointi pos, 
					 std::vector< std::vector<Colour> > *colour_grid, 
					 std::vector< std::vector<Point> > *normal_grid,
					 QuadInts heights)
	: game_state(game_state), 
	video_info(video_info), 
	pos(pos), 
	colour_grid(colour_grid), 
	normal_grid(normal_grid),
	heights(heights) {}

GLMapTile::~GLMapTile() {
}

void GLMapTile::initialise() {
	QuadColours cs = QuadColours(
		(*colour_grid)[pos.x][pos.y],
		(*colour_grid)[pos.x][pos.y+1],
		(*colour_grid)[pos.x+1][pos.y+1],
		(*colour_grid)[pos.x+1][pos.y]);
	
	Colour ac = Colour::average(4, cs.a, cs.b, cs.c, cs.d);
	Point p = Point((GLfloat)pos.x, (GLfloat)pos.y);

	Point n;
	Point na;
	Point nb;
	Point nc;
	Point nd;
	
	na = (*normal_grid)[pos.x][pos.y];
	nb = (*normal_grid)[pos.x][pos.y+1];
	nc = (*normal_grid)[pos.x+1][pos.y+1];
	nd = (*normal_grid)[pos.x+1][pos.y];
	n = (na+nb+nc+nd).normalise();
	
	/*na = n;
	nb = n;
	nc = n;
	nd = n;*/
	
	GLfloat _data[] = {
		// Point -------------------------------------------------------------------------------Normal------------------Colour------------------------------Tex Coords---
		p.x*TILE_SIZE,					p.y*TILE_SIZE,					(GLfloat)heights.a,		na.x, na.y, na.z,		cs.a.r, cs.a.g, cs.a.b, cs.a.a,		0.0f, 0.0f,
		p.x*TILE_SIZE, 					p.y*TILE_SIZE+TILE_SIZE,		(GLfloat)heights.b,		nb.x, nb.y, nb.z,		cs.b.r, cs.b.g, cs.b.b, cs.b.a,		0.0f, 1.0f,
		p.x*TILE_SIZE+TILE_SIZE,		p.y*TILE_SIZE+TILE_SIZE,		(GLfloat)heights.c,		nc.x, nc.y, nc.z,		cs.c.r, cs.c.g, cs.c.b, cs.c.a,		1.0f, 1.0f,
		p.x*TILE_SIZE+TILE_SIZE,		p.y*TILE_SIZE,					(GLfloat)heights.d,		nd.x, nd.y, nd.z,		cs.d.r, cs.d.g, cs.d.b, cs.d.a,		1.0f, 0.0f,
		p.x*TILE_SIZE+TILE_SIZE/2.0f,	p.y*TILE_SIZE+TILE_SIZE/2.0f, 	heights.middle(),		n.x, n.y, n.z,			ac.r, ac.g, ac.b, ac.a,				0.5f, 0.5f
	};
	
	data = std::vector<GLfloat>(_data, _data + sizeof(_data) / sizeof(GLfloat));
}

/**
 * Return value of (0,0,-100) signals a negative result.
 */
Point GLMapTile::findIntersection(Ray ray) {
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
Point GLMapTile::findTriangleIntersection(Ray ray, Point a, Point b, Point c) {
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

const std::vector<GLfloat> GLMapTile::getData() { return data; }

QuadInts GLMapTile::getHeights() {
	return heights;
}

bool GLMapTile::isVisible(GLEngine *glEngine) {
	Point p = Point((GLfloat)pos.x, (GLfloat)pos.y);
	
	Point a = Point(p.x*TILE_SIZE, p.y*TILE_SIZE, (GLfloat)heights.a);
	Point b = Point(p.x*TILE_SIZE, p.y*TILE_SIZE+TILE_SIZE, (GLfloat)heights.b);
	Point c = Point(p.x*TILE_SIZE+TILE_SIZE, p.y*TILE_SIZE+TILE_SIZE, (GLfloat)heights.c);
	Point d = Point(p.x*TILE_SIZE+TILE_SIZE, p.y*TILE_SIZE, (GLfloat)heights.d);
	
	return (glEngine->isVisible(a) || 
		glEngine->isVisible(b) ||
		glEngine->isVisible(c) ||
		glEngine->isVisible(d));
}