#ifndef GLMESHINSTANCE_H
#define GLMESHINSTANCE_H

#include "../types.h"
#include "../utils.h"

#include "GLMesh.h"

class GLMeshInstance {
public:
	static GLuint circle_list;
	
	GLMeshInstance(GLMesh *mesh, Point p, GLEngine *glEngine, int id);
	~GLMeshInstance();
	
	void draw();
	void drawSelected();
	void setPosition(Point p);
	void setRotation(GLfloat rot);
	Point getPosition();
	void initialise();
	void setDrawn(bool b);
	bool hasDrawn();
	void bindBuffers();
	void transform();
	void drawBuffers();
	void unbindBuffers();
	bool isVisible();
	int getId();
private:
	GLMesh *mesh;
	Point position;
	GLEngine *glEngine;
	bool drawn;
	GLfloat rotation;
	int id;
};

#endif