#!/bin/bash
mkdir -p build
cd build 
cmake .. -G Xcode
open build/*.xcodeproj