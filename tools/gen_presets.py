#!/usr/bin/env python3
"""Writes dsp/DrizzlePresets.h: Drizzle's 24 factory settings, designed for Drizzle (no reference values).

Each preset starts from BASE (a clean vocal chain with every section set up sensibly, so any preset also makes a useful
section style) and changes what gives it its character. Echo times are musical at 90 BPM (1/16 167 ms, 1/8 333 ms,
dotted 1/8 500 ms, 1/4 667 ms). Levels are balanced so a preset is about as loud as the dry voice (tools/probe/
audition.sh measures it), which keeps A/B comparisons fair. Run from ports/drizzle:  python3 tools/gen_presets.py"""
import os

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "dsp", "DrizzlePresets.h")

FIELDS = ["overall", "eqMix", "satMix", "chMix", "vbMix", "dlMix", "dlTimeMs", "dlFeedback", "airRate", "airFreq",
          "airMix", "hpf", "lsFreq", "lsQ", "lsGain", "pkFreq0", "pkQ0", "pkGain0", "pkFreq1", "pkQ1", "pkGain1",
          "pkFreq2", "pkQ2", "pkGain2", "hsFreq", "hsQ", "hsGain", "lpf", "satDrive", "posIn", "negIn", "posOut",
          "negOut", "satTrim", "satCut", "chDelayMs", "chDepth", "chStereoDeg", "chRate0", "chRate1", "chRate2",
          "chShape0", "chShape1", "chShape2", "chFilter", "chFreq", "vbPreMs", "vbSize", "vbDecay", "vbDensity",
          "vbDamp", "vbEarly", "vbTone", "outDb"]

# Saturation gain is linear up to the clipper's knee when drive + trim + in + out = -6 dB (the clipper adds 6 dB).
BASE = dict(
    overall=1.0, eqMix=1.0, satMix=0.0, chMix=0.0, vbMix=0.0, dlMix=0.0,
    dlTimeMs=333, dlFeedback=0.25,
    airRate=0.0, airFreq=10000, airMix=0.0,
    hpf=80, lsFreq=200, lsQ=0.7, lsGain=0.0,
    pkFreq0=300, pkQ0=1.2, pkGain0=0.0, pkFreq1=1800, pkQ1=1.0, pkGain1=0.0, pkFreq2=4500, pkQ2=1.0, pkGain2=0.0,
    hsFreq=10000, hsQ=0.7, hsGain=0.0, lpf=20000,
    satDrive=3.0, posIn=0.0, negIn=-2.0, posOut=-9.0, negOut=-7.0, satTrim=0.0, satCut=16000,
    chDelayMs=10, chDepth=0.25, chStereoDeg=0.0, chRate0=0.35, chRate1=0.21, chRate2=0.0,
    chShape0=0, chShape1=2, chShape2=0, chFilter=1, chFreq=220,          # filter 1: high-pass on the wet voices
    vbPreMs=18, vbSize=0.45, vbDecay=0.45, vbDensity=0.75, vbDamp=0.45, vbEarly=0.8, vbTone=0.6,
    outDb=0.0,
)


def grit(drive, neg=2.0, extra=0.0):
    """A saturation stage at unity gain below the knee: `drive` dB in (the positive half), the negative half `neg` dB
    softer (even harmonics), and `extra` dB of make-up on the way out."""
    return dict(satDrive=drive, posIn=0.0, negIn=-neg, posOut=-6.0 - drive + extra, negOut=-6.0 - drive + neg + extra)


PRESETS = [
    # --- polished vocals
    ("First Pour", "The all-rounder: clean lows, a lift at presence and air, light grit, a little echo and plate.",
     dict(hpf=85, pkFreq0=320, pkQ0=1.3, pkGain0=-2.5, pkFreq1=2800, pkQ1=1.0, pkGain1=1.0, pkFreq2=5200, pkQ2=1.2,
          pkGain2=1.0, hsFreq=11000, hsGain=1.5,
          satMix=0.25, **grit(4.0), satCut=15000,
          chMix=0.15, chDelayMs=11, chDepth=0.25,
          dlMix=0.12, dlTimeMs=333, dlFeedback=0.22,
          vbMix=0.18, vbPreMs=20, vbSize=0.4, vbDecay=0.4, vbDamp=0.4, vbEarly=0.8, vbTone=0.65,
          airMix=0.25, airFreq=10000)),
    ("Silk Ribbon", "Smooth R&B: warm body, softened bite, a slow shimmer on top and a soft hall.",
     dict(hpf=90, lsFreq=180, lsGain=0.5, pkFreq0=380, pkGain0=-2.0, pkFreq1=3200, pkQ1=1.6, pkGain1=-1.0,
          pkFreq2=7000, pkGain2=1.0, hsFreq=12000, hsGain=2.0, lpf=19000,
          satMix=0.2, **grit(2.0, 1.0), satCut=12000,
          chMix=0.25, chDelayMs=14, chDepth=0.2, chRate0=0.22, chRate1=0.13, chFreq=300,
          dlMix=0.14, dlTimeMs=500, dlFeedback=0.3,
          vbMix=0.26, vbPreMs=30, vbSize=0.6, vbDecay=0.55, vbDamp=0.55, vbEarly=0.85, vbTone=0.55,
          airMix=0.3, airFreq=12000, airRate=0.12)),
    ("Front Row", "Dry and in your face for rap: mud and honk out, presence up, grit, a slap and a small room.",
     dict(hpf=100, pkFreq0=280, pkQ0=1.4, pkGain0=-3.0, pkFreq1=1100, pkQ1=1.4, pkGain1=-1.0, pkFreq2=3500,
          pkGain2=2.5, hsFreq=9000, hsGain=1.5,
          satMix=0.35, **grit(6.0, 3.0), satCut=14000,
          dlMix=0.1, dlTimeMs=110, dlFeedback=0.05,
          vbMix=0.1, vbPreMs=5, vbSize=0.15, vbDecay=0.3, vbDamp=0.5, vbEarly=0.35, vbTone=0.55,
          airMix=0.2, airFreq=9000)),
    ("Velvet", "Lush pop: a wide three-LFO chorus, quarter-note echo and a big soft hall.",
     dict(hpf=90, pkFreq0=300, pkQ0=1.0, pkGain0=-2.0, pkFreq2=4000, pkQ2=0.9, pkGain2=1.5, hsFreq=12000, hsGain=2.5,
          satMix=0.15, **grit(2.0),
          chMix=0.3, chDelayMs=12, chDepth=0.35, chRate0=0.45, chRate1=0.27, chRate2=0.08, chFreq=150,
          dlMix=0.18, dlTimeMs=667, dlFeedback=0.35,
          vbMix=0.3, vbPreMs=35, vbSize=0.75, vbDecay=0.6, vbDamp=0.5, vbEarly=0.9, vbTone=0.6,
          airMix=0.3, airFreq=11000, airRate=0.08)),
    ("Honey Glaze", "Warm soul: thick lows, rounded top, tape-style grit and a dark plate.",
     dict(hpf=75, lsFreq=160, lsGain=1.0, pkFreq0=400, pkGain0=-1.5, pkFreq1=2500, pkGain1=1.0, pkFreq2=6000,
          pkGain2=-1.0, hsFreq=10000, hsGain=-1.0, lpf=14000,
          satMix=0.45, **grit(5.0, 2.0), satCut=9000,
          chMix=0.18, chDelayMs=7, chDepth=0.2, chRate0=0.6, chRate1=0.0,
          dlMix=0.12, dlTimeMs=333, dlFeedback=0.2,
          vbMix=0.2, vbPreMs=12, vbSize=0.35, vbDecay=0.45, vbDamp=0.6, vbEarly=0.8, vbTone=0.45)),
    ("Daylight", "Bright airy pop: lifted presence and top, a moving air band and a clear room.",
     dict(hpf=110, pkFreq0=250, pkQ0=1.0, pkGain0=-2.0, pkFreq1=900, pkQ1=1.5, pkGain1=-1.0, pkFreq2=5000, pkQ2=0.8,
          pkGain2=2.0, hsFreq=9000, hsGain=3.0,
          satMix=0.15, **grit(2.0),
          chMix=0.2, chDelayMs=10, chDepth=0.25,
          dlMix=0.12, dlTimeMs=500, dlFeedback=0.25,
          vbMix=0.2, vbSize=0.5, vbDecay=0.45, vbDamp=0.3, vbEarly=0.8, vbTone=0.8,
          airMix=0.35, airFreq=12000, airRate=0.25)),
    ("Whisper", "Close and intimate: no boom, soft sibilance, a breath of air and a small plate.",
     dict(hpf=130, pkFreq0=250, pkGain0=-2.0, pkFreq1=2000, pkGain1=0.0, pkFreq2=6500, pkQ2=1.5, pkGain2=-1.5,
          hsFreq=12000, hsGain=2.0,
          satMix=0.15, **grit(2.0),
          chMix=0.12, chDelayMs=9, chDepth=0.2,
          vbMix=0.14, vbPreMs=10, vbSize=0.3, vbDecay=0.4, vbDamp=0.45, vbEarly=0.8, vbTone=0.55,
          airMix=0.3, airFreq=13000)),
    # --- character
    ("Dusty Wax", "Lo-fi record: thin and boxy, crunchy, a warbling chorus and a small dark room.",
     dict(hpf=220, pkFreq1=1300, pkQ1=0.9, pkGain1=3.0, pkFreq2=3500, pkGain2=-2.0, lpf=4800,
          satMix=0.7, **grit(8.0, 4.0), satCut=6000,
          chMix=0.3, chDelayMs=5, chDepth=0.15, chRate0=0.7, chRate1=5.5, chShape1=2, chFilter=0,
          dlMix=0.1, dlTimeMs=167, dlFeedback=0.15,
          vbMix=0.15, vbPreMs=5, vbSize=0.25, vbDecay=0.4, vbDamp=0.7, vbEarly=0.6, vbTone=0.3)),
    ("Phone Line", "Telephone voice: a narrow mid band, driven hard, with a short slap.",
     dict(hpf=450, pkFreq1=1500, pkQ1=1.2, pkGain1=5.0, lpf=3400,
          satMix=0.6, **grit(10.0, 2.0), satCut=4500,
          dlMix=0.08, dlTimeMs=90, dlFeedback=0.0,
          vbMix=0.08, vbPreMs=0, vbSize=0.1, vbDecay=0.2, vbEarly=0.2, vbTone=0.4)),
    ("Cassette", "Tape: warm and soft on top, saturated, with a gentle wow and flutter.",
     dict(hpf=50, lsFreq=120, lsGain=1.5, hsFreq=9000, hsGain=-2.0, lpf=13000,
          satMix=0.55, **grit(4.0, 2.0), satCut=11000,
          chMix=0.3, chDelayMs=5, chDepth=0.12, chRate0=0.5, chRate1=7.0, chShape1=2, chFilter=0,
          vbMix=0.1, vbSize=0.25, vbDecay=0.35, vbDamp=0.6, vbTone=0.45)),
    ("Megaphone", "Bullhorn: a hard mid band clipped hard, with an outdoor slap.",
     dict(hpf=600, pkFreq1=1800, pkQ1=1.5, pkGain1=6.0, lpf=5000,
          satMix=0.9, **grit(14.0, 2.0), satCut=4500,
          dlMix=0.2, dlTimeMs=140, dlFeedback=0.2,
          vbMix=0.15, vbPreMs=10, vbSize=0.3, vbDecay=0.35, vbEarly=0.5, vbTone=0.45)),
    ("Hot Sauce", "Aggressive: driven hard, forward mids and bite, a quick echo.",
     dict(hpf=120, pkFreq0=300, pkGain0=-3.0, pkFreq1=1200, pkGain1=2.0, pkFreq2=3800, pkGain2=1.5, hsFreq=10000,
          hsGain=0.0, lpf=12000,
          satMix=0.8, **grit(9.0, 4.0), satCut=7000,
          chMix=0.1,
          dlMix=0.15, dlTimeMs=167, dlFeedback=0.3,
          vbMix=0.12, vbSize=0.3, vbDecay=0.35, vbEarly=0.6)),
    ("Gravel", "Gritty boom bap: low warmth, a rough top cut short, an eighth echo and a room.",
     dict(hpf=100, lsFreq=150, lsGain=0.5, pkFreq0=400, pkGain0=-2.0, pkFreq2=2800, pkGain2=1.0, lpf=10000,
          satMix=0.6, **grit(8.0, 3.0), satCut=8000,
          dlMix=0.1, dlTimeMs=333, dlFeedback=0.15,
          vbMix=0.1, vbPreMs=8, vbSize=0.25, vbDecay=0.35, vbEarly=0.5, vbTone=0.5)),
    ("Crate Glue", "For samples and drum loops: weight, gentle tape grit, a soft top and a tight room.",
     dict(hpf=30, lsFreq=90, lsGain=2.0, pkFreq0=350, pkGain0=-1.5, pkFreq2=5000, pkGain2=1.0, lpf=15000,
          satMix=0.5, **grit(5.0, 2.0), satCut=12000,
          vbMix=0.1, vbPreMs=0, vbSize=0.2, vbDecay=0.25, vbEarly=0.3, vbTone=0.5,
          airMix=0.15, airFreq=10000)),
    # --- space
    ("Cathedral", "A huge, long, soft hall with a slow echo behind it.",
     dict(hpf=100, pkFreq0=300, pkGain0=-2.0, hsFreq=9000, hsGain=2.5,
          satMix=0.1, **grit(2.0),
          chMix=0.15, chDelayMs=13, chDepth=0.25,
          dlMix=0.1, dlTimeMs=667, dlFeedback=0.3,
          vbMix=0.4, vbPreMs=45, vbSize=0.95, vbDecay=0.72, vbDensity=0.85, vbDamp=0.55, vbEarly=0.95, vbTone=0.5,
          airMix=0.2, airFreq=11000, airRate=0.05)),
    ("Night Drive", "Moody: a slow wide chorus, dotted echoes trailing off, a dark hall.",
     dict(hpf=100, pkFreq0=350, pkGain0=-2.0, pkFreq2=3000, pkGain2=1.5, lpf=15000,
          satMix=0.25, **grit(3.0),
          chMix=0.3, chDelayMs=13, chDepth=0.3, chRate0=0.25, chRate1=0.15,
          dlMix=0.22, dlTimeMs=500, dlFeedback=0.45,
          vbMix=0.25, vbPreMs=25, vbSize=0.6, vbDecay=0.6, vbDamp=0.65, vbEarly=0.85, vbTone=0.4,
          airMix=0.15, airFreq=10000)),
    ("Echo Alley", "Eighth-note echoes that keep going, over a short room.",
     dict(hpf=120, pkFreq0=300, pkGain0=-2.0, pkFreq2=3500, pkGain2=1.5,
          satMix=0.2, **grit(3.0),
          dlMix=0.3, dlTimeMs=333, dlFeedback=0.55,
          vbMix=0.15, vbSize=0.35, vbDecay=0.4, vbEarly=0.7,
          airMix=0.2, airFreq=10000)),
    ("Slapback", "Fifties slap: one quick repeat, some grit, a small room.",
     dict(hpf=90, pkFreq0=300, pkGain0=-2.0, pkFreq2=3000, pkGain2=2.0, lpf=16000,
          satMix=0.4, **grit(5.0, 2.0),
          dlMix=0.28, dlTimeMs=115, dlFeedback=0.08,
          vbMix=0.12, vbPreMs=5, vbSize=0.2, vbDecay=0.35, vbEarly=0.4)),
    ("Halo", "Ethereal: thin lows, bright air, a wide slow chorus, long echoes and a bright hall.",
     dict(hpf=140, pkFreq0=300, pkGain0=-3.0, hsFreq=8000, hsGain=3.0,
          chMix=0.3, chDelayMs=15, chDepth=0.3, chRate0=0.18, chRate1=0.11, chRate2=0.05, chFreq=200,
          dlMix=0.15, dlTimeMs=667, dlFeedback=0.4,
          vbMix=0.38, vbPreMs=40, vbSize=0.8, vbDecay=0.7, vbDamp=0.3, vbEarly=0.9, vbTone=0.8,
          airMix=0.4, airFreq=11000, airRate=0.15)),
    ("Deep Freeze", "A dark, frozen wash: a near-endless hall with long echoes inside it.",
     dict(hpf=150, pkFreq0=300, pkGain0=-2.0, lpf=12000,
          chMix=0.2, chDelayMs=14, chDepth=0.3, chRate0=0.15, chRate1=0.09,
          dlMix=0.12, dlTimeMs=1000, dlFeedback=0.5,
          vbMix=0.35, vbPreMs=60, vbSize=1.0, vbDecay=0.88, vbDensity=0.9, vbDamp=0.7, vbEarly=1.0, vbTone=0.35)),
    # --- width and movement
    ("Doubler", "A second take beside the first: a slow, slightly late copy, spread wide.",
     dict(hpf=90, pkFreq0=300, pkGain0=-1.5,
          satMix=0.15, **grit(2.0),
          chMix=0.45, chDelayMs=22, chDepth=0.1, chRate0=0.25, chRate1=0.15, chFreq=180,
          vbMix=0.1, vbSize=0.25, vbDecay=0.35, vbEarly=0.6,
          airMix=0.2, airFreq=11000)),
    ("Wide Screen", "Wide: a broad chorus with the lows kept in the middle, eighth echoes and a medium hall.",
     dict(hpf=90, pkFreq0=300, pkGain0=-2.0, hsFreq=11000, hsGain=1.5,
          satMix=0.15, **grit(2.0),
          chMix=0.4, chDelayMs=16, chDepth=0.2, chRate0=0.4, chRate1=0.6, chRate2=0.1, chFreq=150,
          dlMix=0.14, dlTimeMs=333, dlFeedback=0.3,
          vbMix=0.22, vbSize=0.55, vbDecay=0.5,
          airMix=0.25, airFreq=11000)),
    ("Seasick", "An effect: deep vibrato on the whole voice, in a room.",
     dict(hpf=100, pkFreq0=300, pkGain0=-2.0,
          chMix=0.75, chDelayMs=6, chDepth=0.5, chRate0=4.5, chRate1=0.0, chFilter=0,
          vbMix=0.2, vbSize=0.4, vbDecay=0.45)),
    ("Flicker", "An effect: the top end pulses quickly, like a shimmering light.",
     dict(hpf=100, pkFreq0=300, pkGain0=-2.0,
          satMix=0.15, **grit(2.0),
          dlMix=0.12, dlTimeMs=333, dlFeedback=0.25,
          vbMix=0.2, vbSize=0.45, vbDecay=0.45,
          airMix=0.7, airFreq=5000, airRate=5.0)),
]

# Make-up level per preset (dB on LEVEL), so each plays about as loud as the dry voice: measured with
# tools/probe/audition.sh (K-weighted, the mean over the sung phrase and the demo vocal), rounded to 0.5 dB.
LEVELS = {"First Pour": 3.5, "Silk Ribbon": 6.0, "Front Row": 3.0, "Velvet": 6.5, "Honey Glaze": 6.0, "Daylight": 5.5,
          "Whisper": 2.5, "Dusty Wax": 6.5, "Phone Line": 3.0, "Cassette": 5.0, "Megaphone": 8.5, "Hot Sauce": 5.0,
          "Gravel": 3.5, "Crate Glue": 2.0, "Cathedral": 4.0, "Night Drive": 7.0, "Echo Alley": 4.5, "Slapback": 3.5,
          "Halo": 6.0, "Deep Freeze": 5.5, "Doubler": 5.0, "Wide Screen": 6.0, "Seasick": 3.0, "Flicker": 2.5}
for name, _, over in PRESETS:
    over["outDb"] = LEVELS[name]

assert len(PRESETS) == 24 and len({n for n, _, _ in PRESETS}) == 24
for name, _, over in PRESETS:
    bad = set(over) - set(BASE)
    assert not bad, (name, bad)
    assert len(name) <= 14, name


def lit(x):
    s = "%.6g" % x
    return (s if ("." in s or "e" in s) else s + ".0") + "f"


lines = [
    "// Drizzle's 24 factory settings, written by tools/gen_presets.py (edit that, not this).",
    "// Designed for Drizzle in the units of drizzle::Controls; none comes from another plugin.",
    "#pragma once",
    "",
    "namespace drizzle {",
    "",
    "struct Preset {",
    "    const char *name;",
    "    float " + ", ".join(FIELDS) + ";",
    "};",
    "",
    "const int kNumPresets = %d;" % len(PRESETS),
    "",
    "const Preset kPresets[kNumPresets] = {",
]
for name, about, over in PRESETS:
    v = dict(BASE, **over)
    lines.append("    // %s" % about)
    lines.append('    {"%s", %s},' % (name, ", ".join(lit(float(v[f])) for f in FIELDS)))
lines += ["};", "", "}  // namespace drizzle", ""]
open(OUT, "w").write("\n".join(lines))
print("%s: %d presets, %d values each" % (os.path.relpath(OUT), len(PRESETS), len(FIELDS)))
