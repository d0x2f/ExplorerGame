#include "../globals.h"

#include "../GAME/Map.h"
#include "../GAME/MapTile.h"

Map::Map(GameState *game_state, Size size) : game_state(game_state), size(size) {
	map_board = std::vector< std::vector<MapTile*> >( size.w, std::vector<MapTile*> ( size.h, NULL ) );
	colour_grid = std::vector< std::vector<Colour> >( size.w+1, std::vector<Colour> ( size.h+1, Colour() ) );
	height_grid = std::vector< std::vector<GLint> >( size.w+1, std::vector<GLint> ( size.h+1, 0 ) );
}

Map::~Map() {
}

std::vector<std::vector<GLint> > Map::getHeights() {
	return height_grid;
}

bool Map::inBounds(GLint x, GLint y) {
	return (x>=0 && x<size.w && y>=0 && y<size.h);
}

bool Map::inBounds(GLint x, GLint y, Size s) {
	return (x>=0 && x<s.w && y>=0 && y<s.h);
}

std::vector<Point> Map::getBuildableSquares() {
	return buildableSquares;
}

GLfloat Map::getHeightAt(Point p) {
	GLint x,y;
	x = (GLint)floor(p.x/TILE_SIZE);
	y = (GLint)floor(p.y/TILE_SIZE);
	return map_board[x][y]->findIntersection(Ray(Point(p.x, p.y, 20.0f), Point(p.x, p.y, -20.0f), Point(0.0f, 0.0f, -1.0f))).z;
}

QuadInts Map::getSquaredHeightsAt(std::vector<std::vector<GLint> > *height_grid, Pointi p) {
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
		((p.x==0||p.y==size.h-1)?-1:max(max(max(h_02, h_01), h_12), h_11)),
		((p.x==size.w-1||p.y==size.h-1)?-1:max(max(max(h_12, h_22), h_21), h_11)),
		((p.x==size.w-1||p.y==0)?-1:max(max(max(h_21, h_20), h_10), h_11)));
}

void Map::generateNewRandomMap() {
	std::vector<std::vector<GLint> > height_grid_tmp( size.w, std::vector<GLint> ( size.h, 0 ) );
	
	for(int x=0; x<=size.w; x++) {
		for(int y=0; y<=size.h; y++) {
			GLfloat gray = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))*0.3f+0.7f;
			colour_grid[x][y] = Colour(gray, gray, gray, 1.0f);
			
			if(x != size.w && y != size.h) {
				height_grid_tmp[x][y] = -1;
				
				if(pow(x-96, 2)+pow(y-64, 2) <= 400) {
					height_grid_tmp[x][y] = rand()%7-3;
				}
				
				
				if(pow(x-32, 2)+pow(y-64, 2) <= 400) {
					height_grid_tmp[x][y] = rand()%7-3;
				}
			}
		}
	}
	
	for(int x=0; x<size.w; x++) {
		for(int y=0; y<size.h; y++) {
			colour_grid[x][y].blend(
				(x==0?Colour(0.0f, 0.0f, 0.0f, 1.0f):colour_grid[x-1][y]), 
				((x==0||y==0)?Colour(0.0f, 0.0f, 0.0f, 1.0f):colour_grid[x-1][y-1]),
				(y==0?Colour(0.0f, 0.0f, 0.0f, 1.0f):colour_grid[x][y-1]),
				colour_grid[x+1][  y], 
				colour_grid[  x][y+1], 
				colour_grid[x+1][y+1], 
				(x==0?Colour(0.0f, 0.0f, 0.0f, 1.0f):colour_grid[x-1][y+1]),
				(y==0?Colour(0.0f, 0.0f, 0.0f, 1.0f):colour_grid[x+1][y-1]));
			
			if(pow(x-32, 2)+pow(y-64, 2) <= 400 || pow(x-96, 2)+pow(y-64, 2) <= 400)
			height_grid_tmp[x][y] = averagei(9, 
				(x==0?				0:height_grid_tmp[x-1][y]), 
				((x==0||y==0)?		0:height_grid_tmp[x-1][y-1]),
				(y==0?				0:height_grid_tmp[x][y-1]),
				(x==size.w-1?	0:height_grid_tmp[x+1][y]),
									  height_grid_tmp[x][y], 
				(y==size.h-1?	0:height_grid_tmp[x][y+1]),
				(y==size.h-1 || x==size.w-1 ?	0:height_grid_tmp[x+1][y+1]),
				(y==size.h-1 || x==0 ?				0:height_grid_tmp[x-1][y+1]),
				(y==0 || x==size.w-1 ?				0:height_grid_tmp[x+1][y-1]));
		}
	}
	
	//Create MapTile Objects
	for(int x=0; x<size.w; x++) {
		for(int y=0; y<size.h; y++) {
			
			QuadInts heights = getSquaredHeightsAt(&height_grid_tmp, Pointi(x, y, 0));
			
			height_grid[x][y] = heights.a;
			height_grid[x][y+1] = heights.b;
			height_grid[x+1][y+1] = heights.c;
			height_grid[x+1][y] = heights.d;
			
			//Find build-able squares
			if(heights.a == heights.b && heights.a == heights.c && heights.a == heights.d && heights.a >= 0)
				buildableSquares.push_back(Point(x, y, heights.a));
			
			map_board[x][y] = new MapTile(game_state, Pointi(x, y), heights);
		}
	}
}