#!/usr/bin/env python3
"""Writes params.json for Drizzle (run from ports/drizzle after changing anything here).

VST indices are the list order and saved projects bind to them, so keys are only ever appended (params.lock checks it).
Keys and units must agree with src/drizzle_engine.cpp (test/engine_test.cpp compares the two); defaults come from the
first factory preset in dsp/DrizzlePresets.h, which is also the sound a new instance starts with."""
import json
import math
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
HEADER = open(os.path.join(HERE, "..", "dsp", "DrizzlePresets.h")).read()
FIELDS = [f.strip() for f in re.search(r"float ([^;]+);", HEADER).group(1).split(",")]
ROWS = re.findall(r'^\s*\{"([^"]+)", ([^}]+)\},', HEADER, re.M)
NAMES = [n for n, _ in ROWS]
FIRST = dict(zip(FIELDS, [float(v.rstrip("f")) for v in ROWS[0][1].split(", ")]))


def pos(hz):
    """A frequency as its knob position (log, 20 Hz .. 20 kHz over 0..100), as hzToPos() in the engine."""
    return round(min(100.0, max(0.0, 100.0 * math.log10(min(20000.0, max(20.0, hz)) / 20.0) / 3.0)), 3)


def cont(key, name, lo, hi, default, unit=""):
    return {"key": key, "name": name, "min": lo, "max": hi, "default": round(default, 4), "unit": unit,
            "dynamic_display": True}


def opts(key, name, options, default):
    return {"key": key, "name": name, "options": options, "default": default}


F = FIRST
SHAPES = ["SINE", "SAW", "TRI"]
params = [
    cont("drizzle", "Drizzle", 0, 100, 100 * F["overall"], "%"),
    cont("tone", "Tone", 0, 100, 100 * F["eqMix"], "%"),
    cont("grit", "Grit", 0, 100, 100 * F["satMix"], "%"),
    cont("swirl", "Swirl", 0, 100, 100 * F["chMix"], "%"),
    cont("space", "Space", 0, 100, 100 * F["vbMix"], "%"),
    cont("dly_time", "Delay Time", 0, 1000, F["dlTimeMs"], "ms"),
    cont("dly_fb", "Feedback", -100, 100, 100 * F["dlFeedback"], "%"),
    cont("dly_mix", "Delay Mix", 0, 100, 100 * F["dlMix"], "%"),
    cont("air_rate", "Air Rate", 0, 20, F["airRate"], "Hz"),
    cont("air_freq", "Air", 0, 100, pos(F["airFreq"])),
    cont("air_mix", "Air Mix", 0, 100, 100 * F["airMix"], "%"),
    opts("preset", "Preset", NAMES, 0),
    opts("tone_style", "Tone Style", NAMES, 0),
    opts("grit_style", "Grit Style", NAMES, 0),
    opts("swirl_style", "Swirl Style", NAMES, 0),
    opts("space_style", "Space Style", NAMES, 0),
    cont("eq_hpf", "Low Cut", 0, 100, pos(F["hpf"])),
    cont("eq_ls_f", "Low Shelf Freq", 0, 100, pos(F["lsFreq"])),
    cont("eq_ls_q", "Low Shelf Q", 0.1, 5, F["lsQ"]),
    cont("eq_ls_g", "Low Shelf Gain", -24, 24, F["lsGain"], "dB"),
]
for k in range(3):
    params += [
        cont("eq_b%d_f" % (k + 1), "Bell %d Freq" % (k + 1), 0, 100, pos(F["pkFreq%d" % k])),
        cont("eq_b%d_q" % (k + 1), "Bell %d Q" % (k + 1), 0.1, 5, F["pkQ%d" % k]),
        cont("eq_b%d_g" % (k + 1), "Bell %d Gain" % (k + 1), -24, 24, F["pkGain%d" % k], "dB"),
    ]
params += [
    cont("eq_hs_f", "High Shelf Freq", 0, 100, pos(F["hsFreq"])),
    cont("eq_hs_q", "High Shelf Q", 0.1, 5, F["hsQ"]),
    cont("eq_hs_g", "High Shelf Gain", -24, 24, F["hsGain"], "dB"),
    cont("eq_lpf", "High Cut", 0, 100, pos(F["lpf"])),
    cont("gr_drive", "Drive", -40, 40, F["satDrive"], "dB"),
    cont("gr_pos_in", "Push +", -20, 60, F["posIn"], "dB"),
    cont("gr_neg_in", "Push -", -20, 60, F["negIn"], "dB"),
    cont("gr_pos_out", "Out +", -20, 60, F["posOut"], "dB"),
    cont("gr_neg_out", "Out -", -20, 60, F["negOut"], "dB"),
    cont("gr_trim", "Trim", -20, 60, F["satTrim"], "dB"),
    cont("gr_cut", "Grit Cut", 0, 100, pos(F["satCut"])),
    cont("sw_delay", "Swirl Delay", 0, 30, F["chDelayMs"], "ms"),
    cont("sw_depth", "Swirl Depth", 0, 100, 100 * F["chDepth"], "%"),
    cont("sw_width", "Swirl Width", 0, 180, F["chStereoDeg"]),
    cont("sw_rate1", "LFO 1 Rate", 0, 20, F["chRate0"], "Hz"),
    cont("sw_rate2", "LFO 2 Rate", 0, 20, F["chRate1"], "Hz"),
    cont("sw_rate3", "LFO 3 Rate", 0, 20, F["chRate2"], "Hz"),
    opts("sw_shape1", "LFO 1 Shape", SHAPES, int(min(2, max(0, F["chShape0"])))),
    opts("sw_shape2", "LFO 2 Shape", SHAPES, int(min(2, max(0, F["chShape1"])))),
    opts("sw_shape3", "LFO 3 Shape", SHAPES, int(min(2, max(0, F["chShape2"])))),
    opts("sw_filter", "Swirl Filter", ["OFF", "LOW CUT", "HIGH CUT"], int(min(2, max(0, F["chFilter"])))),
    cont("sw_freq", "Swirl Filter Freq", 0, 100, pos(F["chFreq"])),
    cont("sp_pre", "Pre-Delay", 0, 100, F["vbPreMs"], "ms"),
    cont("sp_size", "Size", 0, 100, 100 * F["vbSize"], "%"),
    cont("sp_decay", "Decay", 0, 100, 100 * F["vbDecay"], "%"),
    cont("sp_density", "Density", 0, 100, 100 * F["vbDensity"], "%"),
    cont("sp_damp", "Damping", 0, 100, 100 * F["vbDamp"], "%"),
    cont("sp_late", "Late Mix", 0, 100, 100 * F["vbEarly"], "%"),
    cont("sp_tone", "Space Tone", 0, 100, 100 * F["vbTone"], "%"),
]
# 58 on: the < > arrows beside the preset and style names (the wrapper steps the named param itself)
for key, name in (("preset", "Preset"), ("tone_style", "Tone Style"), ("grit_style", "Grit Style"),
                  ("swirl_style", "Swirl Style"), ("space_style", "Space Style")):
    params += [{"key": key + "_prev", "name": "Previous " + name, "momentary": True, "step_of": key, "step_delta": -1},
               {"key": key + "_next", "name": "Next " + name, "momentary": True, "step_of": key, "step_delta": 1}]

# 1.1.0: the processed voice's level before the DRIZZLE blend (each preset sets it so it plays at the dry level)
params.append(cont("level", "Level", -12, 12, F["outDb"], "dB"))

with open(os.path.join(HERE, "..", "params.json"), "w") as f:
    json.dump({"name": "Drizzle", "params": params}, f, indent=1)
print("params.json: %d params" % len(params))
