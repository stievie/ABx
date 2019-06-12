# Requires Codelite makefiles

.PHONY: clean All

All:
	@echo "----------Building project:[ abdb - Release ]----------"
	@cd "../abdb" && "$(MAKE)" -f  "abdb.mk"
	@cd "../.."
	@echo "----------Building project:[ abscommon - Release ]----------"
	@cd "../abscommon" && "$(MAKE)" -f  "abscommon.mk"
	@cd "../.."
	@echo "----------Building project:[ absmath - Release ]----------"
	@cd "../absmath" && "$(MAKE)" -f  "absmath.mk"
	@cd "../.."
	@echo "----------Building project:[ abdata - Release ]----------"
	@cd "../abdata" && "$(MAKE)" -f  "abdata.mk"
	@cd "../.."
	@echo "----------Building project:[ abmsgs - Release ]----------"
	@cd "../abmsgs" && "$(MAKE)" -f  "abmsgs.mk"
	@cd "../.."
	@echo "----------Building project:[ abfile - Release ]----------"
	@cd "../abfile" && "$(MAKE)" -f  "abfile.mk"
	@cd "../.."
	@echo "----------Building project:[ ablb - Release ]----------"
	@cd "../ablb" && "$(MAKE)" -f  "ablb.mk"
	@cd "../.."
	@echo "----------Building project:[ ablogin - Release ]----------"
	@cd "../ablogin" && "$(MAKE)" -f  "ablogin.mk"
	@cd "../.."
	@echo "----------Building project:[ abserv - Release ]----------"
	@cd "../abserv" && "$(MAKE)" -f  "abserv.mk"
	@cd "../.."
	@echo "----------Building project:[ absadmin - Release ]----------"
	@cd "../absadmin" && "$(MAKE)" -f  "absadmin.mk"
	@cd "../.."
clean:
	@echo "----------Cleaning project:[ abdb - Release ]----------"
	@cd "../abdb" && "$(MAKE)" -f  "abdb.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ abscommon - Release ]----------"
	@cd "../abscommon" && "$(MAKE)" -f  "abscommon.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ absmath - Release ]----------"
	@cd "../absmath" && "$(MAKE)" -f  "absmath.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ abdata - Release ]----------"
	@cd "../abdata" && "$(MAKE)" -f  "abdata.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ abmsgs - Release ]----------"
	@cd "../abmsgs" && "$(MAKE)" -f  "abmsgs.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ abfile - Release ]----------"
	@cd "../abfile" && "$(MAKE)" -f  "abfile.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ ablb - Release ]----------"
	@cd "../ablb" && "$(MAKE)" -f  "ablb.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ ablogin - Release ]----------"
	@cd "../ablogin" && "$(MAKE)" -f  "ablogin.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ abserv - Release ]----------"
	@cd "../abserv" && "$(MAKE)" -f  "abserv.mk" clean
	@cd "../.."
	@echo "----------Cleaning project:[ absadmin - Release ]----------"
	@cd "../absadmin" && "$(MAKE)" -f  "absadmin.mk" clean
	@cd "../.."
