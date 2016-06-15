#ifndef GLMESH_H
#define GLMESH_H

#include <map>
#include <vector>

#include "../types.h"
#include "../utils.h"

class GLEngine;

class GLMesh {
public:
	GLMesh(const char *filename, VideoInfo *video_info, GLEngine *glEngine, bool drawCenter = false);
	~GLMesh();
	
	void load();
	void draw(GLfloat rot, Point p);
	void setScale(GLfloat s);
	GLfloat getScale();
	GLfloat getBoundingRadius();
	Point getCenter();
	
	void bindBuffers();
	void transform(Point position, GLfloat rotation);
	void drawBuffers();
	void unbindBuffers();

private:
	bool mesh_loaded;
	const char *filename;
	VideoInfo *video_info;
	GLEngine *glEngine;
	GLuint vbo, ibo;
	GLuint list;
	bool drawCenter;
	
	
	GLfloat scale;
	GLfloat boundingRadius;
	GLfloat minZ, maxZ, minY, maxY, minX, maxX;
	
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::map<std::string, GLuint> textures;
	std::vector<std::pair<GLuint, GLuint> > texture_ipos;
	
	bool initialised;
	
	void loadVBO();
	void loadMaterials(std::string filename);
	void compileList();
};

#endif