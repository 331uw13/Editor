#!/bin/bash



ls -lh ./src/

files=$(find ./src/*.c)
echo $files



if gcc $files -ggdb -Wall -lglfw -lGL -lm -lz -o editor; then
 
    echo -en "\033[32m"
    ls -lh editor
    echo -en "\033[0m"

    if [[ $1 == "r" ]]; then
        ./editor
    fi

fi
