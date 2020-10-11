#!/bin/bash

cd $(dirname $0)

# shopt -s extglob
# rm -r -v !(auto-build.sh) > /dev/null 2>&1

cmake ../src/
make