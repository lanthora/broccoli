#!/bin/bash
PROJECT_DIR=`cd $(dirname $0);pwd`
BUILD_DIR="${PROJECT_DIR}/build"
SRC_DIR="${PROJECT_DIR}/src"

cd "$SRC_DIR"
find . -type f | while IFS= read -rd '' f; do tail -c1 < "$f" | read -r _ || echo >> "$f"; done


mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake ../src/
make
