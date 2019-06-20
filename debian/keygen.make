include common.make

# This may change
DEFINES += -D_CONSOLE
INCLUDES += -I../abscommon/abscommon
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/keygen
SOURDEDIR = ../keygen/keygen
OBJDIR = obj/x64/Release/keygen
LIBS += -lpthread -luuid -llua5.3 -labcrypto -labscommon
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
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(TARGET)
