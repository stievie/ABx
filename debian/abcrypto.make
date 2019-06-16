include common.make

# This may change
TARGETDIR = ../Lib/x64/Release
TARGET = $(TARGETDIR)/libabcrypto.a
SOURDEDIR = ../ThirdParty/abcrypto
OBJDIR = obj/x64/Release/abcrypto
# End changes

SRC_FILES = $(wildcard $(SOURDEDIR)/*.c)

CXXFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.c, $(OBJDIR)/%.o, $(SRC_FILES))
#$(info $(OBJ_FILES))

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@$(MKDIR_P) $(@D)
	$(LINKCMD_LIB) $(OBJ_FILES)

$(OBJDIR)/%.o: $(SOURDEDIR)/%.c
	@$(MKDIR_P) $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(TARGET)
