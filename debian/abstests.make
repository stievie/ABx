include makefile.common

# This may change
INCLUDES += -I../Include/DirectXMath -I../absmath/absmath -I../abscommon/abscommon 
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/Tests
SOURDEDIR = ../Tests/Tests
OBJDIR = obj/x64/Release/Tests
LIBS += -labscommon -labsmath
CXXFLAGS += -fexceptions
DEFINES += -DBUILD_INTRINSICS_LEVEL=1
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
