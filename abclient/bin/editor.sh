#!/bin/sh

# Copy Urho3DPlayer from Urho3D/bin to this directory and also the assets
DRI_PRIME=1 ./Urho3DPlayer Scripts/Editor.as -w -s -p "Data;CoreData;AbData;SoundData"
