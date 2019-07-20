#!/bin/sh

read -p "Overwite local data (Y/[N])? " choice
choice=${choice,,}
if [[ $choice =~ ^(yes|y| ) ]];
then
	rsync -av --delete /mnt/hdd02/abx/data ./
fi

