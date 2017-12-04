#!/bin/bash

path=$(dirname $0)
path=${path/\./$(pwd)}
cmake_file_path=${path}/
output_path=${path}/cmake/
mkdir -p ${output_path}
cd ${output_path}
cmake -G Xcode -DCMAKE_TOOLCHAIN_FILE=${cmake_file_path}/iOS.cmake ${cmake_file_path}
cd ${path}

