include common.make

# This may change
DEFINES += -DNDEBUG
INCLUDES += -I../Include -I../abscommon/abscommon -I/usr/include/postgresql
TARGETDIR = ../Lib/x64/Release
TARGET = $(TARGETDIR)/libabdb.a
SOURDEDIR = ../abdb/abdb
OBJDIR = obj/x64/Release/abdb
SRC_FILES = \
	$(SOURDEDIR)/Database.cpp \
	$(SOURDEDIR)/DatabaseMysql.cpp \
	$(SOURDEDIR)/DatabasePgsql.cpp \
	$(SOURDEDIR)/DatabaseSqlite.cpp \
	$(SOURDEDIR)/stdafx.cpp \
# End changes

CXXFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC_FILES))
$(info $(OBJ_FILES))

$(TARGET): $(OBJ_FILES)
	$(LINKCMD) $(OBJ_FILES)

$(OBJDIR)/%.o: $(SOURDEDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)
$(TARGETDIR):
	mkdir -p $(TARGETDIR)

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(TARGET)
