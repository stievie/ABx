#!/bin/sh

read -p "Are you sure? " -n 1 -r
echo    # (optional) move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
	rsync -av --delete /mnt/hdd02/abx/admin ./
fi

