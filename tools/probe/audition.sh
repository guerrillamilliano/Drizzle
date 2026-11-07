#!/bin/sh
# Renders and measures two preset sets on the same material:  sh audition.sh [old.h] [new.h]   (from Git Bash)
# Defaults: work/audition/old_presets.h (any earlier DrizzlePresets.h you keep there) and ../../dsp/DrizzlePresets.h.
cd "$(dirname "$0")"
OLD="${1:-work/audition/old_presets.h}"
NEW="${2:-../../dsp/DrizzlePresets.h}"
W=work/audition
DEMO="$(cd ../../../chrono/build/demo 2>/dev/null && pwd -W)/03 vocal root.wav"
MSYS_NO_PATHCONV=1 docker run --rm -v "$(pwd -W):/p" -v "$DEMO:/demo.wav:ro" -w /p drizzle-py python3 audition.py make $W >/dev/null
for s in "$OLD" "$NEW"; do grep -o '{"[^"]*"' "$s" | tr -d '{"' > "$W/$(basename "$s" .h).names"; done
MSYS_NO_PATHCONV=1 docker run --rm -v "$(cd ../.. && pwd -W):/port" -w /port/tools/probe gcc:12 sh -c "
g++ -O2 -std=gnu++11 -I../../dsp -include '$OLD' audition.cpp -o $W/aud_old &&
g++ -O2 -std=gnu++11 -DHAS_LEVEL -I../../dsp -include '$NEW' audition.cpp -o $W/aud_new || exit 1
for sig in sung demo; do $W/aud_old $W/\$sig.f32 $W \${sig}_o >/dev/null; $W/aud_new $W/\$sig.f32 $W \${sig}_n >/dev/null; done"
for sig in sung demo; do
    echo "=== $sig: before"
    MSYS_NO_PATHCONV=1 docker run --rm -v "$(pwd -W):/p" -w /p drizzle-py python3 audition.py show $W $sig o "$W/$(basename "$OLD" .h).names"
    echo "=== $sig: new"
    MSYS_NO_PATHCONV=1 docker run --rm -v "$(pwd -W):/p" -w /p drizzle-py python3 audition.py show $W $sig n "$W/$(basename "$NEW" .h).names"
done
