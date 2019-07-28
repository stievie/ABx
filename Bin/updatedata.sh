#!/bin/sh

read -p "Overwite local data (Y/[N])? " choice
if [ "$choice" != "y" ]; then
  exit 0
fi

rsync -av --delete /mnt/hdd02/abx/data ./
