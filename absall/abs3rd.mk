# Requires Codelite makefiles

.PHONY: clean All

All:
	@echo "----------Building project:[ abcrypto - Release ]----------"
	@cd "../ThirdParty/abcrypto" && "$(MAKE)" -f  "abcrypto.mk"
	@cd "../.."
	@echo "----------Building project:[ libless - Release ]----------"
	@cd "../ThirdParty/libless" && "$(MAKE)" -f  "libless.mk"
	@cd "../.."
	@echo "----------Building project:[ lz4 - Release ]----------"
	@cd "../ThirdParty/lz4" && "$(MAKE)" -f  "lz4.mk"
	@cd "../.."
	@echo "----------Building project:[ PugiXml - Release ]----------"
	@cd "../ThirdParty/PugiXml" && "$(MAKE)" -f  "pugixml.mk"
	@cd "../.."
	@echo "----------Building project:[ DebugUtils - Release ]----------"
	@cd "../ThirdParty/recastnavigation" && "$(MAKE)" -f  "DebugUtils.mk"
	@cd "../.."
	@echo "----------Building project:[ Detour - Release ]----------"
	@cd "../ThirdParty/recastnavigation" && "$(MAKE)" -f  "Detour.mk"
	@cd "../.."
	@echo "----------Building project:[ Recast - Release ]----------"
	@cd "../ThirdParty/recastnavigation" && "$(MAKE)" -f  "Recast.mk"
	@cd "../.."
	@echo "----------Building project:[ sqlite3 - Release ]----------"
	@cd "../ThirdParty/sqlite3" && "$(MAKE)" -f  "sqlite3.mk"
	@cd "../.."
clean:
	@echo "----------Cleaning project:[ abcrypto - Release ]----------"
	@cd "../ThirdParty/abcrypto" && "$(MAKE)" -f  "abcrypto.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ libless - Release ]----------"
	@cd "../ThirdParty/libless" && "$(MAKE)" -f  "libless.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ lz4 - Release ]----------"
	@cd "../ThirdParty/lz4" && "$(MAKE)" -f  "lz4.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ PugiXml - Release ]----------"
	@cd "../ThirdParty/PugiXml" && "$(MAKE)" -f  "pugixml.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ DebugUtils - Release ]----------"
	@cd "../ThirdParty/recastnavigation" && "$(MAKE)" -f  "DebugUtils.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ Detour - Release ]----------"
	@cd "../ThirdParty/recastnavigation" && "$(MAKE)" -f  "Detour.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ Recast - Release ]----------"
	@cd "../ThirdParty/recastnavigation" && "$(MAKE)" -f  "Recast.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ sqlite3 - Release ]----------"
	@cd "../ThirdParty/sqlite3" && "$(MAKE)" -f  "sqlite3.mk" clean
	@cd "../.."
