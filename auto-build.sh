#!/bin/bash
PROJECT_DIR=$(dirname $0)
BUILD_DIR="${PROJECT_DIR}/build"

cd "$PROJECT_DIR"
git ls-files -z | while IFS= read -rd '' f; do tail -c1 < "$f" | read -r _ || echo >> "$f"; done

cd "$BUILD_DIR"

# shopt -s extglob
# rm -r -v !(auto-build.sh) > /dev/null 2>&1

cmake ../src/
make
