.PHONY: clean all

all:
	@"$(MAKE)" -f  "../makefiles/tinyexpr.make"
	@"$(MAKE)" -f  "../makefiles/abcrypto.make"
	@"$(MAKE)" -f  "../makefiles/libabclient.make"
	@"$(MAKE)" -f  "../makefiles/abclient.make"
clean:
	@"$(MAKE)" -f  "../makefiles/tinyexpr.make" clean
	@"$(MAKE)" -f  "../makefiles/abcrypto.make" clean
	@"$(MAKE)" -f  "../makefiles/libabclient.make" clean
	@"$(MAKE)" -f  "../makefiles/abclient.make" clean

