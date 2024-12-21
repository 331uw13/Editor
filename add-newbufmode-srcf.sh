#!/bin/bash


function printusage () {
    echo "Usage: $0 <name for new bufmode>"
    echo "NOTE: the input should be in lowercase!"
    exit
}

if [ $# -ne 1 ]; then
    printusage;
fi

srcdir="./src/bufmodes"

name=$1
nameupper=${name^^}

tmp_headerfile="/tmp/$name.h_$(xxd -l16 -ps /dev/urandom)"
tmp_sourcefile="/tmp/$name.c_$(xxd -l16 -ps /dev/urandom)"

if [ -z ${name} ]; then
    printusage;
fi

echo -en "\033[90m---------------------------\033[0m\n"

echo -e \
"#ifndef BUFFER_MODE_${nameupper}_H\n"\
"#define BUFFER_MODE_${nameupper}_H\n"\
"\n"\
"struct editor_t;\n"\
"struct buffer_t;\n"\
"\n"\
"\n"\
"void bufmode_${name}_keypress(\n"\
"       struct editor_t* ed,\n"\
"       struct buffer_t* buf,\n"\
"       int key,\n"\
"       int mods\n"\
"       );\n"\
"\n"\
"void bufmode_${name}_charinput(\n"\
"       struct editor_t* ed,\n"\
"       struct buffer_t* buf,\n"\
"       unsigned char codepoint\n"\
"       );\n"\
"\n"\
"#endif\n" | tee $tmp_headerfile

echo -en "\033[90m---------------------------\033[0m\n"

echo -e "#include \"../editor.h\"\n"\
"#include \"${name}.h\"\n"\
"\n"\
"void bufmode_${name}_keypress(\n"\
"       struct editor_t* ed,\n"\
"       struct buffer_t* buf,\n"\
"       int key,\n"\
"       int mods\n"\
"       )\n"\
"{\n"\
"}\n"\
"\n"\
"void bufmode_${name}_charinput(\n"\
"       struct editor_t* ed,\n"\
"       struct buffer_t* buf,\n"\
"       unsigned char codepoint\n"\
"       )\n"\
"{\n"\
"}\n" | tee $tmp_sourcefile

echo -en "\033[90m---------------------------\033[0m\n"


function cleanup () {
    rm -v $tmp_headerfile
    rm -v $tmp_sourcefile
}

echo -en "  Files \033[34m${srcdir}/${name}.h\033[0m And \033[35m${srcdir}/${name}.c\033[0m Are going to be created.\n"
echo -en "  Press [y] To accept changes: "
while read -r -n1 line ; do
    echo ""
    if [[ ${line^^} == "Y" ]] ; then
        mv -v $tmp_headerfile $srcdir/$name.h
        mv -v $tmp_sourcefile $srcdir/$name.c
        echo "done."
    fi
    cleanup;
    exit
done


