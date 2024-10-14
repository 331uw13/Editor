#!/bin/bash



ls -lh ./src/

files=$(find ./src/*.c)
echo $files


compiler_flag="-ggdb"

if [[ $1 == "o" || $2 == "o" ]]; then
    compiler_flag="-O3"
fi


if gcc $files $compiler_flag -I/usr/include/freetype2 -I/usr/include/libpng16 -Wall \
    -lglfw -lGL -lGLEW -lz -lfreetype -lm \
    -o editor; then
 
    echo -en "\033[32m"
    ls -lh editor
    echo -en "\033[0m"

    if [[ $1 == "r" || $2 == "r" ]]; then
        ./editor
    fi

fi
