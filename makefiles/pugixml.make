include makefile.common

# This may change
TARGETDIR = ../Lib/x64/Release
TARGET = $(TARGETDIR)/libpugixml.a
SOURDEDIR = ../ThirdParty/PugiXml/src
OBJDIR = obj/x64/Release/pugixml
CXXFLAGS += -Werror -Wno-class-memaccess -Wimplicit-fallthrough=0 -Wno-deprecated-copy
# End changes

SRC_FILES = $(wildcard $(SOURDEDIR)/*.cpp)

CXXFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC_FILES))
#$(info $(OBJ_FILES))

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@$(MKDIR_P) $(@D)
	$(LINKCMD_LIB) $(OBJ_FILES)

$(OBJDIR)/%.o: $(SOURDEDIR)/%.cpp
	@$(MKDIR_P) $(@D)
	$(CCACHE) $(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(TARGET)
