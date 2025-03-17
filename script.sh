#!/bin/bash
echo "Compiling"
g++ mergesort.cpp -o mergesort
if [ $? -eq 0 ]; then
    echo "Compilation successful. Executable file: mergesort"
else
    echo "Compilation failed."
fi