#!/bin/bash



ls -lh ./src/

files=$(find ./src/*.c)

name="editor"
compiler_flag="-ggdb"

if [[ $1 == "o" || $2 == "o" ]]; then
    compiler_flag="-O3"
fi


if gcc $files $compiler_flag \
    -Wall \
    -lglfw -lGL -lGLEW -lm -lfreetype \
    -I/usr/include/freetype2 \
    -I/usr/include/libpng16 \
    -o $name; then
 
    echo -en "\033[32m"
    ls -lh $name
    echo -en "\033[0m"

    if [[ $1 == "r" || $2 == "r" ]]; then
        ./$name
    fi

fi
