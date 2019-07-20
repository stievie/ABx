#!/bin/sh

read -p "Are you sure (Y/[N])? " -r
if [[ $REPLY =~ ^[Yy]$ ]]
then
	rsync -av --delete /mnt/hdd02/abx/admin ./
fi

