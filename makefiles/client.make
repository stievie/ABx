.PHONY: clean all

all:
	@"$(MAKE)" -f  "../makefiles/libabclient.make"
	@"$(MAKE)" -f  "../makefiles/abclient.make"
clean:
	@"$(MAKE)" -f  "../makefiles/libabclient.make" clean
	@"$(MAKE)" -f  "../makefiles/abclient.make" clean
