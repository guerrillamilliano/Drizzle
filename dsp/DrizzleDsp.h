// Drizzle: a one-knob vocal chain (EQ -> saturation -> chorus -> delay -> reverb -> moving air shelf).
// C++11, no dependencies, 44.1 kHz stereo float. The section shapes were measured from the outside on a reference
// effect (tools/probe/), and the controls use its units: Hz, dB, ms, and 0..1 amounts.
#pragma once
#include <math.h>
#include <stdint.h>
#include <string.h>

namespace drizzle {

const float kSampleRate = 44100.0f;
const int kControlInterval = 32;     // coefficients and LFO-driven shelves update this often (0.7 ms)
const float kPi = 3.14159265358979f;

inline float clampf(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }
inline float dbToLin(float db) { return powf(10.0f, db * 0.05f); }

// ---------------------------------------------------------------------------------------------------------- controls
struct Controls {
    float overall = 1.0f;                 // scales every section's amount below (0 = the dry signal)
    float outDb = 0;                      // level of the processed voice (dB) before the DRIZZLE blend
    float eqMix = 0, satMix = 0, chMix = 0, vbMix = 0, dlMix = 0;   // 0..1, linear dry/wet blends
    float dlTimeMs = 0, dlFeedback = 0;   // 0..1000 ms, -1..1
    float airRate = 0, airFreq = 20000, airMix = 0;   // Hz (LFO), Hz (shelf corner), 0..1
    // EQ
    float hpf = 20, lsFreq = 20, lsQ = 0.1f, lsGain = 0;
    float pkFreq[3] = {1000, 2000, 4000}, pkQ[3] = {1, 1, 1}, pkGain[3] = {0, 0, 0};
    float hsFreq = 20000, hsQ = 0.1f, hsGain = 0, lpf = 20000;
    // saturation (dB, Hz)
    float satDrive = 0, posIn = 0, negIn = 0, posOut = 0, negOut = 0, satTrim = 0, satCut = 20000;
    // chorus
    float chDelayMs = 0, chDepth = 1, chStereoDeg = 0;
    float chRate[3] = {0, 0, 0}, chShape[3] = {0, 0, 0};   // Hz, 0 sine / 1 saw / 2 triangle
    float chFilter = 0, chFreq = 20000;                    // 0 off / 1 high-pass / 2 low-pass
    // reverb (0..1 except pre-delay)
    float vbPreMs = 0, vbSize = 0.5f, vbDecay = 0.5f, vbDensity = 0.5f, vbDamp = 0.5f, vbEarly = 0.5f, vbTone = 0.5f;
};

// ---------------------------------------------------------------------------------------------------------- biquads
struct Coefs {
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
};

inline Coefs norm(double b0, double b1, double b2, double a0, double a1, double a2) {
    Coefs c;
    c.b0 = (float)(b0 / a0);
    c.b1 = (float)(b1 / a0);
    c.b2 = (float)(b2 / a0);
    c.a1 = (float)(a1 / a0);
    c.a2 = (float)(a2 / a0);
    return c;
}

inline double safeHz(double f) { return f < 2.0 ? 2.0 : (f > 21000.0 ? 21000.0 : f); }

inline Coefs lowPass(double f, double q) {
    double n = 1.0 / tan(kPi * safeHz(f) / kSampleRate), n2 = n * n, iq = 1.0 / q;
    return norm(1, 2, 1, 1 + iq * n + n2, 2 * (1 - n2), 1 - iq * n + n2);
}

inline Coefs highPass(double f, double q) {
    double n = tan(kPi * safeHz(f) / kSampleRate), n2 = n * n, iq = 1.0 / q;
    return norm(1, -2, 1, 1 + iq * n + n2, 2 * (n2 - 1), 1 - iq * n + n2);
}

inline Coefs peak(double f, double q, double gainLin) {
    double A = sqrt(gainLin > 0 ? gainLin : 0), w = 2 * kPi * safeHz(f) / kSampleRate, al = sin(w) / (2 * q), c = -2 * cos(w);
    return norm(1 + al * A, c, 1 - al * A, 1 + al / A, c, 1 - al / A);
}

inline Coefs lowShelf(double f, double q, double gainLin) {
    double A = sqrt(gainLin > 0 ? gainLin : 0), am = A - 1, ap = A + 1, w = 2 * kPi * safeHz(f) / kSampleRate;
    double co = cos(w), beta = sin(w) * sqrt(A) / q, amc = am * co;
    return norm(A * (ap - amc + beta), A * 2 * (am - ap * co), A * (ap - amc - beta), ap + amc + beta, -2 * (am + ap * co),
                ap + amc - beta);
}

inline Coefs highShelf(double f, double q, double gainLin) {
    double A = sqrt(gainLin > 0 ? gainLin : 0), am = A - 1, ap = A + 1, w = 2 * kPi * safeHz(f) / kSampleRate;
    double co = cos(w), beta = sin(w) * sqrt(A) / q, amc = am * co;
    return norm(A * (ap + amc + beta), A * -2 * (am + ap * co), A * (ap + amc - beta), ap - amc + beta, 2 * (am - ap * co),
                ap - amc - beta);
}

struct Biquad {             // transposed direct form II, one channel
    Coefs c;
    float z1 = 0, z2 = 0;
    inline float run(float x) {
        float y = c.b0 * x + z1;
        z1 = c.b1 * x - c.a1 * y + z2;
        z2 = c.b2 * x - c.a2 * y;
        return y;
    }
    void reset() { z1 = z2 = 0; }
};

// Filter pairs that are cheap to skip while flat
struct StereoBiquad {
    Biquad l, r;
    bool active = true;
    void set(const Coefs &c, bool on) {
        l.c = r.c = c;
        if (on && !active) { l.reset(); r.reset(); }
        active = on;
    }
    inline void run(float &a, float &b) {
        if (!active) return;
        a = l.run(a);
        b = r.run(b);
    }
};

// ---------------------------------------------------------------------------------------------------------- delay line
template <int N>                    // N a power of two
struct Line {
    float buf[N];
    int w = 0;
    Line() { memset(buf, 0, sizeof buf); }
    inline void push(float x) { buf[w] = x; w = (w + 1) & (N - 1); }
    inline float tap(int d) const { return buf[(w - 1 - d) & (N - 1)]; }          // d = 0: the newest sample
    inline float frac(float d) const {                                            // linear interpolation
        int i = (int)d;
        float f = d - (float)i;
        return tap(i) + (tap(i + 1) - tap(i)) * f;
    }
};

// ---------------------------------------------------------------------------------------------------------- sections
// EQ: high-pass, low shelf, three bells, high shelf, low-pass (each a 12 dB/oct or cookbook section)
struct Equaliser {
    StereoBiquad hpf, ls, pk[3], hs, lpf;
    void configure(const Controls &c) {
        hpf.set(highPass(c.hpf, 0.8), true);
        ls.set(lowShelf(c.lsFreq, c.lsQ, dbToLin(c.lsGain)), fabsf(c.lsGain) > 0.01f);
        for (int i = 0; i < 3; i++) pk[i].set(peak(c.pkFreq[i], c.pkQ[i], dbToLin(c.pkGain[i])), fabsf(c.pkGain[i]) > 0.01f);
        hs.set(highShelf(c.hsFreq, c.hsQ, dbToLin(c.hsGain)), fabsf(c.hsGain) > 0.01f);
        lpf.set(lowPass(c.lpf, 0.8), true);
    }
    inline void run(float &a, float &b) {
        hpf.run(a, b);
        ls.run(a, b);
        for (int i = 0; i < 3; i++) pk[i].run(a, b);
        hs.run(a, b);
        lpf.run(a, b);
    }
};

// Saturation: gain, separate gains for the two halves, +6 dB into a clipper (straight up to 0.7, a smooth knee to
// full scale at 1.3), output gains per half, DC blocker, then a resonant 12 dB/oct low-pass. No oversampling and
// no latency, like the reference: its overshoot past full scale comes from that low-pass.
struct Saturator {
    float pre = 1, posIn = 1, negIn = 1, posOut = 1, negOut = 1;
    float dcX[2] = {0, 0}, dcY[2] = {0, 0};
    StereoBiquad post;
    void configure(const Controls &c) {
        pre = dbToLin(c.satDrive + c.satTrim);
        posIn = dbToLin(c.posIn);
        negIn = dbToLin(c.negIn);
        posOut = dbToLin(c.posOut);
        negOut = dbToLin(c.negOut);
        post.set(lowPass(c.satCut, 1.0), true);
    }
    static inline float clip(float u) {
        const float a = 0.7f, b = 1.3f;
        float m = fabsf(u);
        if (m <= a) return u;
        float v = m >= b ? 1.0f : m - (m - a) * (m - a) / (2.0f * (b - a));
        return u < 0 ? -v : v;
    }
    inline float shape(float x, int ch) {
        float u = x * pre;
        u *= u > 0 ? posIn : negIn;
        float y = clip(2.0f * u);
        y *= y > 0 ? posOut : negOut;
        float o = y - dcX[ch] + 0.9995f * dcY[ch];   // DC blocker, about 3.5 Hz
        dcX[ch] = y;
        dcY[ch] = o;
        return o;
    }
    inline void run(float &a, float &b) {
        a = shape(a, 0);
        b = shape(b, 1);
        post.run(a, b);
    }
};

// One LFO: 0 sine, 1 rising saw, 2 triangle (whole-number shapes: the fraction is dropped). The saw is built from its
// first six harmonics, as the reference's is: it steps as it rises and falls over about a seventh of a cycle, so a
// fast saw never clicks. Tables of 1024 points, read with linear interpolation.
struct LfoTables {
    enum { N = 1024 };
    float t[3][N + 1];
    LfoTables() {
        float peak = 0;
        for (int i = 0; i <= N; i++) {
            double x = 2.0 * 3.14159265358979 * i / N, saw = 0;
            for (int k = 1; k <= 6; k++) saw += (k & 1 ? 1.0 : -1.0) * sin(k * x) / k;
            t[0][i] = (float)sin(x);
            t[1][i] = (float)saw;
            if (fabs(saw) > peak) peak = (float)fabs(saw);
            double p = (double)i / N - 0.25;
            p -= floor(p);
            t[2][i] = (float)(4.0 * fabs(p - 0.5) - 1.0);
        }
        for (int i = 0; i <= N; i++) t[1][i] /= peak;
    }
};

inline float lfoShape(float phase, int shape) {
    static const LfoTables tables;
    float x = phase * LfoTables::N;
    int i = (int)x;
    if (i >= LfoTables::N) i = LfoTables::N - 1;
    float f = x - (float)i;
    const float *t = tables.t[shape < 0 ? 0 : (shape > 2 ? 2 : shape)];
    return t[i] + (t[i + 1] - t[i]) * f;
}


// Chorus: one voice per side, the delay swinging around its base in proportion to it (three summed LFOs, the
// sides in opposite phase), the two voices rotated across the stereo field, then an optional filter
struct Chorus {
    Line<4096> dl[2];
    double phase[3] = {0, 0, 0};
    float rot_c = 1, rot_s = 0;
    int shape[3] = {0, 0, 0};
    float rate[3] = {0, 0, 0};
    float base = 0, depth = 0;
    StereoBiquad filt;
    void configure(const Controls &c) {
        base = clampf(c.chDelayMs, 0.0f, 30.0f) * 0.001f * kSampleRate;
        depth = clampf(c.chDepth, 0.0f, 1.0f) * 0.25f;
        float phi = clampf(c.chStereoDeg, 0.0f, 180.0f);   // the reference feeds the degrees to sin/cos as radians
        rot_c = cosf(phi);
        rot_s = sinf(phi);
        for (int i = 0; i < 3; i++) {
            rate[i] = clampf(c.chRate[i], 0.0f, 20.0f);
            shape[i] = (int)clampf(c.chShape[i], 0.0f, 2.0f);
        }
        int t = (int)clampf(c.chFilter, 0.0f, 2.0f);
        if (t == 1) filt.set(highPass(c.chFreq, 0.8), true);
        else if (t == 2) filt.set(lowPass(c.chFreq, 0.8), true);
        else filt.set(Coefs(), false);
    }
    inline void run(float &a, float &b) {
        dl[0].push(a);
        dl[1].push(b);
        float m = 0;
        const float w[3] = {0.5f, 0.5f, 1.0f};
        for (int i = 0; i < 3; i++) {
            if (rate[i] <= 0) continue;   // a stopped LFO sits at its start, which is zero for every shape
            m += w[i] * lfoShape((float)phase[i], shape[i]);
            phase[i] += rate[i] / kSampleRate;
            if (phase[i] >= 1.0) phase[i] -= 1.0;
        }
        float dA = base * (1.0f + depth * m), dB = base * (1.0f - depth * m);
        float va = dl[0].frac(clampf(dA, 0.0f, 4000.0f)), vb = dl[1].frac(clampf(dB, 0.0f, 4000.0f));
        a = rot_c * va - rot_s * vb;
        b = rot_s * va + rot_c * vb;
        filt.run(a, b);
    }
};

// Delay: a plain echo per side, feedback -100..100 % with nothing in the loop
struct Echo {
    Line<65536> dl[2];
    float time = -1, target = 1, fb = 0;
    void configure(const Controls &c) {
        target = clampf(c.dlTimeMs, 0.0f, 1000.0f) * 0.001f * kSampleRate;
        if (target < 1) target = 1;
        if (time < 0) time = target;          // the first setting takes hold at once
        fb = clampf(c.dlFeedback, -1.0f, 1.0f);
    }
    inline void run(float &a, float &b) {
        time += (target - time) * 0.0005f;   // glide to a new time (a short pitch bend, no clicks)
        float d = time - 1.0f;
        float ya = dl[0].frac(d), yb = dl[1].frac(d);
        dl[0].push(clampf(a + fb * ya, -8.0f, 8.0f));
        dl[1].push(clampf(b + fb * yb, -8.0f, 8.0f));
        a = ya;
        b = yb;
    }
    inline void idle() {     // while the echo is off its line fills with silence, so turning it up plays nothing stale
        time += (target - time) * 0.0005f;
        dl[0].push(0);
        dl[1].push(0);
    }
};

// Reverb: early reflections (fixed taps, then smeared) crossfaded with an 8-line feedback delay network
struct Allpass {
    Line<2048> d;
    int len = 100;
    float g = 0.5f;
    inline float run(float x) {
        float z = d.tap(len - 1);
        float v = x + g * z;
        d.push(v);
        return z - g * v;
    }
};

struct OnePole {            // y += k (x - y)
    float k = 1, y = 0;
    void setHz(float hz) { k = hz >= 20000.0f ? 1.0f : 1.0f - expf(-2.0f * kPi * hz / kSampleRate); }
    inline float run(float x) { return y += k * (x - y); }
};

struct Reverb {
    Line<8192> in;                      // mono input history: early taps and the pre-delayed feed
    Line<4096> line[8];
    Allpass diff[5], erSmear[2][2];
    float g[8], damp = 0.2f, lp[8], norm = 1, er = 0.5f, preSamples = 0, toneHz = -1, toneGain = 1;
    int len[8];
    float density = -1;
    OnePole tone[2];
    Biquad body[2];                     // the wet signal's fixed darkening

    // early reflections: measured tap times (ms) left and right, and levels
    static const int kTaps = 6;
    Reverb() {
        memset(lp, 0, sizeof lp);
        static const int dlen[5] = {205, 311, 478, 703, 994};    // a diffuser: the late field builds up over ~50 ms
        for (int i = 0; i < 5; i++) diff[i].len = dlen[i];
        static const int slen[2][2] = {{67, 113}, {79, 127}};
        for (int s = 0; s < 2; s++)
            for (int k = 0; k < 2; k++) {
                erSmear[s][k].len = slen[s][k];
                erSmear[s][k].g = s ? -0.45f : 0.45f;     // opposite smear on the right: a wider early field
            }
        body[0].c = body[1].c = lowPass(10000, 0.7071);
        Controls c;
        configure(c);
    }
    void configure(const Controls &c) {
        float size = c.vbSize <= 0.001f ? 1.0f : clampf(c.vbSize, 0.0f, 1.0f);   // size 0 behaves as the largest room
        float dm = clampf(c.vbDamp, 0.0f, 1.0f), dm8 = powf(dm, 8.0f);
        float rt = (0.13f + 2.77f * size) * powf(15.0f, clampf(c.vbDecay, 0.0f, 1.0f) - 0.5f) * (1.0f - 0.78f * dm8);
        static const int base[8] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
        float scale = 0.55f + 0.85f * size;
        for (int i = 0; i < 8; i++) {
            len[i] = (int)(base[i] * scale);
            g[i] = powf(10.0f, -3.0f * len[i] / (rt * kSampleRate));
        }
        damp = 0.06f + 0.22f * dm + 0.1f * dm8;
        norm = kLateGain * powf(rt / 1.5f, rt > 1.5f ? -0.1f : -0.4f);   // longer tails carry a little more energy
        er = 1.0f - clampf(c.vbEarly, 0.0f, 1.0f);
        preSamples = clampf(c.vbPreMs, 0.0f, 100.0f) * 0.001f * kSampleRate;
        float dens = clampf(c.vbDensity, 0.0f, 1.0f);
        if (dens != density) {
            density = dens;
            for (int i = 0; i < 5; i++) diff[i].g = 0.5f + 0.2f * dens;
        }
        float t = clampf(c.vbTone, 0.0f, 1.0f);
        float hz = 100.0f + 19900.0f * powf(t, 1.3f);
        toneGain = dbToLin(2.6f * (t - 0.5f));   // opening the tone also raises the wet level
        if (hz != toneHz) {
            toneHz = hz;
            tone[0].setHz(hz);
            tone[1].setHz(hz);
        }
    }
    void clear() {
        for (int i = 0; i < 8; i++) memset(line[i].buf, 0, sizeof line[i].buf);
        memset(in.buf, 0, sizeof in.buf);
        memset(lp, 0, sizeof lp);
        for (int i = 0; i < 5; i++) memset(diff[i].d.buf, 0, sizeof diff[i].d.buf);
        for (int s = 0; s < 2; s++)
            for (int k = 0; k < 2; k++) memset(erSmear[s][k].d.buf, 0, sizeof erSmear[s][k].d.buf);
        tone[0].y = tone[1].y = 0;
        body[0].reset();
        body[1].reset();
    }
    static constexpr float kLateGain = 0.35f, kEarlyGain = 2.05f;
    inline void run(float &a, float &b) {
        float x = 0.5f * (a + b);
        in.push(x);
        // early reflections
        static const float tL[kTaps] = {19.82f, 47.62f, 50.11f, 53.61f, 67.12f, 89.0f};
        static const float tR[kTaps] = {25.92f, 47.71f, 50.11f, 50.82f, 58.0f, 69.0f};
        static const float gT[kTaps] = {0.040f, 0.120f, 0.120f, 0.160f, 0.240f, 0.440f};
        float el = 0, eR = 0;
        for (int k = 0; k < kTaps; k++) {
            el += gT[k] * in.tap((int)(tL[k] * 44.1f));
            eR += gT[k] * in.tap((int)(tR[k] * 44.1f));
        }
        el = kEarlyGain * erSmear[0][1].run(erSmear[0][0].run(el));
        eR = kEarlyGain * erSmear[1][1].run(erSmear[1][0].run(eR));
        // late field
        float f = in.frac(preSamples);
        for (int i = 0; i < 5; i++) f = diff[i].run(f);
        float o[8];
        for (int i = 0; i < 8; i++) o[i] = line[i].tap(len[i] - 1);
        float h[8];                     // 8-point Hadamard mix
        for (int i = 0; i < 8; i++) h[i] = o[i];
        for (int s = 1; s < 8; s <<= 1)
            for (int i = 0; i < 8; i += 2 * s)
                for (int j = i; j < i + s; j++) {
                    float u = h[j], v = h[j + s];
                    h[j] = u + v;
                    h[j + s] = u - v;
                }
        const float hs = 0.35355339f;   // 1/sqrt(8)
        for (int i = 0; i < 8; i++) {
            float v = h[i] * hs * g[i];
            lp[i] = v + damp * (lp[i] - v);
            line[i].push(lp[i] + (i & 1 ? -f : f));
        }
        float ll = o[0] - o[2] + o[4] - o[6] + 0.5f * (o[1] + o[7]);
        float lr = o[1] - o[3] + o[5] - o[7] + 0.5f * (o[2] - o[4]);
        float late = 1.0f - er;
        float wl = toneGain * (er * el + late * norm * ll), wr = toneGain * (er * eR + late * norm * lr);
        wl = tone[0].run(body[0].run(wl));
        wr = tone[1].run(body[1].run(wr));
        float direct = (0.218f * er + 0.141f * late) * x;
        a = direct + wl;
        b = direct + wr;
    }
};

// The moving air shelf. Measured as a state-variable filter (corner 1.26 x the knob, Q 0.92) whose low-pass is kept
// and whose high band (0.889 high-pass + 0.501 band-pass) is scaled by g(t) = 3.24 - 2.71 sin(LFO): about +9 dB at
// rest, swinging between -5.5 and +14.5 dB, with a dip at the corner as the high band crosses unity. Its mix blends it
// with the dry signal. The filter never changes while the LFO runs, so the sweep is click-free.
struct AirShelf {
    double phase = 0;
    float rate = 0, freq = -1, a1 = 1, a2 = 0, a3 = 0, k = 1;
    float s1[2] = {0, 0}, s2[2] = {0, 0};
    float gain = 3.24f, gainStep = 0;
    void configure(float f, float hz) {
        rate = hz;
        if (f != freq) {
            freq = f;
            float fc = clampf(1.258f * f, 10.0f, 0.47f * kSampleRate);
            float g = tanf(kPi * fc / kSampleRate);
            k = 1.0f / 0.922f;
            a1 = 1.0f / (1.0f + g * (g + k));
            a2 = g * a1;
            a3 = g * a2;
        }
    }
    void block() {           // once per control interval: the high band's gain ramps to the LFO's next value
        phase += (double)rate * kControlInterval / kSampleRate;
        phase -= floor(phase);
        float next = 3.237f - 2.71f * sinf(2.0f * kPi * (float)phase);
        gainStep = (next - gain) / kControlInterval;
    }
    inline float tick(float x, int ch) {
        float v3 = x - s2[ch];
        float v1 = a1 * s1[ch] + a2 * v3;
        float v2 = s2[ch] + a2 * s1[ch] + a3 * v3;
        s1[ch] = 2.0f * v1 - s1[ch];
        s2[ch] = 2.0f * v2 - s2[ch];
        float hp = x - k * v1 - v2;
        return v2 + gain * (0.889f * hp + 0.501f * k * v1);
    }
    inline void run(float &a, float &b) {
        gain += gainStep;
        a = tick(a, 0);
        b = tick(b, 1);
    }
};

// ---------------------------------------------------------------------------------------------------------- the chain
class Core {
public:
    void configure(const Controls &c) {
        ctl = c;
        mOverall = clampf(c.overall, 0.0f, 1.0f);   // one blend of the dry input with the whole chain
        mGain = dbToLin(clampf(c.outDb, -24.0f, 24.0f));
        mEq = clampf(c.eqMix, 0, 1);
        mSat = clampf(c.satMix, 0, 1);
        mCh = clampf(c.chMix, 0, 1);
        mDl = clampf(c.dlMix, 0, 1);
        mVb = clampf(c.vbMix, 0, 1);
        mAir = clampf(c.airMix, 0, 1);
        eq.configure(c);
        sat.configure(c);
        ch.configure(c);
        dl.configure(c);
        vb.configure(c);
        air.configure(c.airFreq, clampf(c.airRate, 0.0f, 20.0f));
    }
    // n <= kControlInterval frames, in place
    void process(float *L, float *R, int n) {
        air.block();
        for (int i = 0; i < n; i++) {
            const float dryA = L[i], dryB = R[i];
            float a = dryA, b = dryB;
            if (mEq > 0) mixIn(a, b, mEq, [this](float &x, float &y) { eq.run(x, y); });
            if (mSat > 0) mixIn(a, b, mSat, [this](float &x, float &y) { sat.run(x, y); });
            if (mCh > 0) mixIn(a, b, mCh, [this](float &x, float &y) { ch.run(x, y); });
            else ch.dl[0].push(a), ch.dl[1].push(b);
            if (mDl > 0) mixIn(a, b, mDl, [this](float &x, float &y) { dl.run(x, y); });
            else dl.idle();
            if (mVb > 0) {
                if (vbIdle > kSampleRate) vb.clear();    // off for a while: start from silence, not a frozen tail
                vbIdle = 0;
                mixIn(a, b, mVb, [this](float &x, float &y) { vb.run(x, y); });
            } else if (vbIdle <= kSampleRate) {
                vbIdle++;
            }
            if (mAir > 0) mixIn(a, b, mAir, [this](float &x, float &y) { air.run(x, y); });
            L[i] = dryA + mOverall * (mGain * a - dryA);
            R[i] = dryB + mOverall * (mGain * b - dryB);
        }
    }
    const Controls &controls() const { return ctl; }

private:
    template <class F>
    static inline void mixIn(float &a, float &b, float m, F f) {
        float wa = a, wb = b;
        f(wa, wb);
        a += m * (wa - a);
        b += m * (wb - b);
    }
    Controls ctl;
    float mOverall = 1, mGain = 1, mEq = 0, mSat = 0, mCh = 0, mDl = 0, mVb = 0, mAir = 0;
    int vbIdle = 0;
    Equaliser eq;
    Saturator sat;
    Chorus ch;
    Echo dl;
    Reverb vb;
    AirShelf air;
};

}  // namespace drizzle
