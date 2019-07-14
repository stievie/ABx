include makefile.common

# This may change
DEFINES += -D_CONSOLE
INCLUDES += -I../abscommon/abscommon -I../abdb/abdb
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/abdata
SOURDEDIR = ../abdata/abdata
OBJDIR = obj/x64/Release/abdata
LIBS += -lpthread -luuid -llua5.3 -labcrypto -labscommon -labdb
CXXFLAGS += -fexceptions
PCH = $(SOURDEDIR)/stdafx.h
CXXFLAGS += -Werror
DEFINES += -DUSE_PGSQL
# Database Libs
# LIBS += -lsqlite3
LIBS += -ldl -lpq -lldap -lssl -lcrypto -lz -lgssapi_krb5
# LIBS +=  -lmariadbclient
# End changes

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
	rm -f $(GCH) $(OBJ_FILES) $(TARGET)
