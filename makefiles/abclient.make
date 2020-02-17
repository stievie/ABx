include makefile.common

# This may change
INCLUDES += -I../libabclient/libabclient -I../abshared -I../Include/Urho3D -I../Include/Urho3D/ThirdParty
TARGETDIR = ../abclient/bin
TARGET = $(TARGETDIR)/fw$(SUFFIX)
SOURDEDIR = ../abclient/abclient
OBJDIR = obj/x64/$(CONFIG)/abclient
LIBS += -lpthread -lGL -lcrypto -lssl -lUrho3D -labclient -labshared -ldl -lrt -lstdc++fs -labcrypto -ltinyexpr
CXXFLAGS += -fexceptions
PCH = $(SOURDEDIR)/stdafx.h
# End changes

# Remove -Wextra
CXXFLAGS :=$(filter-out -Wextra,$(CXXFLAGS))

SRC_FILES = $(filter-out $(SOURDEDIR)/stdafx.cpp, $(wildcard $(SOURDEDIR)/*.cpp))

CXXFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC_FILES))

GCH = $(PCH).gch

all: $(TARGET)

# Link
$(TARGET): $(GCH) $(OBJ_FILES)
	@$(MKDIR_P) $(@D)
	$(LINKCMD_EXE) $(OBJ_FILES) $(LIBS)

# Compile
$(OBJDIR)/%.o: $(SOURDEDIR)/%.cpp
	@$(MKDIR_P) $(@D)
	$(CCACHE) $(CXX) $(CXXFLAGS) -MMD -c $< -o $@

# PCH
$(GCH): $(PCH)
	$(CXX) -x c++-header $(CXXFLAGS) -c $< -o $@

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(GCH) $(OBJ_FILES) $(TARGET) $(OBJDIR)/*.d
