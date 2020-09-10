# fhash

Program to calculate hashes for asset files. These hashes are used the the 
client updater.

Place client data files (*.pak) in the file servers `root_dir`  (by default
`bin/file_root`) and run this program in this directory to index the files.

The client updater patches these files if they are differtent.
