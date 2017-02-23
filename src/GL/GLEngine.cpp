#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#include "globals.h"

#include "GLEngine.h"
#include "../GAME/Map.h"

GLEngine::GLEngine(GameState *game_state, Uint32 game_event_id, Uint32 GL_event_id) : game_state(game_state), game_event_id(game_event_id), GL_event_id(GL_event_id) {
	exit_loop = false;
	last_frame_time = 0;
	font = NULL;
	inFullscreen = false;
	minimapFollowMouse = false;
	fps=0;
	selected_object = -1;
	initialised = false;
	loading = true;
	
	lastWindowSize = Size(800, 600);
	video_info = new VideoInfo(lastWindowSize, 60.0f, 0.5f, 100.0f);
	camera_position = Point(2.0f, 2.0f ,16.0f);
	
	hud = new GLHud(game_state, video_info, this);
	map = new GLMap(game_state, video_info, this, minimap_pos);
	cursor = new GLQuad(Size(20.0f,20.0f), Point(), Colour(1.0f, 1.0f, 1.0f, 1.0f), video_info);
	
	minimap = map->getMinimap();
	
	keys = 0;
}

GLEngine::~GLEngine() {
	if(font != NULL)
		delete this->font;
	
	if(GLQuad::ibo != 0)
		glDeleteBuffers(1, &GLQuad::ibo);
	if(cursor_texture != 0)
		glDeleteTextures(1, &cursor_texture);
	
	delete cursor;
	delete map;
	delete hud;
	delete general_shader;
	
	for (std::map<int, GLGameObject *>::iterator it = game_objects.begin() ; it != game_objects.end(); ++it) {
		delete it->second;
	}
	game_objects.clear();
	
	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	
	#if DEBUG == 1
	std::cout << "Contexts Destroyed" << std::endl;
	#endif
}

bool GLEngine::initialise() {
	font = new FTGLPixmapFont("./resources/fonts/FreeSansBold.ttf");
	
	if(font->Error()) {
		std::cerr << "Couldn't load ttf font: ./resources/fonts/FreeSansBold.ttf" << std::endl;
		//cerr << "FTGL Error Code: " << ftglGetFontError() << endl;
		return false;
	}
	
	font->FaceSize(18);
	
	if(!initSDL())
		return false;
	
	if(!initGL())
		return false;
		
	return true;
}

bool GLEngine::initGame() {
	loadShader();
	
	map->initialise();
	hud->initialise();
	
	cursor_texture = loadTexture("resources/textures/cursor.png");
	cursor->setTexture(cursor_texture);
	cursor->initialise();
	
	precomputeValues();
	
	
	GLMesh *house = new GLMesh("resources/models/house.obj", video_info, this);
	GLMesh *person = new GLMesh("resources/models/person.obj", video_info, this, true);
	GLMesh *tree = new GLMesh("resources/models/tree2.obj", video_info, this);
	
	house->load();
	house->setScale(2.0f);
	
	person->load();
	person->setScale(0.8f);
	
	tree->load();
	tree->setScale(1.0f);
	
	meshes[HOUSE] = house;
	meshes[MALE_PERSON] = person;
	meshes[TREE] = tree;
	
	glGenBuffers(1, &uniform_buffer_id);
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_id);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GLfloat)*36, NULL, GL_DYNAMIC_DRAW);
	 
	matrices_block_index = glGetUniformBlockIndex(map->getShaderProgram(), "matrices");
	glUniformBlockBinding(map->getShaderProgram(), matrices_block_index, 0);
	
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer_id);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	std::vector<Point> potential_spots = game_state->getMap()->getBuildableSquares();
	
	GLMeshGroup *trees = new GLMeshGroup();
	GLMeshGroup *houses = new GLMeshGroup();
	GLMeshGroup *people = new GLMeshGroup();
	
	mesh_groups[TREE] = trees;
	mesh_groups[HOUSE] = houses;
	mesh_groups[MALE_PERSON] = people;
	
	std::map<int, GameObject *> *objects = game_state->getObjects();
	
	for(std::map<int, GameObject *>::iterator it=objects->begin(); it != objects->end(); it++) {
		GAME_OBJECT_TYPE type = it->second->getType();
		GLGameObject *go = new GLGameObject(game_state, it->first, it->second, meshes[type], it->second->getPosition(), this);
		go->initialise();
		
		game_objects[it->first] = go;
		mesh_groups[type]->addInstance(go->getMeshInstance());
	}
	return true;
}

bool GLEngine::initSDL() {
	#if DEBUG == 1
	std::cout << "Initialising SDL..." << std::endl;
	#endif
	
	/* Initialize SDL video subsystem */
	if((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)) { 
		std::cerr << "Could not initialise SDL: " << SDL_GetError() << std::endl;
		return false;
	}
	
	atexit(SDL_Quit);
	
	SDL_RendererInfo rendererInfo;
	
	SDL_CreateWindowAndRenderer(video_info->window_size.w, video_info->window_size.h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE| SDL_WINDOW_ALLOW_HIGHDPI, &window, &renderer);
	SDL_GetRendererInfo(renderer, &rendererInfo);
	if ((rendererInfo.flags & SDL_RENDERER_ACCELERATED) == 0 ||
		(rendererInfo.flags & SDL_RENDERER_TARGETTEXTURE) == 0) {
		std::cerr << "Couldn't create an SDL window: " << SDL_GetError() << std::endl;
		return false;
	}
	
	SDL_SetWindowTitle(window, "Explorer");
	
	/*if(SDL_SetVideoMode(video_info->window_size.w, video_info->window_size.h, video->vfmt->BitsPerPixel, SDL_DOUBLEBUF | SDL_OPENGL | SDL_HWSURFACE | SDL_RESIZABLE) == 0) {
	//if(SDL_SetVideoMode(video->current_w, video->current_h, video->vfmt->BitsPerPixel, SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_OPENGL | SDL_HWSURFACE | SDL_RESIZABLE) == 0) {
		std::cerr << "Couldn't set video mode: " << SDL_GetError() << std::endl;
		return false;
	}*/
	
	//Choose context version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); 
	
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	//SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL,0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,16);

	//SDL_GL_SetSwapInterval(0);
	
	gl_context = SDL_GL_CreateContext(window);
	if(gl_context == NULL) { 
		std::cerr << "Could not create OpenGL context: " << SDL_GetError() << std::endl;
		return false;
	}
	
	checkSDLError(__LINE__);
	
	#if DEBUG == 1
	std::cout << "SDL initialized..." << std::endl << std::endl;
	#endif
	
	initialised = true;
	return true;
}

bool GLEngine::initGL() {
	if(ogl_LoadFunctions()  == ogl_LOAD_FAILED) {
		std::cerr << "Couldn't load OpenGL Extensions:." << std::endl;
		return false;
	}
	
	#if DEBUG == 1
	std::cout << "OpenGL version: " << ogl_GetMajorVersion() << "." << ogl_GetMinorVersion() << std::endl;
	#endif
	if(ogl_GetMajorVersion()*100+ogl_GetMinorVersion() < 301) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Your computer doesn't support OpenGL 3.1.\nTry updating your drivers.", NULL);
	}
	
	GLenum error;
	
	glClearColor(0., 0. ,0., 0.);
	
	glEnable (GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	glDisable (GL_LIGHTING);
	
	glEnable(GL_NORMALIZE);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	
	glEnable (GL_TEXTURE_2D);
	
	glEnable (GL_BLEND);
	
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable (GL_LINE_SMOOTH);
	
	glDisable (GL_POLYGON_SMOOTH);
	
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	
	glHint (GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
	
	glEnable(GL_COLOR_MATERIAL);
	//glDisable(GL_COLOR_MATERIAL);
	
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	
	//SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_ShowCursor(SDL_DISABLE);
	
	error = glGetError();
	if(error != GL_NO_ERROR) {
		std::cerr << "OPENGL ERROR: " << gluErrorString(error) << "(" << error << ")" << std::endl;
		return false;
	}
	
	return true;
}

void GLEngine::loadShader() {
	general_shader = new GLShader(readFile("resources/shaders/general.frag"), readFile("resources/shaders/general.vert"));
	general_shader->initialise();
	
	GLfloat light_La[] = {0.8f, 0.8f, 0.8f};
	GLfloat light_Ld[] = {1.0f, 1.0f, 1.0f};
	GLfloat light_Ls[] = {0.3f, 0.3f, 0.3f};
	
	GLfloat material_Ka[] = {0.6f, 0.6f, 0.6f};
	GLfloat material_Kd[] = {1.0f, 1.0f, 1.0f};
	GLfloat material_Ks[] = {0.2f, 0.2f, 0.2f};
	GLfloat material_shininess = 100.0f;

	general_shader->use();
	
	general_shader->setUniform("light.La", light_La[0], light_La[1], light_La[2]);
	general_shader->setUniform("light.Ld", light_Ld[0], light_Ld[1], light_Ld[2]);
	general_shader->setUniform("light.Ls", light_Ls[0], light_Ls[1], light_Ls[2]);
	
	general_shader->setUniform("material.Ka", material_Ka[0], material_Ka[1], material_Ka[2]);
	general_shader->setUniform("material.Kd", material_Kd[0], material_Kd[1], material_Kd[2]);
	general_shader->setUniform("material.Ks", material_Ks[0], material_Ks[1], material_Ks[2]);
	general_shader->setUniform("material.Shininess", material_shininess);
	
	general_shader->setUniform("tex0", 0);
}

void GLEngine::drawLoadingScreen() {
	GLuint loading_texture = loadTexture("resources/textures/loading.png");
	GLuint loading_text_texture = loadTexture("resources/textures/loading_text.png");
	
	GLQuad loading_image(Size(video_info->window_size.w, video_info->window_size.h), Point(0.0f, 0.0f), Colour(1.0f, 1.0f, 1.0f), video_info);
	loading_image.setTexture(loading_texture);
	loading_image.initialise();	
	
	GLQuad loading_text(Size(256.0, 64.0f), Point(video_info->window_size.w/2.0f - 128.0f, video_info->window_size.h/2.0f - 32.0f), Colour(1.0f, 1.0f, 1.0f), video_info);
	loading_text.setTexture(loading_text_texture);
	loading_text.initialise();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glPushMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glOrtho(0.0f,video_info->window_size.w, video_info->window_size.h,0.0f,-1.0f,1.0f);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glDepthFunc(GL_ALWAYS);
	
	loading_image.draw();
	loading_text.draw();
	
	glDepthFunc(GL_LEQUAL);
	
	glPopMatrix();
	
	SDL_GL_SwapWindow(window);
	
	//SDL_Delay(2000);
}

void GLEngine::windowResize(Size s) {
	if(inFullscreen)
		return;
	
	video_info->resize(s);

	/*if(SDL_SetVideoMode(s.w, s.h, video->vfmt->BitsPerPixel, SDL_DOUBLEBUF | SDL_OPENGL | SDL_HWSURFACE | SDL_RESIZABLE) == 0) {
	//if(SDL_SetVideoMode(video->current_w, video->current_h, video->vfmt->BitsPerPixel, SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_OPENGL | SDL_HWSURFACE | SDL_RESIZABLE) == 0) {
		std::cerr << "Couldn't set video mode: " << SDL_GetError() << std::endl;
		SDL_Quit();
	}*/
	
	hud->resize();
	precomputeValues();
	
	glViewport(0, 0, s.w, s.h);
	
	lastWindowSize = s;
}

void GLEngine::exitFullscreen() {
	SDL_SetWindowFullscreen(window, 0);
	windowResize(lastWindowSize);
}

void GLEngine::enterFullscreen() {
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	
	SDL_DisplayMode mode;
	SDL_GetDisplayMode(0, 0, &mode);
	windowResize(Size(mode.w, mode.h));
	/*video = SDL_GetVideoInfo();
	if(video == NULL) {
		std::cerr << "Couldn't get video information: " << SDL_GetError() << std::endl;
		return;
	}
	
	video_info->resize(fullscreenSize);
	
	std::cout << video_info->window_size.w << ":" << video_info->window_size.h << std::endl;
	
	if(SDL_SetVideoMode(video_info->window_size.w, video_info->window_size.h, video->vfmt->BitsPerPixel, SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_OPENGL | SDL_HWSURFACE | SDL_RESIZABLE) == 0) {
	//if(SDL_SetVideoMode(video->current_w, video->current_h, video->vfmt->BitsPerPixel, SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_OPENGL | SDL_HWSURFACE | SDL_RESIZABLE) == 0) {
		std::cerr << "Couldn't set video mode: " << SDL_GetError() << std::endl;
		SDL_Quit();
	}
	
	hud->resize();
	precomputeValues();
	
	glViewport(0, 0, video_info->window_size.w, video_info->window_size.h);*/
}

void GLEngine::precomputeValues() {
	camera_move_factor = (sin(PI/2.0f)*WINDOW_SPEED);
	minimap_pos = Point(video_info->window_size.w-250.0f, video_info->window_size.h-250.0f);
}

void GLEngine::run() {
	SDL_Event event;
	
	while(game_state->getVar(GameState::EXIT_GAME) != 1) {
		if(loading) {
			drawLoadingScreen();
		} else {
			drawFrame();
		}
		
		SDL_PumpEvents();
		while((game_state->getVar(GameState::EXIT_GAME) != 1) && 
			(
				(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_MOUSEWHEEL) > 0) ||
				(SDL_PeepEvents(&event, 1, SDL_GETEVENT, game_event_id, game_event_id) > 0)
			)) {
			switch(event.type) {
				case SDL_QUIT:
					game_state->setVar(GameState::EXIT_GAME, 1);
					break;
					
				case SDL_KEYDOWN:
					handleKeypress(&event);
					break;
					
				case SDL_KEYUP:
					handleKeyrelease(&event);
					break;
					
				case SDL_WINDOWEVENT:
				{
					if(event.window.event == SDL_WINDOWEVENT_RESIZED)
						windowResize(Size(event.window.data1, event.window.data2));
					break;
				}
				case SDL_MOUSEWHEEL:
					handleMouseWheel(&event);
					break;
				case SDL_MOUSEBUTTONDOWN:
					handleMouseButtonDown(&event);
					break;
					
				case SDL_MOUSEBUTTONUP:
					handleMouseButtonUp(&event);
					break;
					
				case SDL_MOUSEMOTION:
					handleMouseMotion(&event);
					break;
			}
			
			if(event.type == game_event_id) {
				handleGameThreadEvent(&event);
			}
			SDL_PumpEvents();
		}
	}
}

void GLEngine::handleGameThreadEvent(SDL_Event *event) {
	switch((USEREVENTS)event->user.code) {
		case GAME_THREAD_LOADED:
			initGame();
			loading = false;
			break;
			
		default:
			break;
	}
}

void GLEngine::handleKeypress(SDL_Event *event) {
	switch(event->key.keysym.sym) {
		case SDLK_q:
			game_state->setVar(GameState::EXIT_GAME, 1);
			break;
		case SDLK_f:
			if(inFullscreen) {
				exitFullscreen();
			} else {
				enterFullscreen();
			}
			inFullscreen = !inFullscreen;
			break;
		case SDLK_BACKQUOTE:
			r_debug = !r_debug;
			break;
			
		// WASD/arrows
		case SDLK_w:
		case SDLK_UP:
			keys |= 1;
			break;
			
		case SDLK_a:
		case SDLK_LEFT:
			keys |= 2;
			break;
			
		case SDLK_s:
		case SDLK_DOWN:
			keys |= 4;
			break;
			
		case SDLK_d:
		case SDLK_RIGHT:
			keys |= 8;
			break;
		default:
			break;
	}
}

void GLEngine::handleKeyrelease(SDL_Event *event) {
	Uint8 tmpKeys = keys;
	keys = 0;
	switch(event->key.keysym.sym) {
		// WASD/arrows
		case SDLK_w:
		case SDLK_UP:
			keys |= 1;
			break;
			
		case SDLK_a:
		case SDLK_LEFT:
			keys |= 2;
			break;
			
		case SDLK_s:
		case SDLK_DOWN:
			keys |= 4;
			break;
			
		case SDLK_d:
		case SDLK_RIGHT:
			keys |= 8;
			break;
		default:
			break;
	}
	keys = ((~keys) & tmpKeys);
}

void GLEngine::handleMouseWheel(SDL_Event *event) {
	camera_position.z -= event->wheel.y;
	
	if(camera_position.z < 1.0f)
		camera_position.z = 1.0f;
	else if(camera_position.z > 40.0f)
		camera_position.z = 40.0f;
}

void GLEngine::handleMouseButtonDown(SDL_Event *event) {
	Point p;
	switch(event->button.button) {
			
		case SDL_BUTTON_LEFT:
			//Check minimap click
			p = minimap->checkClick(Point(event->button.x, event->button.y));
			if(p.z != 1.0f) {
				lookAt(p);
				minimapFollowMouse = true;
			} else {
				//Check object selection
				clickSelect(Point2D(event->button.x, event->button.y));
			}
			break;
			
		case SDL_BUTTON_RIGHT:
			//If a movable unit is selected, move it!
			if(selected_object != -1) {
				GLGameObject *go = game_objects.at(selected_object);
				if(go->isMovable())
					go->moveTo(getClickedPosition(Point2D(event->button.x, event->button.y)));
			}
			break;
		default:
			break;
	}
}

void GLEngine::handleMouseButtonUp(SDL_Event *event) {
	Point p;
	switch(event->button.button) {
		case SDL_BUTTON_LEFT:
			minimapFollowMouse = false;
			break;
		default:
			break;
	}
}

void GLEngine::handleMouseMotion(SDL_Event *event) {
	cursor_position.x = event->motion.x;
	cursor_position.y = event->motion.y;
	
	if(minimapFollowMouse) {
		Point p = minimap->checkClick(Point(event->motion.x, event->motion.y));
		if(p.z != 1.0f)
			lookAt(p);
	}
}

void GLEngine::moveCameraPosition() {
	GLfloat inc = (camera_move_factor*camera_position.z)/fps;
	
	//Forward
	if((keys & KEY_UP) == KEY_UP) {
		camera_position.x += inc;
		camera_position.y += inc;
	}
	
	//Backward
	if((keys & KEY_DOWN) == KEY_DOWN) {
		camera_position.x -= inc;
		camera_position.y -= inc;
	}
	
	//Left
	if((keys & KEY_LEFT) == KEY_LEFT) {
		camera_position.x -= inc;
		camera_position.y += inc;
	}
	
	//Right
	if((keys & KEY_RIGHT) == KEY_RIGHT) {
		camera_position.x += inc;
		camera_position.y -= inc;
	}
	
	if(camera_position.x < -50.0f) {
		camera_position.x = -50.0f;
	}
	
	if(camera_position.y < -50.0f) {
		camera_position.y = -50.0f;
	}
	
	Size map_size = game_state->getMapSize();
	if(camera_position.x > map_size.w*TILE_SIZE-5.0f) {
		camera_position.x = map_size.w*TILE_SIZE-5.0f;
	}
	
	if(camera_position.y > map_size.h*TILE_SIZE-5.0f) {
		camera_position.y = map_size.h*TILE_SIZE-5.0f;
	}
}

void GLEngine::lookAt(Point p) {
	camera_position.x = p.x-2.0f;
	camera_position.y = p.y-2.0f;
}

GLMap *GLEngine::getMap() {
	return map;
}

Ray GLEngine::getClickedRay(Point2D p) {
	GLint viewport[4];
	
	glGetIntegerv(GL_VIEWPORT, viewport);
	
	p.y = GLfloat(viewport[3] - p.y);
	
	GLdouble x, y, z;
	gluUnProject( (GLdouble) p.x, (GLdouble) p.y, 0.0f, // Near
				  modelMatrix, projectionMatrix, viewport,
			   &x, &y, &z );
	
	Point a = Point(x, y, z);
	
	gluUnProject( (GLdouble) p.x,(GLdouble)  p.y, 1.0f, // Far
				  modelMatrix, projectionMatrix, viewport,
			   &x, &y, &z );
	
	Point b = Point(x, y, z);
	
	Point dir = b - a;
	dir.normalise();
	
	#if DEBUG == 1
	click_ray = Ray(a, b, dir);
	#endif
	
	return Ray(a, b, dir);
}

Point GLEngine::getClickedPosition(Point2D p) {
	Ray ray = getClickedRay(p);
	
	return map->findIntersection(ray);;
}

void GLEngine::clickSelect(Point2D p) {
	Ray ray = getClickedRay(p);
	
	int select = -1;
	//Check for intersections
	for (std::map<int, GLGameObject *>::iterator it = game_objects.begin() ; it != game_objects.end(); ++it) {
		if(it->second->isSelectable() && it->second->intersects(ray)) {
			if(select != -1) {
				if(it->second->getSelectionRadius() < game_objects[select]->getSelectionRadius()) {
					select = it->first;
				}
			} else {
				select = it->first;
			}
		}
	}
	
	if(select != -1) {
		selected_object = select;
		std::cout << selected_object << std::endl;
		emit(GL_event_id, OBJECT_SELECTED, (void*)selected_object);
	}
}

GLfloat *GLEngine::getModelViewMatrix() {
	return modelview;
}

GLfloat *GLEngine::getProjectionMatrix() {
	return projection;
}

void GLEngine::updateMatrices() {
	glFinish();
	//Retrieve model and projection matrices for ray casting
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
	
	//Retrieve model and projection matrices for everything else
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
	glGetFloatv(GL_PROJECTION_MATRIX, projection);
	
	//Extract the frustum for each frame for out of bounds culling
	extractFrustum();
	
	GLfloat modelviewprojection[16];
	matrixMultiply44(modelview, projection, modelviewprojection);
	
	GLfloat light_position[4] = {camera_position.x, camera_position.y, camera_position.z, 1.0f};

	//Upload to shader programs
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_id);
	
	GLint indices[3];
	GLint offset[3];
	
	glGetActiveUniformBlockiv(map->getShaderProgram(), matrices_block_index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices);
	glGetActiveUniformsiv(map->getShaderProgram(), 3, (GLuint *)indices, GL_UNIFORM_OFFSET, offset);
	
	glBufferSubData(GL_UNIFORM_BUFFER, offset[0], sizeof(GLfloat)*16, modelviewprojection);
	glBufferSubData(GL_UNIFORM_BUFFER, offset[1], sizeof(GLfloat)*16, modelview);
	glBufferSubData(GL_UNIFORM_BUFFER, offset[2], sizeof(GLfloat)*4, light_position);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GLEngine::setPerspective() {
	//gluPerspective(60.0f, video_info->ratio, 0.5f, 100.0f);

	GLfloat fW, fH;
	fH = tan( video_info->fovY / 360 * PI ) * video_info->zNear;
	fW = fH * video_info->ratio;
	glFrustum( -fW, fW, -fH, fH, video_info->zNear, video_info->zFar );
}

void GLEngine::drawFrame() {
	//Calculate FPS
	Uint64 current_time = getTimeUs();
	Uint64 time_diff = current_time - this->last_frame_time;
	if(time_diff == 0) time_diff = 1;
	fps = (fps+(1000000.0f/((float)(time_diff))))/2.0f;
	last_frame_time = current_time;
	
	//Check for errors
	#if DEBUG == 1
	GLenum error = glGetError();
	if(error != GL_NO_ERROR)
		std::cerr << "OPENGL ERROR: " << gluErrorString(error) << "(" << error << ")" << std::endl;
	#endif
	
	//Move the camera based on the keys currently held
	moveCameraPosition();
	
	//Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Set projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	setPerspective();
	
	//Set model matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(
		camera_position.x, camera_position.y, camera_position.z,					//Camera Position
		camera_position.x+5.0f, camera_position.y+5.0f, camera_position.z-10.0f,	//Look at
		0.0f, 0.0f, 1.0f															//Up vector
	);
	
	glPushMatrix();
	
	updateMatrices();

	//Draw the map
	drawMap();
	
	glUseProgram(0);
	//Draw the ray cast on click
	#if DEBUG == 1
	if(r_debug) {
		glBegin(GL_LINES);

		glColor3f(1.0f, 1.0f, 0.0f);
		glVertex3f(click_ray.a.x, click_ray.a.y, click_ray.a.z);
		glVertex3f(click_ray.b.x, click_ray.b.y, click_ray.b.z);

		glEnd();
	}
	#endif

	general_shader->use();
	
	//Draw game objects
	
	//Do physics first
	/*for (std::map<int, GLGameObject *>::iterator it = game_objects.begin() ; it != game_objects.end(); ++it) {
		it->second->doPhysics();
	}*/
	
	//Draw objects in a mesh group
	for (std::map<GAME_OBJECT_TYPE, GLMeshGroup *>::iterator it = mesh_groups.begin() ; it != mesh_groups.end(); ++it) {
		it->second->draw(selected_object);
	}
	
	//Draw remaining objects
	for (std::map<int, GLGameObject *>::iterator it = game_objects.begin() ; it != game_objects.end(); ++it) {
		it->second->draw(it->first == selected_object);
		it->second->getMeshInstance()->setDrawn(false);
	}

	glPopMatrix();
	
	//Draw the hud
	drawHUD();
	
	//SDL_GL_SwapBuffers();
	SDL_GL_SwapWindow(window);
}

Point GLEngine::getCameraPosition() {
	return camera_position;
}

void GLEngine::drawHUD() {
	glUseProgram(0);

	glPushMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glOrtho(0.0f,video_info->window_size.w, video_info->window_size.h,0.0f,-1.0f,1.0f);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glDisable (GL_DEPTH_TEST);
	hud->draw();
	minimap->draw(minimap_pos);
	
	cursor->draw(cursor_position.x, cursor_position.y, 1.0f);
	glEnable (GL_DEPTH_TEST);
	
#if DEBUG == 1
	if(r_debug) {
		//Show FPS
		glPixelTransferf(GL_RED_BIAS, -1.0f);
		glPixelTransferf(GL_GREEN_BIAS, -1.0f);
		glPixelTransferf(GL_BLUE_BIAS, -0.0f);
		
		char fps_buffer[32];
		sprintf(fps_buffer, "Fps: %0.0f", fps);
		font->Render(fps_buffer);
	}
#endif
	
	glPopMatrix();
}

void GLEngine::drawMap() {
	map->draw();
}

GLShader *GLEngine::getGeneralShader() {
	return general_shader;
}

GLuint GLEngine::loadTexture(const char *filename) {
	if(textures[std::string(filename)] != 0)
		return textures[std::string(filename)];
	
	#if DEBUG == 1
	std::cout << "Loading Texture: " << filename << std::endl;
	#endif
	
	GLuint texture_id;
	SDL_Surface* surface = IMG_Load(filename);
	
	if(!surface) {
		std::cout << "Error loading texture: " << IMG_GetError() << std::endl;
		return 0;
	}
	
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	
	int mode = GL_RGB;
	
	if(surface->format->BytesPerPixel == 4) {
		mode = GL_RGBA;
	}
	
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
   glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); 
   
   glTexImage2D(GL_TEXTURE_2D, 0, mode, surface->w, surface->h, 0, mode, GL_UNSIGNED_BYTE, surface->pixels);
	
	//gluBuild2DMipmaps(GL_TEXTURE_2D, mode, surface->w, surface->h, mode, GL_UNSIGNED_BYTE, surface->pixels);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	textures[std::string(filename)] = texture_id;
	
	return texture_id;
}

Uint64 GLEngine::getTimeUs() {
	timeval t;
	gettimeofday(&t, NULL);
	return ((unsigned long long)t.tv_sec * 1000000) + t.tv_usec;
}

void GLEngine::extractFrustum() {
	GLfloat   clip[16];
	GLfloat   t;
	
	/* Combine the two matrices (multiply projection by modelview) */
	clip[ 0] = modelview[ 0] * projection[ 0] + modelview[ 1] * projection[ 4] + modelview[ 2] * projection[ 8] + modelview[ 3] * projection[12];
	clip[ 1] = modelview[ 0] * projection[ 1] + modelview[ 1] * projection[ 5] + modelview[ 2] * projection[ 9] + modelview[ 3] * projection[13];
	clip[ 2] = modelview[ 0] * projection[ 2] + modelview[ 1] * projection[ 6] + modelview[ 2] * projection[10] + modelview[ 3] * projection[14];
	clip[ 3] = modelview[ 0] * projection[ 3] + modelview[ 1] * projection[ 7] + modelview[ 2] * projection[11] + modelview[ 3] * projection[15];
	
	clip[ 4] = modelview[ 4] * projection[ 0] + modelview[ 5] * projection[ 4] + modelview[ 6] * projection[ 8] + modelview[ 7] * projection[12];
	clip[ 5] = modelview[ 4] * projection[ 1] + modelview[ 5] * projection[ 5] + modelview[ 6] * projection[ 9] + modelview[ 7] * projection[13];
	clip[ 6] = modelview[ 4] * projection[ 2] + modelview[ 5] * projection[ 6] + modelview[ 6] * projection[10] + modelview[ 7] * projection[14];
	clip[ 7] = modelview[ 4] * projection[ 3] + modelview[ 5] * projection[ 7] + modelview[ 6] * projection[11] + modelview[ 7] * projection[15];
	
	clip[ 8] = modelview[ 8] * projection[ 0] + modelview[ 9] * projection[ 4] + modelview[10] * projection[ 8] + modelview[11] * projection[12];
	clip[ 9] = modelview[ 8] * projection[ 1] + modelview[ 9] * projection[ 5] + modelview[10] * projection[ 9] + modelview[11] * projection[13];
	clip[10] = modelview[ 8] * projection[ 2] + modelview[ 9] * projection[ 6] + modelview[10] * projection[10] + modelview[11] * projection[14];
	clip[11] = modelview[ 8] * projection[ 3] + modelview[ 9] * projection[ 7] + modelview[10] * projection[11] + modelview[11] * projection[15];
	
	clip[12] = modelview[12] * projection[ 0] + modelview[13] * projection[ 4] + modelview[14] * projection[ 8] + modelview[15] * projection[12];
	clip[13] = modelview[12] * projection[ 1] + modelview[13] * projection[ 5] + modelview[14] * projection[ 9] + modelview[15] * projection[13];
	clip[14] = modelview[12] * projection[ 2] + modelview[13] * projection[ 6] + modelview[14] * projection[10] + modelview[15] * projection[14];
	clip[15] = modelview[12] * projection[ 3] + modelview[13] * projection[ 7] + modelview[14] * projection[11] + modelview[15] * projection[15];
	
	/* Extract the numbers for the RIGHT plane */
	frustum[0][0] = clip[ 3] - clip[ 0];
	frustum[0][1] = clip[ 7] - clip[ 4];
	frustum[0][2] = clip[11] - clip[ 8];
	frustum[0][3] = clip[15] - clip[12];
	
	/* Normalize the result */
	t = sqrt( frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2] );
	frustum[0][0] /= t;
	frustum[0][1] /= t;
	frustum[0][2] /= t;
	frustum[0][3] /= t;
	
	/* Extract the numbers for the LEFT plane */
	frustum[1][0] = clip[ 3] + clip[ 0];
	frustum[1][1] = clip[ 7] + clip[ 4];
	frustum[1][2] = clip[11] + clip[ 8];
	frustum[1][3] = clip[15] + clip[12];
	
	/* Normalize the result */
	t = sqrt( frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2] );
	frustum[1][0] /= t;
	frustum[1][1] /= t;
	frustum[1][2] /= t;
	frustum[1][3] /= t;
	
	/* Extract the BOTTOM plane */
	frustum[2][0] = clip[ 3] + clip[ 1];
	frustum[2][1] = clip[ 7] + clip[ 5];
	frustum[2][2] = clip[11] + clip[ 9];
	frustum[2][3] = clip[15] + clip[13];
	
	/* Normalize the result */
	t = sqrt( frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2] );
	frustum[2][0] /= t;
	frustum[2][1] /= t;
	frustum[2][2] /= t;
	frustum[2][3] /= t;
	
	/* Extract the TOP plane */
	frustum[3][0] = clip[ 3] - clip[ 1];
	frustum[3][1] = clip[ 7] - clip[ 5];
	frustum[3][2] = clip[11] - clip[ 9];
	frustum[3][3] = clip[15] - clip[13];
	
	/* Normalize the result */
	t = sqrt( frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2] );
	frustum[3][0] /= t;
	frustum[3][1] /= t;
	frustum[3][2] /= t;
	frustum[3][3] /= t;
	
	/* Extract the FAR plane */
	frustum[4][0] = clip[ 3] - clip[ 2];
	frustum[4][1] = clip[ 7] - clip[ 6];
	frustum[4][2] = clip[11] - clip[10];
	frustum[4][3] = clip[15] - clip[14];
	
	/* Normalize the result */
	t = sqrt( frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2] );
	frustum[4][0] /= t;
	frustum[4][1] /= t;
	frustum[4][2] /= t;
	frustum[4][3] /= t;
	
	/* Extract the NEAR plane */
	frustum[5][0] = clip[ 3] + clip[ 2];
	frustum[5][1] = clip[ 7] + clip[ 6];
	frustum[5][2] = clip[11] + clip[10];
	frustum[5][3] = clip[15] + clip[14];
	
	/* Normalize the result */
	t = sqrt( frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2] );
	frustum[5][0] /= t;
	frustum[5][1] /= t;
	frustum[5][2] /= t;
	frustum[5][3] /= t;
}

bool GLEngine::isVisible(Point p) {
	for(int i = 0; i < 6; i++ )
		if( frustum[i][0] * p.x + frustum[i][1] * p.y + frustum[i][2] * p.z + frustum[i][3] <= 0 )
			return false;
		return true;
}

bool GLEngine::isVisible(Point p, GLfloat radius) {
	for(int i = 0; i < 6; i++ )
		if( frustum[i][0] * p.x + frustum[i][1] * p.y + frustum[i][2] * p.z + frustum[i][3] < -radius )
			return false;
		return true;
}
