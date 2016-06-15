#include "../types.h"
#include "../globals.h"

#include "GLHud.h"
#include "GLEngine.h"

GLHud::GLHud(GameState *game_state, VideoInfo *video_info, GLEngine *glEngine) : game_state(game_state), video_info(video_info), glEngine(glEngine) {
	precomputeValues();
}

GLHud::~GLHud() {
	clean();
}

void GLHud::setupQuads() {
	Size ws = video_info->window_size;
	GLuint tex = glEngine->loadTexture("resources/textures/hud.png");
	GLuint middle_tex = glEngine->loadTexture("resources/textures/hud_middle.png");
	
	left_hud = new GLQuad(Size(160.0f, 160.0f), Point(0.0f, ws.h-160.0f), Colour(), video_info);
	middle_hud = new GLQuad(Size(ws.w-672.0f, 160.0f), Point(160.0f, ws.h-160.0f), Colour(), video_info);
	right_hud = new GLQuad(Size(512.0f, 299.0f), Point(ws.w-512.0f, ws.h-299.0f), Colour(), video_info);
	
	left_hud->setTexture(tex, Size(512.0f, 512.0f), Point(), Point(160.0f, 160.0f));
	middle_hud->setTexture(middle_tex, Size(128.0f, 256.0f), Point(0.0f, 0.0f), Point( 128.0f*((ws.w-672.0f)/128.0f), 160.0f ) );
	right_hud->setTexture(tex, Size(512.0f, 512.0f), Point(0.0f, 213.0f), Point(512.0f, 512.0f));
}

void GLHud::clean() {
	delete left_hud;
	delete middle_hud;
	delete right_hud;
	
	glDeleteLists(hud_list, 1);
}

void GLHud::initialise() {
	setupQuads();
	
	left_hud->initialise();
	middle_hud->initialise();
	right_hud->initialise();
	
	hud_list = glGenLists(1);
	glNewList(hud_list, GL_COMPILE);
	
	glPushMatrix();
	
	glTranslatef(window_center.x, window_center.y, 0.0f);
	
	glLineWidth (3.0f);
	
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	
	glVertex3d(-crossheir_size.x, 0, 0.0f);
	glVertex3d(crossheir_size.x, 0, 0.0f);
	
	glVertex3d(0, -crossheir_size.y, 0.0f);
	glVertex3d(0, crossheir_size.y, 0.0f);
	
	glEnd();
	
	glPopMatrix();
	
	glEndList();
}

void GLHud::precomputeValues() {
	window_center = Point(video_info->window_size.w/2.0f, video_info->window_size.h/2.0f);
	crossheir_size = Point(5, 5);
}

void GLHud::draw() {
	left_hud->draw();
	middle_hud->draw();
	right_hud->draw();
	
	//glCallList(hud_list);
}

void GLHud::resize() {
	clean();
	setupQuads();
	initialise();
}