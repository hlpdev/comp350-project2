#!/bin/bash

set -e

if [[ -f build/osh ]]; then
  rm build/osh
fi

mkdir -p build

gcc src/*.c -Iinclude -o build/osh
