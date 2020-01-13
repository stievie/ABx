include makefile.common

# This may change
TARGETDIR = ../Lib/x64/$(CONFIG)
TARGET = $(TARGETDIR)/libtinyexpr.a
SOURDEDIR = ../ThirdParty/tinyexpr
OBJDIR = obj/x64/$(CONFIG)/tinyexpr
SRC_FILES = \
	$(SOURDEDIR)/tinyexpr.c
CFLAGS += -Werror 
# End changes

CFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.c, $(OBJDIR)/%.o, $(SRC_FILES))
#$(info $(OBJ_FILES))

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@$(MKDIR_P) $(@D)
	$(LINKCMD_LIB) $(OBJ_FILES)

$(OBJDIR)/%.o: $(SOURDEDIR)/%.c
	@$(MKDIR_P) $(@D)
	$(CCACHE) $(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(TARGET)
