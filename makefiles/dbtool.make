include makefile.common

# This may change
INCLUDES += -I../abscommon/abscommon -I../abdb/abdb
TARGETDIR = ../Bin
TARGET = $(TARGETDIR)/dbtool$(SUFFIX)
SOURDEDIR = ../dbtool/dbtool
OBJDIR = obj/x64/$(CONFIG)/dbtool
LIBS += -lpthread -llua5.3 -labscommon -labdb
CXXFLAGS += -Werror
DEFINES += -DUSE_PGSQL
# Database Libs
ifneq (,$(findstring USE_PGSQL,$(DEFINES)))
LIBS += -ldl -lpq -lldap -lssl -lcrypto -lz -lgssapi_krb5
endif
ifneq (,$(findstring USE_SQLITE,$(DEFINES)))
LIBS += -lsqlite3
endif
ifneq (,$(findstring USE_MYSQL,$(DEFINES)))
LIBS +=  -lmariadbclient
endif
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
	$(CCACHE) $(CXX) $(CXXFLAGS) -MMD -c $< -o $@

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(TARGET)
