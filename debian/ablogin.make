include common.make

# This may change
DEFINES += -D_CONSOLE
INCLUDES += -I../abscommon/abscommon
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/ablogin
SOURDEDIR = ../ablogin/ablogin
OBJDIR = obj/x64/Release/ablogin
LIBS += -lpthread -luuid -llua5.3 -labscommon -labcrypto
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
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(TARGET)
