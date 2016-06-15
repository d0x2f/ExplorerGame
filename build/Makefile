CC =		g++ -g
INCLUDES =	-L/mingw/lib -I"../src/"
CFLAGS =	-c -Wall
CPPFLAGS =	-c -Wall -std=c++11 -pthread `sdl2-config --cflags`  $(INCLUDES)
LIBS =		-lmingw32 -lpthread -lboost_filesystem-mgw48-mt-1_55 -lboost_system-mgw48-mt-1_55 -lglu32 -lopengl32 -lftgl -lSDL2main -lSDL2 -mwindows -lSDL2_image 
CPPSOURCES =	../src/main.cpp \
		../src/GL/GLEngine.cpp \
		../src/GL/GLHud.cpp \
		../src/GL/GLMap.cpp \
		../src/GL/GLMapTile.cpp \
		../src/GL/GLQuad.cpp \
		../src/GL/GLMesh.cpp \
		../src/GL/GLUnit.cpp \
		../src/GL/GLMiniMap.cpp \
		../src/GL/GLMeshInstance.cpp \
		../src/GL/GLGameObject.cpp \
		../src/GL/GLShader.cpp \
		../src/GL/GLMeshGroup.cpp \
		../src/GL/gl_core_3_1.c \
		../src/GAME/GameEngine.cpp \
		../src/GAME/GameState.cpp \
		../src/GAME/GameObject.cpp \
		../src/GAME/Map.cpp \
		../src/GAME/MapTile.cpp \
		../src/globals.cpp \
		../src/utils.cpp
CPPHEADERS =	../src/GL/GLEngine.h \
		../src/GL/GLHud.h \
		../src/GL/GLMap.h \
		../src/GL/GLMapTile.h \
		../src/GL/GLQuad.h \
		../src/GL/GLMesh.h \
		../src/GL/GLUnit.h \
		../src/GL/GLMiniMap.h \
		../src/GL/GLMeshInstance.h \
		../src/GL/GLGameObject.h \
		../src/GL/GLShader.h \
		../src/GL/GLMeshGroup.h \
		../src/GL/gl_core_3_1.h \
		../src/GAME/GameEngine.h \
		../src/GAME/GameState.h \
		../src/GAME/GameObject.h \
		../src/GAME/Map.h \
		../src/GAME/MapTile.h \
		../src/types.h \
		../src/globals.h \
		../src/utils.h
OBJECTS =	$(CPPSOURCES:.cpp=.o) $(CSOURCES:.c=.o)
EXECUTABLE =	Explore


all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LIBS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
.cpp.o:
	$(CC) $(CPPFLAGS) $< -o $@

clean:
	rm -f ../src/GL/*.o
	rm -f ../src/GAME/*.o
	rm -f ../src/*.o
