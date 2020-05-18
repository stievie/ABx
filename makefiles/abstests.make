include makefile.common

# This may change
INCLUDES += -I../Include/DirectXMath -I../absmath -I../abscommon -I../abai -I../abipc
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/Tests$(SUFFIX)
SOURDEDIR = ../Tests/Tests
OBJDIR = obj/x64/$(CONFIG)/Tests
LIBS += -labscommon -labsmath -labai -labipc -ltinyexpr -llua5.3 -lpthread
PCH = $(SOURDEDIR)/stdafx.h
CXXFLAGS += -fexceptions -Werror -Wno-unused-variable -Wno-deprecated-copy
# End changes

SRC_FILES = $(filter-out $(SOURDEDIR)/stdafx.cpp, $(wildcard $(SOURDEDIR)/*.cpp))

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
	rm -f $(GCH) $(OBJ_FILES) $(TARGET) $(OBJDIR)/*.d

.PHONY: run
run:
	$(TARGET)
