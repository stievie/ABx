.PHONY: clean all

all:
	@echo "------------------- Building abcrypto -------------------"
	@$(MAKE) -f  abcrypto.make
	@echo "-------------------- Building pugixml -------------------"
	@$(MAKE) -f  pugixml.make
	@echo "-------------------- Building recast --------------------"
	@$(MAKE) -f  recast.make
	@echo "-------------------- Building detour --------------------"
	@$(MAKE) -f  detour.make
	@echo "-------------------- Building libless -------------------"
	@$(MAKE) -f  libless.make
	@echo "--------------------- Building abdb ---------------------"
	@$(MAKE) -f  abdb.make
	@echo "------------------- Building abscommon ------------------"
	@$(MAKE) -f  abscommon.make
	@echo "-------------------- Building absmath -------------------"
	@$(MAKE) -f  absmath.make
	@echo "-------------------- Building abdata --------------------"
	@$(MAKE) -f  abdata.make
	@echo "-------------------- Building abfile --------------------"
	@$(MAKE) -f  abfile.make
	@echo "--------------------- Building ablb ---------------------"
	@$(MAKE) -f  ablb.make
	@echo "-------------------- Building ablogin -------------------"
	@$(MAKE) -f  ablogin.make
	@echo "-------------------- Building abmsgs --------------------"
	@$(MAKE) -f  abmsgs.make
	@echo "------------------- Building absadmin -------------------"
	@$(MAKE) -f  absadmin.make
	@echo "-------------------- Building abserv --------------------"
	@$(MAKE) -f  abserv.make
	@echo "-------------------- Building keygen --------------------"
	@$(MAKE) -f  keygen.make
	@echo "------------------- Building abstests -------------------"
	@$(MAKE) -f  abstests.make
	@echo "--------------------- Running Tests ---------------------"
	@../Bin/Tests
	@echo "------------------- Making artifact ---------------------"
	@$(MAKE) -f  artifact.make

clean:
	@$(MAKE) -f  abcrypto.make clean
	@$(MAKE) -f  pugixml.make clean
	@$(MAKE) -f  recast.make clean
	@$(MAKE) -f  detour.make clean
	@$(MAKE) -f  libless.make clean
	@$(MAKE) -f  abdb.make clean
	@$(MAKE) -f  abscommon.make clean
	@$(MAKE) -f  absmath.make clean
	@$(MAKE) -f  abdata.make clean
	@$(MAKE) -f  abfile.make clean
	@$(MAKE) -f  ablb.make clean
	@$(MAKE) -f  ablogin.make clean
	@$(MAKE) -f  abmsgs.make clean
	@$(MAKE) -f  absadmin.make clean
	@$(MAKE) -f  abserv.make clean
	@$(MAKE) -f  keygen.make clean
	@$(MAKE) -f  abstests.make clean
	@$(MAKE) -f  artifact.make
