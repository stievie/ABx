include makefile.common

# This may change
DEFINES += -D_CONSOLE
INCLUDES += -I../abscommon/abscommon -I../abdb/abdb
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/abdata
SOURDEDIR = ../abdata/abdata
OBJDIR = obj/x64/Release/abdata
LIBS += -lpthread -luuid -llua5.3 -labcrypto -lsqlite3 -ldl -lpq -lldap -lssl -lcrypto -lmariadbclient -lz -lgssapi_krb5 -labscommon -labdb
CXXFLAGS += -fexceptions
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
