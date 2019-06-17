include common.make

# This may change
DEFINES += -D_CONSOLE
INCLUDES += -I../abscommon/abscommon -I../Include/recastnavigation -I../Include/ai -I../absmath/absmath -I../Include/DirectXMath
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/abserv
SOURDEDIR = ../abserv/abserv
OBJDIR = obj/x64/Release/abserv
LIBS += -lpthread -luuid -llua5.3 -labscommon -labcrypto -labsmath -lpugixml -ldetour -lstdc++fs
# End changes

SRC_FILES = $(wildcard $(SOURDEDIR)/*.cpp $(SOURDEDIR)/*/*.cpp)

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
