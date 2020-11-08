#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need GLFW (http://www.glfw.org):
# Linux:
#   apt-get install libglfw-dev
# Mac OS X:
#   brew install glfw
# MSYS2:
#   pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw
#

#CXX = g++
#CXX = clang++

EXE = ./TinyTriangle
SOURCES = ./src/main.cpp
SOURCES += ../depends/imgui/imgui_impl_glfw.cpp ../imgui_impl_opengl3.cpp
SOURCES += ./src/utils.cpp ./src/perlin.cpp ./src/createimage.cpp 
SOURCES += ../../depends/imgui/imgui.cpp ../../depends/imgui/imgui_demo.cpp ../../depends/imgui/imgui_draw.cpp ../../depends/imgui/imgui_widgets.cpp

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)

CXXFLAGS = -I./src/ -I../ -I../../ -I./depends/imgui -I./depends/glm
CXXFLAGS += -g -Wall -Wformat
LIBS =

##---------------------------------------------------------------------
## OPENGL LOADER
##---------------------------------------------------------------------

## Using OpenGL loader: gl3w
SOURCES += ./depends/gl3w/GL/gl3w.c
CXXFLAGS += -I./depends/gl3w -DIMGUI_IMPL_OPENGL_LOADER_GL3W

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------
ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += -lGL `pkg-config --static --libs glfw3`

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += -L/usr/local/lib -L/opt/local/lib
	#LIBS += -lglfw3
	LIBS += -lglfw

	CXXFLAGS += -I/usr/local/include -I/opt/local/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lglfw3 -lgdi32 -lopengl32 -limm32

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------
CXX = g++ --std=c++14
CC = cc

%.o:./depends/imgui/%.cpp
	$(CXX) $(CXXFLAGS) -c -o ./objs/$@ $<

%.o:../%.cpp
	$(CXX) $(CXXFLAGS) -c -o ./objs/$@ $<

%.o:../../%.cpp
	$(CXX) $(CXXFLAGS) -c -o ./objs/$@ $<

%.o:./depends/gl3w/GL/%.c
	$(CC) $(CFLAGS) -c -o ./objs/$@ $<

%.o:./src/%.cpp
	mkdir -p ./objs
	$(CXX) $(CXXFLAGS) -c -o ./objs/$@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@  $(addprefix ./objs/, $(OBJS)) $(CXXFLAGS) $(LIBS)

clean:
#rm -f $(EXE) $(addprefix ./objs/, $(OBJS))
	rm -rf ./objs
	rm ./imgui.ini
	rm $(EXE)
