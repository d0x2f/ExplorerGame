#ifndef GLUNIT_H
#define GLUNIT_H

#include "../types.h"

#include "GLMesh.h"

class GLUnit {
public:
	GLUnit(Point pos, GLMesh *mesh, VideoInfo *video_info);
	~GLUnit();
	
	void draw();
private:
	Point pos;
	GLMesh *mesh;
	VideoInfo *video_info;
};

#endif