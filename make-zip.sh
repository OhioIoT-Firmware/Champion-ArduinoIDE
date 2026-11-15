#!/bin/bash


cd "$(dirname "$0")"

name=$(basename "$PWD")

rm -f "$name.zip"


# zip the code you can edit so you can share it
# skip git, build folders, and credential files
# credentials.template is not a credential file

exclude=(
    "*.git*"
    "*/.git/*"
    ".pio/*"
    "*/.pio/*"
    "build/*"
    "*/build/*"
    "*credentials.h"
    "*secrets*"
)

if [ -f platformio.ini ]; then
    zip -r "$name.zip" src/main.cpp src/controller modules -x "${exclude[@]}"
else
    zip -r "$name.zip" examples/Basic src -x "${exclude[@]}"
fi
