# Drizzle

A one-knob vocal chain effect for the **Akai MPC** (MPC OS 3.x standalone, tested on an MPC Live II). Pick a preset,
turn **DRIZZLE** up, and the voice gets EQ, grit, chorus, echo, reverb and a moving air shelf at once. Every stage is
also open for editing.

## Install

Download the zip from [Releases](../../releases), unpack it on the MPC (or a computer with the MPC's drive) and run
`install.sh`; it stops and restarts MPC. Then add **Drizzle** to a track as an effect.

## Controls

**MAIN** page:

| Control | What it does |
|---|---|
| Preset `< >` | 24 factory presets. Also in MPC's PRESET menu. |
| TONE / GRIT / SWIRL / SPACE | How much of each section you hear (dry/wet). |
| Style `< >` under each | Loads that section's inner settings from any preset, keeping its amount. |
| DRIZZLE | One blend of the dry voice with the whole chain. At 0 the plugin is bypassed. |
| LEVEL | The processed voice's level (-12 to +12 dB) before the DRIZZLE blend. Each preset sets it so it plays about as loud as the dry voice. |
| ECHO: TIME, FEEDBACK, MIX | 0 to 1000 ms, -100 to +100 % feedback (negative flips each repeat). |
| AIR: RATE, AIR, MIX | A high shelf at AIR's frequency. RATE (0 to 20 Hz) sweeps its boost from a cut to about +14 dB; at 0 it holds about +9 dB. |

The other pages open each section:

- **TONE**: low cut, low shelf, three bells, high shelf, high cut (12 dB/oct cuts, cookbook bells and shelves).
- **GRIT**: drive and trim into a clipper, separate push and output gains for the positive and negative halves
  (asymmetry gives even harmonics), and a resonant low-pass after it.
- **SWIRL**: one modulated voice per side, three summed LFOs (sine, saw, triangle), width, and a low or high cut on
  the wet signal.
- **SPACE**: early reflections and a late field, with pre-delay, size, decay, density, damping, late mix and tone.

Signal order: TONE, GRIT, SWIRL, ECHO, SPACE, AIR, LEVEL, then the DRIZZLE blend.

## Presets

Every preset sets all four sections, so each one also works as a section style. Echo times are musical at 90 BPM.

| Group | Presets |
|---|---|
| Polished vocals | First Pour (the all-rounder), Silk Ribbon (smooth R&B), Front Row (dry, upfront rap), Velvet (lush pop), Honey Glaze (warm soul), Daylight (bright and airy), Whisper (close and intimate) |
| Character | Dusty Wax (lo-fi record), Phone Line (telephone), Cassette (tape warmth and flutter), Megaphone (bullhorn), Hot Sauce (aggressive drive), Gravel (gritty boom bap), Crate Glue (for samples and drum loops) |
| Space | Cathedral (huge hall), Night Drive (moody dotted echoes), Echo Alley (running eighth echoes), Slapback (fifties slap), Halo (ethereal), Deep Freeze (near-endless dark wash) |
| Width and movement | Doubler (a second take), Wide Screen (wide chorus with centred lows), Seasick (deep vibrato effect), Flicker (pulsing top end effect) |

Measured on a sung test phrase peaking at -6 dBFS: every preset plays within about 2 dB of the dry voice, loses under
2 dB when summed to mono (Seasick, a vibrato effect, 2.7 dB) and peaks at -2.2 dBFS at most. The reverb and echo
presets peak about 4 dB above the dry voice, so turn LEVEL down a little on a vocal recorded close to full scale.

## CPU

Measured on an MPC Live II: **PASS**, p99 11.7 % of a block, worst block 27.5 %. See `bench.txt`.

## Build

Drizzle is a port for [sd88me/mpc-vst-plugins](https://github.com/sd88me/mpc-vst-plugins) (MIT), which provides the
VST2 wrapper, skin builder and toolchain. It is not included here, and Drizzle needs no changes to it.

1. Clone mpc-vst-plugins and copy this repo's files (except `windows/`) into `ports/drizzle/`.
2. Generate: `python3 tools/gen_presets.py` (`dsp/DrizzlePresets.h`), `python3 tools/gen_params.py` (`params.json`)
   and `python3 art/make_art.py` (the art and `layout.conf`; needs Pillow, numpy and scipy). Edit the scripts, not
   their outputs. VST indices are the order in `params.json`; only append.
3. Build: `tools/build_port.sh ports/drizzle/vst.json` from the toolkit's root. On Windows with Docker Desktop only,
   use `windows/mpc-build.ps1` (build the helper image first: `docker build -t mpc-builder windows/mpc-builder`).
4. Test: `sh tools/test.sh` from `ports/drizzle` (gcc, ASan/UBSan) and the toolkit's
   `tools/test_port.sh ports/drizzle/vst.json`.
5. Release: `sh tools/release.sh <version>` from `ports/drizzle` (in a python:3 container with the toolkit mounted).

`tools/probe/audition.sh` renders two preset sets on the same material and prints each preset's level, peak, tone
tilt, width, mono-sum loss and tail (needs Docker, gcc:12 and a Python image with numpy and scipy tagged `drizzle-py`).

| Path | What |
|---|---|
| `dsp/DrizzleDsp.h` | the whole sound, C++11, no dependencies |
| `src/drizzle_engine.cpp` | the MPC adapter: parameters, smoothing, presets, styles, saved state, value text |
| `test/engine_test.cpp` | adapter checks, including every preset staying finite and in range |
| `skin_post.py` | after the build: the level-ring knob strips and the skin version |

## Licence

MIT, see [LICENSE](LICENSE). Third-party parts: [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).
