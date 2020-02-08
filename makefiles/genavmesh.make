include makefile.common

# This may change
INCLUDES += -I../Include/recastnavigation -I../Include/stb
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/genavmesh$(SUFFIX)
SOURDEDIR = ../genavmesh/genavmesh
OBJDIR = obj/x64/$(CONFIG)/genavmesh
LIBS += -lrecast -ldetour -lpthread -lassimp
CXXFLAGS += 
# End changes

SRC_FILES = $(wildcard $(SOURDEDIR)/*.cpp)

CXXFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC_FILES))

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@$(MKDIR_P) $(@D)
	$(LINKCMD_EXE) $(OBJ_FILES) $(LIBS)

$(OBJDIR)/%.o: $(SOURDEDIR)/%.cpp
	@$(MKDIR_P) $(@D)
	$(CCACHE) $(CXX) $(CXXFLAGS) -MMD -c $< -o $@

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(TARGET)

