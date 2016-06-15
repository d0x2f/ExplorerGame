#include <vector>

#include "GLMeshGroup.h"

GLMeshGroup::GLMeshGroup() {
}

GLMeshGroup::~GLMeshGroup() {
}

void GLMeshGroup::draw(int selected) {
	if(meshes.size() == 0)
		return;
		
	bool firstDraw = true;
	drawSelected(selected);

	for(std::vector<GLMeshInstance *>::iterator it = meshes.begin(); it != meshes.end(); it++) {
		if(!(*it)->hasDrawn()) {
			if((*it)->isVisible()) {
				if(firstDraw) {
					firstDraw = false;
					(*it)->bindBuffers();
				}
				(*it)->transform();
				(*it)->drawBuffers();
			}
		}
	}
	if(!firstDraw) {
		GLMeshInstance *first = meshes[0];
		first->unbindBuffers();
	}
}

void GLMeshGroup::drawSelected(int selected) {
	for(std::vector<GLMeshInstance *>::iterator it = meshes.begin(); it != meshes.end(); it++) {
		if(!(*it)->hasDrawn()) {
			if((*it)->getId() == selected) {
				(*it)->drawSelected();
			}
		}
	}
}

void GLMeshGroup::addInstance(GLMeshInstance *instance) {
	meshes.push_back(instance);
}