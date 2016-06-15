#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

#include "../globals.h"

#include "GLMesh.h"
#include "GLEngine.h"

GLMesh::GLMesh(const char *filename, VideoInfo *video_info, GLEngine *glEngine, bool drawCenter) : filename(filename), video_info(video_info), glEngine(glEngine), drawCenter(drawCenter) {
	vbo = 0;
	ibo = 0;
	list = 0;
	drawCenter = false;
	mesh_loaded = false;
	initialised = false;
	
	
	scale = 1.0f;
	boundingRadius = 0.0f;
	minZ=10.0f;
	maxZ=0.0f;
	minY=10.0f;
	maxY=0.0f;
	minX=10.0f;
	maxX=0.0f;
}

GLMesh::~GLMesh() {
	if(initialised) {
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ibo);
	}
}

void GLMesh::load() {
	#if DEBUG == 1
	std::cout << "loading model from: " << filename << std::endl;
	#endif
	
	std::ifstream file(filename);
	
	std::vector<Point> vertices;
	std::vector<Point> normals;
	std::vector<Point2D> tex_coords;
	
	GLfloat minlen=10.0f, maxlen=0.0f;
	
	std::string line;
	
	//Do a pass for vertices and normals (and mtllib)
	while (std::getline(file, line))
	{
		std::string token;
		Point v;
		Point2D t;
		
		std::stringstream ss(line);
		
		ss >> token;
		
		if(token.compare("mtllib") == 0) {
			loadMaterials(std::string("resources/models/") + line.substr(7, std::string::npos));
		} else if(token.compare("v") == 0) {
			ss >> v.x;
			ss >> v.y;
			ss >> v.z;
			
			if(v.x < minX)
				minX = v.x;
			if(v.y < minY)
				minY = v.y;
			if(v.z < minZ)
				minZ = v.z;
			
			if(v.x > maxX)
				maxX = v.x;
			if(v.y > maxY)
				maxY = v.y;
			if(v.z > maxZ)
				maxZ = v.z;
			
			GLfloat len = v.length();
			if(len < minlen)
				minlen = len;
			
			if(len > maxlen)
				maxlen = len;
			
			vertices.push_back(v);
		} else if(token.compare("vn") == 0) {
			ss >> v.x;
			ss >> v.y;
			ss >> v.z;
			normals.push_back(v);
		} else if(token.compare("vt") == 0) {
			ss >> t.x;
			ss >> t.y;
			tex_coords.push_back(t);
		}
	}

	//Center mesh
	Point translation = Point(-(maxX-minX)/2.0f-minX, -(maxY-minY)/2.0f-minY, -(maxZ-minZ)/2.0f-minZ);
	
	for(std::vector<Point>::iterator it = vertices.begin(); it != vertices.end(); it++) {
		it->x += translation.x;
		it->y += translation.y;
		it->z += translation.z;
		//(*it) *= scale;
		//(*it)->rotate(rotation);
	}
	
	boundingRadius = std::max(std::max(maxX-minX, maxY-minY), maxZ-minZ)*0.7f;
	
	file.clear();
	file.seekg (0, file.beg);
	
	std::map<std::string, GLuint> vertex_index;
	
	//Now do a pass for faces and material usage
	while (std::getline(file, line))
	{
		std::string s, token;

		std::stringstream ss(line);

		ss >> token;
		if(token.compare("usemtl") == 0) {
			ss >> s;
			
			texture_ipos.push_back(std::make_pair(textures[s], this->indices.size()));
		} else if(token.compare("f") == 0) {
			do {
				ss >> s;

				std::vector<std::string> parts = split(s, '/');
				
				Vertex v;
				if(parts.size() == 3) {
					GLuint index = atoi(parts[0].c_str())-1;
					GLuint tex_i = atoi(parts[1].c_str())-1;
					GLuint normal_i = atoi(parts[2].c_str())-1;
					v.p = vertices[index];
					
					if(tex_i < tex_coords.size())
						v.t = tex_coords[tex_i];
					
					if(normal_i < normals.size())
						v.n = normals[normal_i];
				} else if(parts.size() == 1) {
					GLuint index = atoi(parts[0].c_str())-1;
					
					v.p = vertices[index];
				} else {
					//Weird formatting, abort!
					continue;
				}
				
				GLuint index = vertex_index[s];
				if(index == 0) {
					this->vertices.push_back(v);
					index = this->vertices.size();
					vertex_index[s] = index;
				}
				this->indices.push_back(index-1);
			} while(!ss.eof());
		}
	}

	loadVBO();
	//compileList();
	
	initialised = true;
}

void GLMesh::loadMaterials(std::string filename) {
	std::ifstream file(filename);
	std::string line, token, current_material_name;
	
	#if DEBUG == 1
	std::cout << "loading materials from: " << filename << std::endl;
	#endif
	
	while (std::getline(file, line)) {
		if(line.empty())
			continue;
		
		std::stringstream ss(line);
		
		ss >> token;
		
		if(token.compare("newmtl") == 0) {
			ss >> current_material_name;
		} else if(token.compare("map_Kd") == 0) {
			if(!current_material_name.empty()) {
				GLuint texture_id = glEngine->loadTexture((std::string("resources/textures/") + path(line.substr(7, std::string::npos).c_str()).filename().string()).c_str());
				textures[current_material_name] = texture_id;
				
				#if DEBUG == 1
				std::cout << "loaded " << current_material_name << std::endl;
				#endif
			}
		}
	}
}

void GLMesh::loadVBO() {
	//Generate and populate mega VBO
	//Vertex Data:
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	
	//Index Data:
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*indices.size(), indices.data(), GL_STATIC_DRAW);
	
	//Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLMesh::setScale(GLfloat scale) {
	this->scale = scale;
}

GLfloat GLMesh::getScale() {
	return scale;
}

GLfloat GLMesh::getBoundingRadius() {
	return boundingRadius*scale;
}

Point GLMesh::getCenter() {
	if(drawCenter)
		return Point();
	else
		return Point((maxX-minX)*scale/2.0f, (maxY-minY)*scale/2.0f, (maxZ-minZ)*scale/2.0f);
}

void GLMesh::compileList() {
	list = glGenLists(1);

	glNewList(list, GL_COMPILE);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)Vertex::offset(0));	//Vertex
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)Vertex::offset(1));	//Normal
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)Vertex::offset(2));	//Colour
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)Vertex::offset(3));	//Texture
	
	GLuint rendered = 0;
	for (unsigned i=0; i<texture_ipos.size(); i++) {
		glBindTexture(GL_TEXTURE_2D, texture_ipos[i].first);
		
		if(i != texture_ipos.size()-1) {
			glDrawElements(GL_TRIANGLES, texture_ipos[i+1].second - rendered, GL_UNSIGNED_INT, (const void*)(rendered * sizeof(GLuint)));
			rendered = texture_ipos[i+1].second;
		} else {
			glDrawElements(GL_TRIANGLES, indices.size() - rendered, GL_UNSIGNED_INT, (const void*)(rendered * sizeof(GLuint)));
		}
	}
	
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	
	//unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEndList();
}

void GLMesh::draw(GLfloat rot, Point p) {
	glPushMatrix();
	
	Point translation;;
	if(drawCenter)
		translation = Point(p.x, p.y, p.z+(maxZ-minZ)*scale/2.0f);
	else
		translation = Point(p.x+(maxX-minX)*scale/2.0f, p.y+(maxY-minY)*scale/2.0f, p.z+(maxZ-minZ)*scale/2.0f);

	GLShader *shader = glEngine->getGeneralShader();
	shader->use();

	shader->setUniform("rotate", rot);
	shader->setUniform("scale", scale);
	shader->setUniform("translate", translation.x, translation.y, translation.z);
	
	glEngine->getGeneralShader()->use();
	checkOGLError(__FILE__, __LINE__);
	glCallList(list);
	checkOGLError(filename, __LINE__);
	
	glPopMatrix();
}

void GLMesh::bindBuffers() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)Vertex::offset(0));	//Vertex
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)Vertex::offset(1));	//Normal
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)Vertex::offset(2));	//Colour
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)Vertex::offset(3));	//Texture
}
void GLMesh::transform(Point position, GLfloat rotation) {
	GLShader *shader = glEngine->getGeneralShader();
	shader->use();
		
	checkOGLError(__FILE__, __LINE__);
	
	Point translation;
	
	if(drawCenter)
		translation = Point(position.x, position.y, position.z+(maxZ-minZ)*scale/2.0f);
	else
		translation = Point(position.x+(maxX-minX)*scale/2.0f, position.y+(maxY-minY)*scale/2.0f, position.z+(maxZ-minZ)*scale/2.0f);
	
	shader->setUniform("rotate", rotation);
	shader->setUniform("scale", scale);
	shader->setUniform("translate", translation.x, translation.y, translation.z);
	
	#if DEBUG == 1
	if(r_debug) {
		glPushMatrix();
		
		glUseProgram(0);
		
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glLineWidth(0.5f);
		
		Point t;
		if(drawCenter)
			t = Point(position.x, position.y, position.z+(maxZ-minZ)*scale/2.0f);
		else
			t = Point(position.x+(maxX-minX)*scale/2.0f, position.y+(maxY-minY)*scale/2.0f, position.z+(maxZ-minZ)*scale/2.0f);
		glTranslatef(t.x, t.y, t.z);
		
		glScalef(scale, scale, scale);
		
		GLUquadricObj *quadric;
		quadric = gluNewQuadric();
		
		gluQuadricDrawStyle(quadric, GLU_LINE );
		gluSphere( quadric , boundingRadius , 10 , 4 );
		
		gluDeleteQuadric(quadric);
		
		//glTranslatef(-t.x, -t.y, -t.z);
		
		GLShader *shader = glEngine->getGeneralShader();
		shader->use();
		
		glPopMatrix();
	}
	#endif
}

void GLMesh::drawBuffers() {
	GLuint rendered = 0;
	for (unsigned i=0; i<texture_ipos.size(); i++) {
		glBindTexture(GL_TEXTURE_2D, texture_ipos[i].first);
		
		if(i != texture_ipos.size()-1) {
			glDrawElements(GL_TRIANGLES, texture_ipos[i+1].second - rendered, GL_UNSIGNED_INT, (const void*)(rendered * sizeof(GLuint)));
			rendered = texture_ipos[i+1].second;
		} else {
			glDrawElements(GL_TRIANGLES, indices.size() - rendered, GL_UNSIGNED_INT, (const void*)(rendered * sizeof(GLuint)));
		}
	}
}

void GLMesh::unbindBuffers() {
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	
	//unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}