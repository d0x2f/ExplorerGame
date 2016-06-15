#ifndef QUAD_H
#define QUAD_H

#include <SDL2/SDL.h>

#include "../types.h"

class GLQuad {
public:
	static GLuint ibo;
	
	enum Type {
		COLOUR,
		TEXTURE
	};
	
	enum GradientDirection {
		NONE,
		HORIZONTAL,
		VERTICAL,
		QUAD
	};
	
	GLQuad(Size size, Point pos, Colour colour, VideoInfo *video_info);
	GLQuad(Size size, Point pos, Colour colour_a, Colour colour_b, GradientDirection gradient_direction, VideoInfo *video_info);
	GLQuad(Size size, Point pos, QuadColours colours, QuadVectors normals, QuadInts heights, Point surface_normal, VideoInfo *video_info);
	~GLQuad();
	
	GLuint initialise(GLuint vbo_id=0);
	void draw();
	void draw(GLfloat x, GLfloat y, GLfloat z);
	void draw(GLfloat x, GLfloat y, GLfloat z, GLfloat rot);
	void setTexture(GLint texture_id);
	void setTexture(GLint texture_id, Size tex_size, Point top_left, Point bottom_right);
private:
	Size size;
	Point pos;
	Colour colour_a, colour_b, colour_c, colour_d;
	QuadVectors normals;
	QuadInts heights;
	Point surface_normal;
	GLuint vbo, cbo, nbo, tbo;
	VideoInfo *video_info;
	GradientDirection gradient_direction;
	GLuint texture_id;
	Point tex_top_left, tex_bottom_right;
	GLuint list;
	bool initialised = false;
	
	//Pre computed values
	Uint32 stride_length, colour_offset;
	
	void init(Size size, Point pos, Colour colour, VideoInfo *video_info);
	void precomputeValues();
	GLuint loadVBO(GLuint vbo_id=0);
	void compileList();
};

#endif