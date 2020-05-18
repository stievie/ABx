include makefile.common

# This may change
INCLUDES += -I../absmath/absmath -I../abscommon/abscommon -I../Include/DirectXMath -I../Include/stb
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/import$(SUFFIX)
SOURDEDIR = ../import/import
OBJDIR = obj/x64/$(CONFIG)/import
LIBS += -labsmath -labscommon -lpthread -lEASTL -lassimp
CXXFLAGS += -Werror -Wno-unused-variable -Wno-deprecated-copy
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
	$(PRE_CXX) $(CXX) $(CXXFLAGS) -MMD -c $< -o $@

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(TARGET) $(OBJ_FILES:.o=.d)

