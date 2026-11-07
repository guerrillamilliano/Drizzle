#!/bin/sh
# Offline tests of Drizzle's MPC adapter, from ports/drizzle in a gcc container:
#   docker run --rm -v <repo>:/r -w /r/ports/drizzle gcc:12 sh tools/test.sh
set -e
mkdir -p build
g++ -std=gnu++11 -O1 -g -fsanitize=address,undefined -Wall -Wextra -Wno-unused-parameter -Idsp -I../../wrapper \
    test/engine_test.cpp src/drizzle_engine.cpp -o build/engine_test
build/engine_test
