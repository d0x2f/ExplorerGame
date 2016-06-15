#include <iostream>

#include "../types.h"
#include "../globals.h"
#include "../utils.h"

#include "GLMap.h"
#include "GLMapTile.h"
#include "GLEngine.h"

#include "../GAME/Map.h"

GLMap::GLMap(GameState *game_state, VideoInfo *video_info, GLEngine *glEngine, Point minimap_pos) : game_state(game_state), video_info(video_info), glEngine(glEngine), minimap_pos(minimap_pos) {
	map_grid_list = 0;
	land_texture_id = 0;
	sand_texture_id = 0;
	water_texture_id = 0;
	water_edge_texture_id = 0;
	vbo = 0;
	ibo = 0;
	index_count = 0;
	vertex_data_size = 0;
	initialised = false;
	
	Size map_size = game_state->getMapSize();
	
	
	map_board = std::vector< std::vector<GLMapTile*> >( map_size.w, std::vector<GLMapTile*> ( map_size.h, NULL ) );
	colour_grid = std::vector< std::vector<Colour> >( map_size.w+1, std::vector<Colour> ( map_size.h+1, Colour() ) );
	height_grid = std::vector< std::vector<GLint> >( map_size.w+1, std::vector<GLint> ( map_size.h+1, 0 ) );
	normal_grid = std::vector< std::vector<Point> >( map_size.w+1, std::vector<Point> ( map_size.h+1, Point() ) );
	
	minimap = new GLMiniMap(Size(192.0f, 192.0f), minimap_pos, video_info, game_state, &height_grid);
	empty_tile = new GLMapTile(game_state, video_info, Pointi(), &colour_grid, &normal_grid, QuadInts(0,0,0,0));
	
	time_loop = 0;//glEngine->getTimeUs();.
	last_draw_time = 0;
}

GLMap::~GLMap() {
	if(initialised) {
		Size map_size = game_state->getMapSize();
		
		for(int x=0; x<map_size.w; x++) {
			for(int y=0; y<map_size.h; y++) {
				delete map_board[x][y];
			}
		}
		
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ibo);
		
		delete water_shader;
		delete land_shader;
	}
	
	if(map_grid_list != 0)
		glDeleteLists(map_grid_list, 1);
	
	if(land_texture_id != 0)
		glDeleteTextures(1, &land_texture_id);
		
	if(sand_texture_id != 0)
		glDeleteTextures(1, &sand_texture_id);

	if(water_texture_id != 0)
		glDeleteTextures(1, &water_texture_id);
	
	if(water_edge_texture_id != 0)
		glDeleteTextures(1, &water_edge_texture_id);
		
	delete minimap;
}

GLfloat GLMap::getHeightAt(Point p) {
	GLint x,y;
	x = (GLint)floor(p.x/TILE_SIZE);
	y = (GLint)floor(p.y/TILE_SIZE);
	return map_board[x][y]->findIntersection(Ray(Point(p.x, p.y, 20.0f), Point(p.x, p.y, -20.0f), Point(0.0f, 0.0f, -1.0f))).z;
}

GLint GLMap::getHeightAt(Point2D p) {
	return getHeightAt(p.x, p.y);
}

GLint GLMap::getHeightAt(int x, int y) {
	Size m = game_state->getMapSize();
	if(inBounds(x, y, Size(1.0f, 1.0f)+m))
		return height_grid[x][y];
	else
		return -1;
}

Point GLMap::findIntersection(Ray ray) {
	//Check only the 25 adjacent squares at the z=0 intersect point
	GLfloat t = -ray.a.z/ray.dir.z;
	Point p(ray.a.x+ray.dir.x*t, ray.a.y+ray.dir.y*t, 0.0f);
	
	GLint x = (GLint)floor(p.x/TILE_SIZE);
	GLint y = (GLint)floor(p.y/TILE_SIZE);
	
	Point intersects[5][5];
	
	for(int i=0; i<5; i++) {
		for(int j=0; j<5; j++) {
			intersects[i][j] = getTile(Pointi(x-(i-2), y-(j-2)))->findIntersection(ray);
		}
	}
	
	Point closest = intersects[0][0];
	GLfloat distance = (intersects[0][0] - ray.a).length();
	
	for(int i=0; i<5; i++) {
		for(int j=0; j<5; j++) {
			GLfloat len = (intersects[i][j] - ray.a).length();
			if(len < distance) {
				distance = len;
				closest = intersects[i][j];
			}
		}
	}
	
	return closest;
}

GLMapTile *GLMap::getTile(Pointi p) {
	Size s = game_state->getMapSize();
	
	if(p.x < 0 || p.x >= s.w || p.y < 0 || p.y >= s.h)
		return empty_tile;
	
	return map_board[p.x][p.y];
}

bool GLMap::inBounds(GLint x, GLint y) {
	Size m = game_state->getMapSize();
	return (x>=0 && x<m.w && y>=0 && y<m.h);
}

bool GLMap::inBounds(GLint x, GLint y, Size s) {
	return (x>=0 && x<s.w && y>=0 && y<s.h);
}

QuadInts GLMap::getSquaredHeightsAt(std::vector<std::vector<GLint> > *height_grid, Pointi p) {
	Size map_size = game_state->getMapSize();
	
	GLint h_00 = inBounds(p.x-1, p.y-1) ? (*height_grid)[p.x-1][p.y-1]:-1;
	GLint h_01 = inBounds(p.x-1, p.y  ) ? (*height_grid)[p.x-1][p.y  ]:-1;
	GLint h_02 = inBounds(p.x-1, p.y+1) ? (*height_grid)[p.x-1][p.y+1]:-1;
	GLint h_10 = inBounds(p.x  , p.y-1) ? (*height_grid)[p.x  ][p.y-1]:-1;
	GLint h_11 = inBounds(p.x  , p.y  ) ? (*height_grid)[p.x  ][p.y  ]:-1;
	GLint h_12 = inBounds(p.x  , p.y+1) ? (*height_grid)[p.x  ][p.y+1]:-1;
	GLint h_20 = inBounds(p.x+1, p.y-1) ? (*height_grid)[p.x+1][p.y-1]:-1;
	GLint h_21 = inBounds(p.x+1, p.y  ) ? (*height_grid)[p.x+1][p.y  ]:-1;
	GLint h_22 = inBounds(p.x+1, p.y+1) ? (*height_grid)[p.x+1][p.y+1]:-1;
	
	// 02 12 22
	// 01 11 21
	// 00 10 20
	
	return QuadInts(
		((p.x==0||p.y==0)?-1:max(max(max(h_00, h_10), h_01), h_11)),
		((p.x==0||p.y==map_size.h-1)?-1:max(max(max(h_02, h_01), h_12), h_11)),
		((p.x==map_size.w-1||p.y==map_size.h-1)?-1:max(max(max(h_12, h_22), h_21), h_11)),
		((p.x==map_size.w-1||p.y==0)?-1:max(max(max(h_21, h_20), h_10), h_11)));
}

void GLMap::initialise() {
	if(initialised)
		return;
	
	Size map_size = game_state->getMapSize();
	
	land_texture_id = glEngine->loadTexture("resources/textures/grass.png");
	sand_texture_id = glEngine->loadTexture("resources/textures/sand.png");
	water_texture_id = glEngine->loadTexture("resources/textures/water.png");
	water_edge_texture_id = glEngine->loadTexture("resources/textures/water_edge.png");
	
	height_grid = game_state->getMap()->getHeights();
	
	//Generate colours and normals
	for(int x=0; x<=map_size.w; x++) {
		for(int y=0; y<=map_size.h; y++) {
			GLfloat gray = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))*0.3f+0.7f;
			colour_grid[x][y] = Colour(gray, gray, gray, 1.0f);
			
			Point a, b, c, d, e, f, g, h, current;
			
			Point pf = Point((GLfloat)x, (GLfloat)y);
			
			current = Point(x, y, getHeightAt(x, y));
			a = (Point(pf.x-1, pf.y, getHeightAt(x-1, y))-current).normalise();
			b = (Point(pf.x, pf.y-1, getHeightAt(x, y-1))-current).normalise();
			c = (Point(pf.x+1, pf.y, getHeightAt(x+1, y))-current).normalise();
			d = (Point(pf.x, pf.y+1, getHeightAt(x, y+1))-current).normalise();
			
			e = (Point(pf.x-1, pf.y-1, getHeightAt(x-1, y-1))-current).normalise();
			f = (Point(pf.x-1, pf.y+1, getHeightAt(x-1, y+1))-current).normalise();
			g = (Point(pf.x+1, pf.y+1, getHeightAt(x+1, y+1))-current).normalise();
			h = (Point(pf.x+1, pf.y-1, getHeightAt(x+1, y-1))-current).normalise();
			
			normal_grid[x][y] = (a.cross(e) + f.cross(a) + e.cross(b) + b.cross(h) +
								 h.cross(c) + c.cross(g) + g.cross(d) + d.cross(f)).normalise();
		}
	}
	
	//Even out the colours
	for(int x=0; x<map_size.w; x++) {
		for(int y=0; y<map_size.h; y++) {
			colour_grid[x][y].blend(
				(x==0?Colour(0.0f, 0.0f, 0.0f, 1.0f):colour_grid[x-1][y]), 
				((x==0||y==0)?Colour(0.0f, 0.0f, 0.0f, 1.0f):colour_grid[x-1][y-1]),
				(y==0?Colour(0.0f, 0.0f, 0.0f, 1.0f):colour_grid[x][y-1]),
				colour_grid[x+1][  y], 
				colour_grid[  x][y+1], 
				colour_grid[x+1][y+1], 
				(x==0?Colour(0.0f, 0.0f, 0.0f, 1.0f):colour_grid[x-1][y+1]),
				(y==0?Colour(0.0f, 0.0f, 0.0f, 1.0f):colour_grid[x+1][y-1]));
				
			QuadInts heights;
			heights.a = getHeightAt(x, y);
			heights.b = getHeightAt(x, y+1);
			heights.c = getHeightAt(x+1, y+1);
			heights.d = getHeightAt(x+1, y);
				
			map_board[x][y] = new GLMapTile(game_state, video_info, Pointi(x, y), &colour_grid, &normal_grid, heights);
		}
	}
	
	generateVBO();
	loadShaders();
	
	//Create render list
	if(!map_grid_list) {
		
		map_grid_list = glGenLists(1);
		glNewList(map_grid_list, GL_COMPILE);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*12, (const void*)0);	//Vertex
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*12, (const void*)(sizeof(GLfloat)*3));	//Normal
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*12, (const void*)(sizeof(GLfloat)*6));	//Colour
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*12, (const void*)(sizeof(GLfloat)*10)); //Texture
			
		//Land
		if(land_texture_id != 0 && sand_texture_id != 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, land_texture_id);
			
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, sand_texture_id);
			land_shader->use();
		}
		glDrawElements(GL_TRIANGLES, 12*(map_size.w*map_size.h), GL_UNSIGNED_INT, (const void*)0);

		//Water
		if(water_texture_id != 0 && water_edge_texture_id != 0) {
			glBindTexture(GL_TEXTURE_2D, water_edge_texture_id);
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, water_texture_id);
			water_shader->use();
		}
		glDrawElements(GL_TRIANGLES, index_count-12*(map_size.w*map_size.h), GL_UNSIGNED_INT, (const void*)((12*((GLint)map_size.w*(GLint)map_size.h))*sizeof(GLuint)));
			
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);
		
		//unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glEndList();
	}
	
	minimap->initialise();
	
	initialised = true;
}

void GLMap::generateVBO() {
	Size s = game_state->getMapSize();
	
	//Construct mega VBO
	std::vector<GLfloat> data;
	std::vector<GLuint> indices;
	
	GLuint offset = 0;
	
	for(int x=0; x<s.w; x++) {
		for(int y=0; y<s.h; y++) {
			map_board[x][y]->initialise();
			
			std::vector<GLfloat> _data;
			
			_data = map_board[x][y]->getData();
			
			
			const GLuint _indices[] = {
				offset, offset+4, offset+1,
				offset+2, offset+4, offset+3,
				offset+0, offset+3, offset+4,
				offset+1, offset+4, offset+2
			};
			
			data.insert(data.end(), _data.begin(), _data.end());
			indices.insert(indices.end(), _indices, _indices + 12);
			
			//Five indices per quad
			offset += 5;
		}
	}
	
	//Now for the water!
	for(int x=0; x<s.w; x++) {
		for(int y=0; y<s.h; y++) {
			Point p = Point((GLfloat)x, (GLfloat)y);
			
			QuadColours cs = QuadColours(
				colour_grid[x][y],
				colour_grid[x][y+1],
				colour_grid[x+1][y+1],
				colour_grid[x+1][y]
			);
			
			const GLfloat _data[] = {
				// Point -----------------------------------------------------------Normal------------------Colour------------------------------Tex Coords---
				p.x*TILE_SIZE,				p.y*TILE_SIZE,				-0.4f,		0.0f, 0.0f, 1.0f,		cs.a.r, cs.a.g, cs.a.b, cs.a.a,		0.0f, 0.0f,
				p.x*TILE_SIZE, 				p.y*TILE_SIZE+TILE_SIZE,	-0.4f,		0.0f, 0.0f, 1.0f,		cs.b.r, cs.b.g, cs.b.b, cs.b.a,		0.0f, 1.0f,
				p.x*TILE_SIZE+TILE_SIZE,	p.y*TILE_SIZE+TILE_SIZE,	-0.4f,		0.0f, 0.0f, 1.0f,		cs.c.r, cs.c.g, cs.c.b, cs.c.a,		1.0f, 1.0f,
				p.x*TILE_SIZE+TILE_SIZE,	p.y*TILE_SIZE,				-0.4f,		0.0f, 0.0f, 1.0f,		cs.d.r, cs.d.g, cs.d.b, cs.d.a,		1.0f, 0.0f
			};
			
			const GLuint _indices[] = {
				offset, offset+2, offset+1,
				offset, offset+3, offset+2
			};
			
			data.insert(data.end(), _data, _data + 48);
			indices.insert(indices.end(), _indices, _indices + 6);
			
			offset += 4;
		}
	}
	
	index_count = indices.size();
	vertex_data_size = data.size();
	
	//Generate and populate mega VBO
	//Vertex Data:
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*data.size(), data.data(), GL_STATIC_DRAW);
	
	//Index Data:
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*indices.size(), indices.data(), GL_STATIC_DRAW);
	
	//Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLMap::loadShaders() {
	land_shader = new GLShader(readFile("resources/shaders/land.frag"), readFile("resources/shaders/land.vert"));
	water_shader = new GLShader(readFile("resources/shaders/water.frag"), readFile("resources/shaders/water.vert"));
	
	land_shader->initialise();
	water_shader->initialise();

	GLfloat light_La[] = {0.8f, 0.8f, 0.8f};
	GLfloat light_Ld[] = {1.0f, 1.0f, 1.0f};
	GLfloat light_Ls[] = {0.2f, 0.2f, 0.2f};
	
	GLfloat material_Ka[] = {0.4f, 0.4f, 0.4f};
	GLfloat material_Kd[] = {1.0f, 1.0f, 1.0f};
	GLfloat material_Ks[] = {0.2f, 0.2f, 0.2f};
	GLfloat material_shininess = 10.0f;
	
	land_shader->use();
	
	land_shader->setUniform("light.La", light_La[0], light_La[1], light_La[2]);
	land_shader->setUniform("light.Ld", light_Ld[0], light_Ld[1], light_Ld[2]);
	land_shader->setUniform("light.Ls", light_Ls[0], light_Ls[1], light_Ls[2]);
	
	land_shader->setUniform("material.Ka", material_Ka[0], material_Ka[1], material_Ka[2]);
	land_shader->setUniform("material.Kd", material_Kd[0], material_Kd[1], material_Kd[2]);
	land_shader->setUniform("material.Ks", material_Ks[0], material_Ks[1], material_Ks[2]);
	land_shader->setUniform("material.Shininess", material_shininess);
	
	land_shader->setUniform("tex0", 0);
	land_shader->setUniform("tex1", 1);
	
	water_shader->use();
	
	water_shader->setUniform("light.La", light_La[0], light_La[1], light_La[2]);
	water_shader->setUniform("light.Ld", light_Ld[0], light_Ld[1], light_Ld[2]);
	water_shader->setUniform("light.Ls", light_Ls[0], light_Ls[1], light_Ls[2]);
	
	water_shader->setUniform("material.Ka", material_Ka[0], material_Ka[1], material_Ka[2]);
	water_shader->setUniform("material.Kd", material_Kd[0], material_Kd[1], material_Kd[2]);
	water_shader->setUniform("material.Ks", material_Ks[0], material_Ks[1], material_Ks[2]);
	water_shader->setUniform("material.Shininess", material_shininess);
	
	water_shader->setUniform("tex0", 0);
	//water_shader->setUniform("tex1", 1);
}

GLuint GLMap::getShaderProgram() {
	return water_shader->getId();
}

void GLMap::draw() {
	water_shader->use();
	
	water_shader->setUniform("time", ((GLfloat)(time_loop%2000000))/2000000.0f);

	glCallList(map_grid_list);
	
	Uint64 current_time = glEngine->getTimeUs();
	time_loop = (time_loop+(current_time-last_draw_time))%2000000;
	last_draw_time = current_time;
}

GLMiniMap *GLMap::getMinimap() {
	return minimap;
}