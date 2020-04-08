include makefile.common

# This may change
INCLUDES += -I../abscommon
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/absadmin$(SUFFIX)
SOURDEDIR = ../absadmin/absadmin
OBJDIR = obj/x64/$(CONFIG)/absadmin
LIBS += -lpthread -labscommon -lssl -lcrypto -labcrypto -lstdc++fs -lpugixml -lless -luuid -llua5.3 -ldl -ldeathhandler
CXXFLAGS += -fexceptions -Werror -Wimplicit-fallthrough=0
PCH = $(SOURDEDIR)/stdafx.h
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

