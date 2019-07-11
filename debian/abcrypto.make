include makefile.common

# This may change
TARGETDIR = ../Lib/x64/Release
TARGET = $(TARGETDIR)/libabcrypto.a
SOURDEDIR = ../ThirdParty/abcrypto
OBJDIR = obj/x64/Release/abcrypto
SRC_FILES = \
	$(SOURDEDIR)/aes.c \
	$(SOURDEDIR)/arc4random.c \
	$(SOURDEDIR)/bcrypt.c \
	$(SOURDEDIR)/blowfish.c \
	$(SOURDEDIR)/dh.c \
	$(SOURDEDIR)/md5.c \
	$(SOURDEDIR)/safebfuns.c \
	$(SOURDEDIR)/sha1.c \
	$(SOURDEDIR)/sha256.c \
	$(SOURDEDIR)/xxtea.c
# I don't want to mess around with crypto code, just silence the warnings
CFLAGS += -Werror -Wno-pointer-sign -Wno-comment -Wno-missing-braces -Wno-missing-field-initializers -Wformat-truncation=0
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
