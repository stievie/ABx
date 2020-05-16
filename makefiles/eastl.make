include makefile.common

# This may change
INCLUDES += -I../ThirdParty/EASTL/include
TARGETDIR = ../Lib/x64/$(CONFIG)
TARGET = $(TARGETDIR)/libEASTL.a
SOURDEDIR = ../ThirdParty/EASTL
OBJDIR = obj/x64/$(CONFIG)/EASTL
SRC_FILES = $(wildcard $(SOURDEDIR)/source/*.cpp)
CFLAGS += -Werror
# End changes

CFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC_FILES))
#$(info $(OBJ_FILES))

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@$(MKDIR_P) $(@D)
	$(LINKCMD_LIB) $(OBJ_FILES)

$(OBJDIR)/%.o: $(SOURDEDIR)/%.c
	@$(MKDIR_P) $(@D)
	$(PRE_CXX) $(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(TARGET) $(OBJ_FILES:.o=.d)
