#!/bin/bash

set -e

mkdir build
gcc src/*.c -Iinclude -o build/osh
