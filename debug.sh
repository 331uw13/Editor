#!/bin/bash


./build.sh

gdb -x gdb_commands.txt --args editor


