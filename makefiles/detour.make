include makefile.common

# This may change
INCLUDES += -I../ThirdParty/recastnavigation/Detour/Include -I../ThirdParty/recastnavigation/DetourTileCache/Include
TARGETDIR = ../Lib/x64/$(CONFIG)
TARGET = $(TARGETDIR)/libdetour.a
SOURDEDIR = ../ThirdParty/recastnavigation
OBJDIR = obj/x64/$(CONFIG)/recastnavigation
CXXFLAGS += -Werror -Wno-class-memaccess -Wno-maybe-uninitialized
# End changes

SRC_FILES = $(wildcard $(SOURDEDIR)/Detour/Source/*.cpp $(SOURDEDIR)/DetourTileCache/Source/*.cpp)

CXXFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC_FILES))
#$(info $(OBJ_FILES))

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@$(MKDIR_P) $(@D)
	$(LINKCMD_LIB) $(OBJ_FILES)

$(OBJDIR)/%.o: $(SOURDEDIR)/%.cpp
	@$(MKDIR_P) $(@D)
	$(PRE_CXX) $(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(TARGET) $(OBJDIR)/*.d
