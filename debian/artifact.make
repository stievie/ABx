.PHONY: clean all

all:
	7z a -tzip Bin.zip ../Bin/abdata ../Bin/abmsgs ../Bin/ablogin ../Bin/abserv ../Bin/abfile ../Bin/ablb ../Bin/absadmin ../Bin/keygen
	yes | cp -f Bin.zip /mnt/hdd02/abx

clean:
	rm -f Bin.zip
