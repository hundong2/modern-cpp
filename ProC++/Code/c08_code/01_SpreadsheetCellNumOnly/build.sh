#!/bin/bash

# Set the source files
MODULE_INTERFACE="SpreadsheetCell.cppm"
MODULE_IMPLEMENTATION="SpreadsheetCell.cpp"
TEST_FILE="SpreadsheetCellTest.cpp"
OUTPUT_EXECUTABLE="SpreadsheetCellTest"

# Compile the module interface
clang++ -std=c++20 -fmodules-ts -c $MODULE_INTERFACE -o SpreadsheetCell.pcm
if [ $? -ne 0 ]; then
    echo "Failed to compile module interface"
    exit 1
fi

# Compile the module implementation
clang++ -std=c++20 -fmodules-ts -c $MODULE_IMPLEMENTATION -fmodule-file=SpreadsheetCell.pcm -o SpreadsheetCell.o
if [ $? -ne 0 ]; then
    echo "Failed to compile module implementation"
    exit 1
fi

# Compile the test file and link
clang++ -std=c++20 -fmodules-ts $TEST_FILE SpreadsheetCell.o -o $OUTPUT_EXECUTABLE -fmodule-file=SpreadsheetCell.pcm
if [ $? -ne 0 ]; then
    echo "Failed to compile and link test file"
    exit 1
fi

# Run the executable
./$OUTPUT_EXECUTABLE