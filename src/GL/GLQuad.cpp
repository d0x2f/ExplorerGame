#include <iostream>

#include "../globals.h"

#include "GLQuad.h"
#include "GLEngine.h"

GLuint GLQuad::ibo = 0;

GLQuad::GLQuad(Size size, Point pos, Colour colour, VideoInfo *video_info) {
	init(size, pos, colour, video_info);
}

GLQuad::GLQuad(Size size, Point pos, Colour colour_a, Colour colour_b, GradientDirection gradient_direction, VideoInfo *video_info)
	: 	colour_b(colour_b), gradient_direction(gradient_direction) {
	init(size, pos, colour_a, video_info);
}
		
GLQuad::GLQuad(Size size, Point pos, QuadColours colours, QuadVectors normals, QuadInts heights, Point surface_normal, VideoInfo *video_info)
	:	size(size), pos(pos), colour_c(colours.c), colour_d(colours.d), 
		normals(normals), heights(heights), surface_normal(surface_normal), gradient_direction(QUAD) {
		colour_b = colours.b;
	init(size, pos, colours.a, video_info);
}

GLQuad::~GLQuad() {
	if(initialised) {
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &tbo);
		
		if(list != 0)
			glDeleteLists(list, 1);
	}
}

void GLQuad::init(Size size, Point pos, Colour colour, VideoInfo *video_info) {
	this->size = size;
	this->pos = pos;
	this->colour_a = colour;
	this->video_info = video_info;

	vbo=0;
	cbo=0;
	nbo=0;
	tbo=0;
	texture_id = 0;
	list = 0;
}

void GLQuad::setTexture(GLint texture_id) {
	this->texture_id = texture_id;
	
	tex_top_left = Point(0.0f, 1.0f);
	tex_bottom_right = Point(1.0f, 0.0f);
}

void GLQuad::setTexture(GLint texture_id, Size tex_size, Point top_left, Point bottom_right) {
	this->texture_id = texture_id;
	
	tex_top_left = top_left/tex_size.toPoint();
	tex_bottom_right = bottom_right/tex_size.toPoint();
	
	//Convert from tex coord origin
	GLfloat tmp = tex_top_left.y;
	tex_top_left.y = tex_bottom_right.y;
	tex_bottom_right.y = tmp;
}

GLuint GLQuad::initialise(GLuint vbo_id) {
	GLuint ret = loadVBO(vbo_id);
	compileList();
	initialised = true;
	return ret;
}
	
GLuint GLQuad::loadVBO(GLuint vbo_id) {
	this->vbo = vbo_id;
	precomputeValues();
	
	Colour colour_a, colour_b, colour_c, colour_d;
	
	switch(gradient_direction) {
		case HORIZONTAL:
			colour_a = this->colour_a;
			colour_b = this->colour_a;
			colour_c = this->colour_b;
			colour_d = this->colour_b;
			break;
		case VERTICAL:
			colour_a = this->colour_a;
			colour_b = this->colour_b;
			colour_c = this->colour_b;
			colour_d = this->colour_a;
			break;
		case NONE:
			colour_a = this->colour_a;
			colour_b = this->colour_a;
			colour_c = this->colour_a;
			colour_d = this->colour_a;
			break;
		case QUAD:
			colour_a = this->colour_a;
			colour_b = this->colour_b;
			colour_c = this->colour_c;
			colour_d = this->colour_d;
			break;
	}
	
	//Static buffer objects
	if(ibo == 0) {
		/*const GLubyte indices[] = {
			0, 1, 4,
			2, 3, 4,
			0
		};*/
		
		const GLubyte indices[] = {
			0, 1, 4,
			2, 3, 4,
			0, 4, 3,
			1, 2, 4
		};
		
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*12, indices, GL_STATIC_DRAW);
	}
	
	if(tbo == 0) {
		const GLfloat tex_coords[] = {
			tex_top_left.x, tex_bottom_right.y,
			tex_top_left.x, tex_top_left.y,
			tex_bottom_right.x, tex_top_left.y,
			tex_bottom_right.x, tex_bottom_right.y,
			tex_top_left.x + (tex_bottom_right.x - tex_top_left.x)/2.0f, tex_bottom_right.y + (tex_top_left.y - tex_bottom_right.y)/2.0f
		};
		
		glGenBuffers(1, &tbo);
		glBindBuffer(GL_ARRAY_BUFFER, tbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*10, tex_coords, GL_STATIC_DRAW);
	}
	
	if(vbo == 0) {
		//Colour for the center vertex
		Colour avg_colour = Colour::average(4, colour_a, colour_b, colour_c, colour_d);
		
		const GLfloat vertices[] = {
			//Points
			0.0f, 	0.0f,	(GLfloat)heights.a,
			0.0f, 	size.h,	(GLfloat)heights.b,
			size.w,	size.h,	(GLfloat)heights.c,
			size.w,	0.0f,	(GLfloat)heights.d,
			size.w/2.0f, size.h/2.0f, heights.middle(),
			
			//Normals
			surface_normal.x, surface_normal.y, surface_normal.z,
			surface_normal.x, surface_normal.y, surface_normal.z,
			surface_normal.x, surface_normal.y, surface_normal.z,
			surface_normal.x, surface_normal.y, surface_normal.z,
			surface_normal.x, surface_normal.y, surface_normal.z,
			
			//Colours (diffuse lighting)
			colour_a.r, colour_a.g, colour_a.b, colour_a.a,
			colour_b.r, colour_b.g, colour_b.b, colour_b.a,
			colour_c.r, colour_c.g, colour_c.b, colour_c.a,
			colour_d.r, colour_d.g, colour_d.b, colour_d.a,
			avg_colour.r, avg_colour.g, avg_colour.b, avg_colour.a
		};
		
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*50, vertices, GL_STATIC_DRAW);
	}
	
	//Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	return vbo;
}

void GLQuad::compileList() {
	list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	
	//glBindBuffer(GL_ARRAY_BUFFER, nbo);
	glNormalPointer(GL_FLOAT, 0, (const void*)(sizeof(GLfloat)*15));
	
	//glBindBuffer(GL_ARRAY_BUFFER, cbo);
	glColorPointer(4, GL_FLOAT, 0, (const void*)(sizeof(GLfloat)*30));

	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glTexCoordPointer(2, GL_FLOAT, 0, NULL);

	if(texture_id != 0)
		glBindTexture(GL_TEXTURE_2D, texture_id);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_BYTE, NULL);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	//unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	/*
	Point v = surface_normal;
	v.normalise(0.5f);
	glLineWidth (1.0f);
	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
	
	glBegin(GL_LINES);
	
	glVertex3f(size.w/2.0f, size.h/2.0f, heights.middle());
	glVertex3f((size.w/2.0f)+v.x, (size.h/2.0f)+v.y, heights.middle()+v.z);
	
	
	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	
	glVertex3f(0.0f, 0.0f, heights.a);
	glVertex3f(normals.a.x, normals.a.y, heights.a+normals.a.z);
	
	glVertex3f(0.0f, size.h, heights.b);
	glVertex3f(normals.b.x, size.h+normals.b.y, heights.b+normals.b.z);
	
	glVertex3f(size.w, size.h, heights.c);
	glVertex3f(size.w+normals.c.x, size.h+normals.c.y, heights.c+normals.c.z);
	
	glVertex3f(size.w, 0.0f, heights.d);
	glVertex3f(size.w+normals.d.x, normals.d.y, heights.d+normals.d.z);
	
	glEnd();
	*/
	
	glEndList();
}

void GLQuad::precomputeValues() {
	stride_length = sizeof(GLfloat)*6;
	colour_offset = sizeof(GLfloat)*2;
}

void GLQuad::draw() {
	glPushMatrix();
	
	glTranslatef(pos.x, pos.y, pos.z);
	
	if(list != 0)
		glCallList(list);
	
	glPopMatrix();
}

void GLQuad::draw(GLfloat x, GLfloat y, GLfloat z) {
	glPushMatrix();
	
	glTranslatef(x, y, z);
	
	glCallList(list);
	
	glPopMatrix();
	
}

void GLQuad::draw(GLfloat x, GLfloat y, GLfloat z, GLfloat rot) {
	glPushMatrix();
	
	glTranslatef(size.w/2.0f+x, size.h/2.0f+y, 0.0f+z);
	glRotatef(rot, 0.0f, 0.0f, -1.0f);
	glTranslatef(-size.w/2.0f, -size.h/2.0f, 0.0f);
	
	glCallList(list);
	
	glPopMatrix();
}