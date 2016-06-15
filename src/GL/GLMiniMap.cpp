#include <iostream>
#include <cmath>

#include "../globals.h"

#include "GLMiniMap.h"

GLMiniMap::GLMiniMap(Size size, Point pos, VideoInfo *video_info, GameState *game_state, std::vector< std::vector<GLint> > *height_grid) : size(size), pos(pos), video_info(video_info), game_state(game_state), height_grid(height_grid) {
	texture_id = 0;
	minimap_quad = new GLQuad(size, Point(0.0f, 0.0f), Colour(1.0f, 1.0f, 1.0f, 1.0f), video_info);
}

GLMiniMap::~GLMiniMap() {
	delete minimap_quad;
	
	if(texture_id != 0)
		glDeleteTextures(1, &texture_id);
}

//Create a texture representing the map
void GLMiniMap::initialise() {
	Size s = game_state->getMapSize();
	
	_Colour base_colour;
	base_colour.r = 0.5f;
	base_colour.g = 0.7f;
	base_colour.b = 0.5f;
	
	std::vector<_Colour> map_texture_data = std::vector<_Colour>( s.w * s.h, base_colour );
	GLint height;
	
	for(int x=0; x<s.w; x++) {
		for(int y=0; y<s.h; y++) {
			height = (*height_grid)[x][(s.h-1)-y];
			map_texture_data[y*s.w + x].r += height*0.1f;
			map_texture_data[y*s.w + x].g += height*0.1f;
			map_texture_data[y*s.w + x].b += height*0.1f;
			if(height >= 0) {
				map_texture_data[y*s.w + x].g = 1.0f;
			} else if(height < 0) {
				map_texture_data[y*s.w + x].b = 1.0f;
			}
		}
	}
	
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s.w, s.h, 0, GL_RGB, GL_FLOAT, map_texture_data.data());
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	minimap_quad->setTexture(texture_id);
	minimap_quad->initialise();
}

void GLMiniMap::draw(Point p) {
	pos = p;
	minimap_quad->draw(pos.x, pos.y, pos.z, 45.0f);
}

//Use matrix transformations to find where the click equates to on the map.
//Returns the point clicked or (0,0,1) if the map wasn't clicked.
Point GLMiniMap::checkClick(Point p) {
	p -= pos;
	p.x -= size.w/2.0f;
	p.y -= size.h/2.0f;
	
	GLfloat s = sin(PI/4.0f);
	GLfloat c = cos(PI/4.0f);
	
	Size ms = game_state->getMapSize();
	
	Point map_pos = Point( ((c*p.x-s*p.y + size.w/2.0f)/size.w)*ms.w, ((size.h-(s*p.x+c*p.y + size.h/2.0f))/size.h)*ms.h );
	
	if(map_pos.x > ms.w || map_pos.x < 0 || map_pos.y > ms.h || map_pos.y < 0)
		map_pos = Point(0.0f, 0.0f, 1.0f);
	
	return map_pos;	
}