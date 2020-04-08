include makefile.common

# This may change
INCLUDES += -I../abserv/abserv -I../Include/recastnavigation -I../Include/DirectXMath -I../absmath -I../abscommon -I../abshared -I../abai -I../abipc
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/abserv$(SUFFIX)
SOURDEDIR = ../abserv/abserv
OBJDIR = obj/x64/$(CONFIG)/abserv
LIBS += -lpthread -llua5.3 -labscommon -labcrypto -labsmath -labai -labipc -labshared -lpugixml -ldetour -lstdc++fs -luuid -ldl -ldeathhandler
CXXFLAGS += -fexceptions -Werror -Wno-maybe-uninitialized
PCH = $(SOURDEDIR)/stdafx.h
# End changes

SRC_FILES = $(filter-out $(SOURDEDIR)/stdafx.cpp, $(wildcard $(SOURDEDIR)/*.cpp $(SOURDEDIR)/*/*.cpp))

CXXFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC_FILES))
#$(info $(OBJ_FILES))
GCH = $(PCH).gch

all: $(TARGET)

$(TARGET): $(GCH) $(OBJ_FILES)
	@$(MKDIR_P) $(@D)
	$(LINKCMD_EXE) $(OBJ_FILES) $(LIBS)

$(OBJDIR)/%.o: $(SOURDEDIR)/%.cpp
	@$(MKDIR_P) $(@D)
	$(PRE_CXX) $(CXX) $(CXXFLAGS) -MMD -c $< -o $@

# PCH
$(GCH): $(PCH)
	$(CXX) -x c++-header $(CXXFLAGS) -c $< -o $@

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(GCH) $(OBJ_FILES) $(TARGET)  $(OBJDIR)/*.d  $(OBJDIR)/*/*.d
