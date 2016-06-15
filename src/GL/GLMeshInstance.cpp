#include <math.h>

#include "../globals.h"

#include "GLMeshInstance.h"
#include "GLEngine.h"

GLuint GLMeshInstance::circle_list = 0;

GLMeshInstance::GLMeshInstance(GLMesh *mesh, Point p, GLEngine *glEngine, int id) : mesh(mesh), position(p), glEngine(glEngine), id(id) {
	drawn = false;
}

GLMeshInstance::~GLMeshInstance() {
	
}

void GLMeshInstance::initialise() {
	if(circle_list != 0)
		return;
	
	circle_list = glGenLists(1);
	
	glNewList(circle_list, GL_COMPILE);
	
	glLineWidth(1.0f);
	glBegin(GL_LINE_STRIP);
	
	glColor3f(1.0f, 1.0f, 1.0f);
	
	for(int i=0; i<361; i++) {
		glVertex3f(cos( (GLfloat)((i*PI)/180) ), sin( (GLfloat)((i*PI)/180) ), 0.0f );
	}
	
	glEnd();
	
	glEndList();
}

void GLMeshInstance::setPosition(Point p) {
	position = p;
}

void GLMeshInstance::setRotation(GLfloat rot) {
	rotation = rot;
}

Point GLMeshInstance::getPosition() {
	return position;
}

void GLMeshInstance::setDrawn(bool b) {
	drawn = b;
}

bool GLMeshInstance::hasDrawn() {
	return drawn;
}

void GLMeshInstance::bindBuffers() {
	mesh->bindBuffers();
}
void GLMeshInstance::transform() {
	mesh->transform(position, rotation);
}

void GLMeshInstance::drawBuffers() {
	if(!drawn && isVisible())
		mesh->drawBuffers();
		
	drawn = true;
}

void GLMeshInstance::unbindBuffers() {
	mesh->unbindBuffers();
}

bool GLMeshInstance::isVisible() {
	Point sphere = mesh->getCenter() + position;
	GLfloat radius = mesh->getBoundingRadius();
	
	return glEngine->isVisible(sphere, radius);
}

int GLMeshInstance::getId() {
	return id;
}

void GLMeshInstance::drawSelected() {
	glPushMatrix();
	glUseProgram(0);
	glDepthFunc(GL_ALWAYS);
	
	Point center = mesh->getCenter();
	glTranslatef(center.x+position.x, center.y+position.y, position.z+0.1f);
	
	GLfloat scale_factor = mesh->getBoundingRadius();
	glScalef(scale_factor, scale_factor, 0.0f);
	
	glCallList(circle_list);
	
	glDepthFunc(GL_LEQUAL);
	glPopMatrix();
}

void GLMeshInstance::draw() {
	if(!drawn && isVisible())
		mesh->draw(rotation, position);
	
	drawn = true;
}