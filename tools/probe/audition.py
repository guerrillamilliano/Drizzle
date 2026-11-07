#!/usr/bin/env python3
"""Measures Drizzle's factory presets on test material (sauce-py image), for comparing two preset sets.

  python3 audition.py make <dir>                  -> <dir>/sung.f32, <dir>/demo.f32 (peaks at -6 dBFS)
  python3 audition.py show <dir> <sig> <tag> [<names.txt>]   one line per rendered preset <dir>/<sig>_<tag>NN.f32

Columns: LOUD level change against the dry input (K-weighted, active part, dB); PEAK output peak (dBFS);
LOW..AIR the tone change per band against the change at 600-2400 Hz (dB); WIDE side over mid (dB, -inf for mono);
MONO the level lost when the two sides are summed (dB, near 0 is safe); TAIL level 1.5-2 s after the input stops
(dB under the active level)."""
import os
import sys
import wave

import numpy as np
from scipy.signal import bilinear, lfilter, resample_poly

SR = 44100


def sung(sec=6.0):
    r = np.random.RandomState(7)
    t = np.arange(int(SR * sec)) / SR
    f0 = 180 * (1 + 0.02 * np.sin(2 * np.pi * 5.5 * t)) * np.where((t % 1.25) < 0.6, 1.0, 1.26)
    ph = np.cumsum(2 * np.pi * f0 / SR)
    x = sum(np.sin(k * ph) / k for k in range(1, 60))
    from scipy.signal import iirpeak
    y = np.zeros_like(x)
    for fc, q, g in ((700, 4, 1.0), (1200, 5, 0.6), (2600, 6, 0.35), (3400, 8, 0.2), (8000, 2, 0.05)):
        b, a = iirpeak(fc, q, SR)
        y += g * lfilter(b, a, x)
    y += 0.04 * r.randn(t.size) * ((t % 1.25) < 0.06)
    env = np.clip(np.minimum((t % 1.25) / 0.02, (0.9 - (t % 1.25)) / 0.08), 0, 1)
    y *= env * (t < 4.9)
    return y


def kfilter(x):
    # K-weighting (BS.1770) at 44.1 kHz: a +4 dB shelf above ~1.5 kHz and a high-pass near 38 Hz
    b1 = [1.53090959, -2.65116903, 1.16916686]; a1 = [1.0, -1.66375011, 0.71265753]
    b2 = [1.0, -2.0, 1.0]; a2 = [1.0, -1.98998364, 0.99000400]
    return lfilter(b2, a2, lfilter(b1, a1, x, axis=0), axis=0)


def db(p):
    return 10 * np.log10(p + 1e-20)


EDGES = [(40, 200), (200, 600), (600, 2400), (2400, 6000), (6000, 16000)]


def bands(x):
    f = np.fft.rfftfreq(x.size, 1 / SR)
    X = abs(np.fft.rfft(x * np.hanning(x.size))) ** 2
    return np.array([db(X[(f >= a) & (f < b)].sum()) for a, b in EDGES])


def main():
    d = sys.argv[2]
    if sys.argv[1] == "make":
        x = sung()
        w = wave.open("/demo.wav")
        n, ch, sw, sr = w.getnframes(), w.getnchannels(), w.getsampwidth(), w.getframerate()
        y = np.frombuffer(w.readframes(n), "<i2").reshape(-1, ch).mean(1) / 32768.0
        if sr != SR:
            y = resample_poly(y, SR, sr)
        for name, s in (("sung", x), ("demo", y)):
            s = 0.5 * s / abs(s).max()
            np.stack([s, s], 1).astype("<f4").tofile(os.path.join(d, name + ".f32"))
            print(name, "%.2f s" % (s.size / SR))
        return
    sig, tag = sys.argv[3], sys.argv[4]
    names = open(sys.argv[5]).read().splitlines() if len(sys.argv) > 5 else []
    x = np.fromfile(os.path.join(d, sig + ".f32"), "<f4").reshape(-1, 2).astype(np.float64)[:, 0]
    act = np.flatnonzero(abs(x) > 1e-4)
    end = act[-1] + 1
    xk = kfilter(x[:end])
    lin = db(np.mean(xk ** 2))
    bi = bands(x[:end])
    print("%-14s %6s %6s | %5s %5s %5s %5s %5s | %6s %5s %6s" % ("preset", "LOUD", "PEAK", "LOW", "LMID", "MID",
                                                                  "PRES", "AIR", "WIDE", "MONO", "TAIL"))
    i = 0
    while os.path.exists(os.path.join(d, "%s_%s%02d.f32" % (sig, tag, i))):
        y = np.fromfile(os.path.join(d, "%s_%s%02d.f32" % (sig, tag, i)), "<f4").reshape(-1, 2).astype(np.float64)
        L, R = y[:, 0], y[:, 1]
        a = slice(0, end)
        k = kfilter(y[a])
        loud = db(np.mean(k ** 2)) - lin
        st = db(np.mean(k ** 2))                       # both sides, averaged
        mono = db(np.mean(((k[:, 0] + k[:, 1]) / 2) ** 2)) - st
        mid, side = (L + R) / 2, (L - R) / 2
        wide = db(np.mean(side[a] ** 2)) - db(np.mean(mid[a] ** 2))
        b = bands(mid[a]) - bi
        b = b - b[2]                                   # tone tilt: each band against the mids
        t0, t1 = end + int(1.5 * SR), end + int(2.0 * SR)
        tail = db(np.mean(y[t0:t1] ** 2)) - db(np.mean(y[a] ** 2))
        peak = 20 * np.log10(abs(y).max() + 1e-20)
        nm = names[i] if i < len(names) else "%s%02d" % (tag, i)
        print("%-14s %+6.1f %6.1f | %s | %6.1f %+5.1f %6.1f" % (nm[:14], loud, peak, " ".join("%+5.1f" % v for v in b),
                                                               wide, mono, tail))
        i += 1


main()
