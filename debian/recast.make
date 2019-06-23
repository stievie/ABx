include makefile.common

# This may change
INCLUDES += -I../ThirdParty/recastnavigation/Recast/Include
TARGETDIR = ../Lib/x64/Release
TARGET = $(TARGETDIR)/librecast.a
SOURDEDIR = ../ThirdParty/recastnavigation
OBJDIR = obj/x64/Release/recastnavigation
CXXFLAGS += -Werror
# End changes

SRC_FILES = $(wildcard $(SOURDEDIR)/Recast/Source/*.cpp)

CXXFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC_FILES))
#$(info $(OBJ_FILES))

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@$(MKDIR_P) $(@D)
	$(LINKCMD_LIB) $(OBJ_FILES)

$(OBJDIR)/%.o: $(SOURDEDIR)/%.cpp
	@$(MKDIR_P) $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(TARGET)
