#ifndef GLMESHGROUP_H
#define GLMESHGROUP_H

#include "../globals.h"
#include "../types.h"

#include "GLMesh.h"
#include "GLMeshInstance.h"

class GLMeshGroup {
public:
	GLMeshGroup();
	~GLMeshGroup();
	
	void draw(int selected);
	void addInstance(GLMeshInstance *instance);
private:
	std::vector<GLMeshInstance *> meshes;
	
	void drawSelected(int selected);
};

#endif