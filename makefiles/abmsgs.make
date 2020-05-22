include makefile.common

# This may change
INCLUDES += -I../abscommon
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/abmsgs$(SUFFIX)
SOURDEDIR = ../abmsgs/abmsgs
OBJDIR = obj/x64/$(CONFIG)/abmsgs
LIBS += -lpthread -labcrypto -labscommon -luuid -lEASTL -llua5.3 -ldeathhandler -ldl
PCH = $(SOURDEDIR)/stdafx.h
CXXFLAGS += -fexceptions -Werror -Wno-unused-variable -Wno-deprecated-copy
# End changes

SRC_FILES = $(filter-out $(SOURDEDIR)/stdafx.cpp, $(wildcard $(SOURDEDIR)/*.cpp))

CXXFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC_FILES))
$(info $(OBJ_FILES))
GCH = $(PCH).gch

all: $(TARGET)

$(TARGET): $(GCH) $(OBJ_FILES)
	@$(MKDIR_P) $(@D)
	$(LINKCMD_EXE) $(OBJ_FILES) $(LIBS)

$(OBJDIR)/%.o: $(SOURDEDIR)/%.cpp
	@$(MKDIR_P) $(@D)
	$(PRE_CXX) $(CXX) $(CXXFLAGS) -include $(PCH) -MMD -c $< -o $@

# PCH
$(GCH): $(PCH)
	$(CXX) -x c++-header $(CXXFLAGS) -c $< -o $@

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(GCH) $(OBJ_FILES) $(TARGET) $(OBJDIR)/*.d
