#include <iostream>

#include "GLGameObject.h"
#include "GLEngine.h"

GLGameObject::GLGameObject(GameState *game_state, int id, GameObject *go, GLMesh *mesh, Point pos, GLEngine *glEngine) : game_state(game_state), id(id), go(go), mesh(mesh), glEngine(glEngine) {
	mesh_instance = new GLMeshInstance(mesh, pos, glEngine, this->id);
}

GLGameObject::~GLGameObject() {
}

void GLGameObject::initialise() {
	mesh_instance->initialise();
}

GLMeshInstance *GLGameObject::getMeshInstance() {
	return mesh_instance;
}

bool GLGameObject::isVisible() {
	return mesh_instance->isVisible();
}

int GLGameObject::getId() {
	return id;
}

bool GLGameObject::intersects(Ray ray) {
	Point sphere = mesh->getCenter() + mesh_instance->getPosition();
	GLfloat radius = mesh->getBoundingRadius();
	
	Point sphere_dir = sphere - ray.a;
	
	if(ray.dir.dot(sphere_dir) <= 0)
		return false;
	
	Point projection = sphere_dir.projectOnto(ray.dir)+ray.a;
	GLfloat length = (sphere - projection).length();
	
	if(length < radius)
		return true;
	
	return false;
}

GLfloat GLGameObject::getSelectionRadius() {
	return mesh->getBoundingRadius();
}

bool GLGameObject::isSelectable() {
	return go->isSelectable();
}

bool GLGameObject::isMovable() {
	return go->isMovable();
}

void GLGameObject::moveTo(Point p) {
	go->moveTo(p);
}
	
void GLGameObject::draw(bool selected) {
	mesh_instance->setPosition(go->getPosition());
	mesh_instance->setRotation(go->getRotation());

	if(mesh_instance->hasDrawn())
		return;
	
	if(selected)
		mesh_instance->drawSelected();
	
	mesh_instance->draw();
}