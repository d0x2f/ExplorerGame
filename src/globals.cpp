#include "globals.h"

bool r_debug = false;

void checkSDLError(int line) {
#if DEBUG ==1
        const char *error = SDL_GetError();
        if (*error != '\0')
        {
                std::cout << "SDL Error: " << error <<std::endl;
                if (line != -1)
					std::cout << " + line: " << line <<std::endl;
                SDL_ClearError();
        }
#endif
}

void checkOGLError(const char *file, int line) {
#if DEBUG ==1
        GLenum error = glGetError();
		if(error != GL_NO_ERROR) {
			std::cerr << "OPENGL ERROR: " << gluErrorString(error) << "(" << error << ")" << std::endl;
			if (line != -1)
				std::cout << " + file: " << file << " line: " << line <<std::endl;
		}
#endif
}

void emit(Uint32 event_id, USEREVENTS event_type, void *data) {
		SDL_Event event;
		SDL_zero(event);
		event.type = event_id;
		event.user.code = event_type;
		event.user.data1 = data;
		event.user.data2 = 0;
		SDL_PushEvent(&event);
}