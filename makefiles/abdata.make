include makefile.common

# This may change
INCLUDES += -I../abscommon -I../abdb
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/abdata$(SUFFIX)
SOURDEDIR = ../abdata/abdata
OBJDIR = obj/x64/$(CONFIG)/abdata
LIBS += -lpthread -labcrypto -labscommon -labdb -luuid -lstdc++fs -llua5.3 -ldl -ldeathhandler
CXXFLAGS += -fexceptions -Werror
PCH = $(SOURDEDIR)/stdafx.h
DEFINES += -DUSE_PGSQL
# Database Libs
ifneq (,$(findstring USE_PGSQL,$(DEFINES)))
LIBS += -lpq -lldap -lssl -lcrypto -lz -lgssapi_krb5
endif
ifneq (,$(findstring USE_SQLITE,$(DEFINES)))
LIBS += -lsqlite3
endif
ifneq (,$(findstring USE_MYSQL,$(DEFINES)))
LIBS +=  -lmariadbclient
endif
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
	$(PRE_CXX) $(CXX) $(CXXFLAGS) -MMD -c $< -o $@

# PCH
$(GCH): $(PCH)
	$(CXX) -x c++-header $(CXXFLAGS) -c $< -o $@

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(GCH) $(OBJ_FILES) $(TARGET) $(OBJDIR)/*.d
