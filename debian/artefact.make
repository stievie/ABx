.PHONY: clean all

all:
	7z a -tzip Bin.zip ../Bin/abdata ../Bin/abmsgs ../Bin/ablogin ../Bin/abserv ../Bin/abfile ../Bin/ablb ../Bin/absadmin ../Bin/keygen ../Bin/00updatedata.sh ../Bin/00updateadmin.sh ../Bin/00updatefile.sh
	yes | cp -f Bin.zip /mnt/hdd02/abx
	7z a -tzip systemd.zip ../systemd/*.service
	yes | cp -f systemd.zip /mnt/hdd02/abx

clean:
	rm -f Bin.zip
	rm -f systemd.zip
