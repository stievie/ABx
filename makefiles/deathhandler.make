include makefile.common

# This may change
TARGETDIR = ../Lib/x64/$(CONFIG)
TARGET = $(TARGETDIR)/libdeathhandler.a
SOURDEDIR = ../ThirdParty/DeathHandler
OBJDIR = obj/x64/$(CONFIG)/DeathHandler
CXXFLAGS += -Werror
# End changes

SRC_FILES = \
	$(SOURDEDIR)/death_handler.cc

CXXFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.cc, $(OBJDIR)/%.o, $(SRC_FILES))

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@$(MKDIR_P) $(@D)
	$(LINKCMD_LIB) $(OBJ_FILES)

$(OBJDIR)/%.o: $(SOURDEDIR)/%.cc
	@$(MKDIR_P) $(@D)
	$(PRE_CXX) $(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(TARGET) $(OBJDIR)/*.d
