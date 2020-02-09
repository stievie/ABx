include makefile.common

# This may change
INCLUDES += -I../abscommon/abscommon -I../abai/abai -I../abipc/abipc
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/dbgclient$(SUFFIX)
SOURDEDIR = ../dbgclient/dbgclient
OBJDIR = obj/x64/$(CONFIG)/dbgclient
LIBS += -llua5.3 -labscommon -lpthread -lncurses -labipc
CXXFLAGS += -Werror
DEFINES += -DUSE_STANDALONE_ASIO
# End changes

SRC_FILES = $(wildcard $(SOURDEDIR)/*.cpp)

CXXFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC_FILES))
#$(info $(OBJ_FILES))

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
	rm -f $(OBJ_FILES) $(TARGET) $(OBJDIR)/*.d
