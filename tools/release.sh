#!/bin/sh
# Builds Drizzle's release zip from the last device build:  sh tools/release.sh <version>   (from ports/drizzle, in a python:3 container)
# Needs bench.txt: the bench's -j output for this build (docs/BENCH.md).
set -e
V="$1"
[ -n "$V" ] || { echo "usage: sh tools/release.sh <version>"; exit 2; }
python3 ../../tools/release.py --so build/drizzle.so --skin "build/skin/Milliano - VST - Drizzle" \
    --entry build/pluginlist-entry.xml --version "$V" \
    --bench bench.txt \
    --extra LICENSE:LICENSE.txt \
    --repo guerrillamilliano/Drizzle --license MIT --id drizzle \
    --about "A one-knob vocal chain: EQ, grit, swirl chorus, echo, space reverb and a moving air shelf, with 24 presets and four section styles." \
    -o dist
python3 ../../tools/catalog_check.py "dist/Drizzle-$V-mpc-armv7.zip" --expect-id drizzle --expect-repo guerrillamilliano/Drizzle
