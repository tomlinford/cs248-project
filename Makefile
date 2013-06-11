TARGET  := EndersGame
FILES   := Level main Networking Scene Objects/Flyable Objects/Map Objects/Object
FILES   += Utilities/Bitmap Utilities/Buffer Utilities/FBO Utilities/Frustum
FILES   += Utilities/Model Utilities/OBJFile Utilities/ParticleSystem
FILES   += Utilities/RenderBuffer Utilities/Screen Utilities/Texture
FILES   += Utilities/Program Objects/Turret
FILES   += Menu Sound HUD HudElement


CC                  := clang++
LD                  := clang++
OBJSUFFIX	        := .o
LIBPREFIX	        := lib
STATIC_LIBSUFFIX    := .a
CFLAGS 		        := -g
CFLAGS_PLATFORM     := 
LDFLAGS		        := -m32
FRAMEWORKS	        :=
LIBS		        :=
EXESUFFIX           :=
DEPARGS             := -MD

# How we link to libraries depends on the platform
ARCH=$(shell uname | sed -e 's/-.*//g')

ifeq ($(ARCH), CYGWIN_NT)
# If building on Cygwin, use glut32, opengl32
EXESUFFIX           := .exe
LIBS                += glut32 opengl32
else

ifeq ($(ARCH), Darwin)
# Building on Mac
CFLAGS_PLATFORM		+= -stdlib=libc++
FRAMEWORKS          += OpenGL GLUT
CFLAGS				+= -m32
LIBS				+= glfw boost_system-mt boost_timer-mt boost_iostreams-mt
LIBS				+= fmodex ftgl
else
# Building on Linux
LIBS                += GLEW GL glfw boost_system boost_timer fmodex64 ftgl
endif

endif # Not CYGWIN_END


TARGET          := $(addsuffix $(EXESUFFIX), $(TARGET))
INCDIRS         := . Utilities Objects
LIBDIRS         :=

CFLAGS          += -std=c++11 -O2 $(addprefix -I, $(INCDIRS)) $(CFLAGS_PLATFORM)
LDFLAGS         += $(addprefix -L, $(LIBDIRS))

LDLIBS          := $(addprefix -l, $(LIBS))
LDFRAMEWORKS    := $(addprefix -framework , $(FRAMEWORKS))


OBJS := $(addsuffix $(OBJSUFFIX), $(FILES))
DEPS := $(addsuffix .d, $(FILES))

.SUFFIXES: .cpp $(OBJSUFFIX)

.PHONY: clean release all

all: $(TARGET)

# Handle dependencies
-include $(patsubst %.o,%.d,$(OBJS))

$(TARGET): $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LDLIBS) $(LDFRAMEWORKS)

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $< $(DEPARGS)

clean:
	rm -rf $(OBJS) $(TARGET) $(DEPS)

release:
	@make --no-print-directory RELEASE=1
