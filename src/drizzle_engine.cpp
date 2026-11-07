// Drizzle on MPC: adapts the DSP core (dsp/DrizzleDsp.h) to the toolkit's engine interface (wrapper/engine.h).
// The sound lives in the core; this file holds the parameter table, smoothing, presets, section styles and saved state.
// Parameter order is the VST index order and must match tools/gen_params.py (test/engine_test.cpp checks it).
extern "C" {
#include "engine.h"
}
#include "DrizzleDsp.h"
#include "DrizzlePresets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace drizzle;

namespace {

// kinds: p percent 0..100, b bipolar percent, f frequency as a log position 0..100 (20 Hz .. 20 kHz), d dB, q Q,
// m ms, h Hz (LFO rate), w width (the reference's degrees), o option index (no smoothing)
struct Def {
    const char *key;
    char kind;
};

enum {
    P_DRIZZLE, P_TONE, P_GRIT, P_SWIRL, P_SPACE, P_DLY_TIME, P_DLY_FB, P_DLY_MIX, P_AIR_RATE, P_AIR_FREQ, P_AIR_MIX,
    P_EQ_HPF, P_EQ_LS_F, P_EQ_LS_Q, P_EQ_LS_G, P_EQ_B1_F, P_EQ_B1_Q, P_EQ_B1_G, P_EQ_B2_F, P_EQ_B2_Q, P_EQ_B2_G,
    P_EQ_B3_F, P_EQ_B3_Q, P_EQ_B3_G, P_EQ_HS_F, P_EQ_HS_Q, P_EQ_HS_G, P_EQ_LPF,
    P_GR_DRIVE, P_GR_POS_IN, P_GR_NEG_IN, P_GR_POS_OUT, P_GR_NEG_OUT, P_GR_TRIM, P_GR_CUT,
    P_SW_DELAY, P_SW_DEPTH, P_SW_WIDTH, P_SW_RATE1, P_SW_RATE2, P_SW_RATE3, P_SW_SHAPE1, P_SW_SHAPE2, P_SW_SHAPE3,
    P_SW_FILTER, P_SW_FREQ,
    P_SP_PRE, P_SP_SIZE, P_SP_DECAY, P_SP_DENSITY, P_SP_DAMP, P_SP_LATE, P_SP_TONE,
    P_LEVEL,                    // appended in 1.1.0: VST indices of the others stay put
    NSTORED
};

const Def DEFS[NSTORED] = {
    {"drizzle", 'p'}, {"tone", 'p'}, {"grit", 'p'}, {"swirl", 'p'}, {"space", 'p'},
    {"dly_time", 'm'}, {"dly_fb", 'b'}, {"dly_mix", 'p'}, {"air_rate", 'h'}, {"air_freq", 'f'}, {"air_mix", 'p'},
    {"eq_hpf", 'f'}, {"eq_ls_f", 'f'}, {"eq_ls_q", 'q'}, {"eq_ls_g", 'd'}, {"eq_b1_f", 'f'}, {"eq_b1_q", 'q'},
    {"eq_b1_g", 'd'}, {"eq_b2_f", 'f'}, {"eq_b2_q", 'q'}, {"eq_b2_g", 'd'}, {"eq_b3_f", 'f'}, {"eq_b3_q", 'q'},
    {"eq_b3_g", 'd'}, {"eq_hs_f", 'f'}, {"eq_hs_q", 'q'}, {"eq_hs_g", 'd'}, {"eq_lpf", 'f'},
    {"gr_drive", 'd'}, {"gr_pos_in", 'd'}, {"gr_neg_in", 'd'}, {"gr_pos_out", 'd'}, {"gr_neg_out", 'd'},
    {"gr_trim", 'd'}, {"gr_cut", 'f'},
    {"sw_delay", 'm'}, {"sw_depth", 'p'}, {"sw_width", 'w'}, {"sw_rate1", 'h'}, {"sw_rate2", 'h'}, {"sw_rate3", 'h'},
    {"sw_shape1", 'o'}, {"sw_shape2", 'o'}, {"sw_shape3", 'o'}, {"sw_filter", 'o'}, {"sw_freq", 'f'},
    {"sp_pre", 'm'}, {"sp_size", 'p'}, {"sp_decay", 'p'}, {"sp_density", 'p'}, {"sp_damp", 'p'}, {"sp_late", 'p'},
    {"sp_tone", 'p'}, {"level", 'd'},
};

// The four section styles load one section's inner settings (not its amount) from a factory preset
enum { S_TONE, S_GRIT, S_SWIRL, S_SPACE, NSTYLES };
const char *const STYLE_KEYS[NSTYLES] = {"tone_style", "grit_style", "swirl_style", "space_style"};
const int STYLE_FIRST[NSTYLES] = {P_EQ_HPF, P_GR_DRIVE, P_SW_DELAY, P_SP_PRE};
const int STYLE_LAST[NSTYLES] = {P_EQ_LPF, P_GR_CUT, P_SW_FREQ, P_SP_TONE};

const float SMOOTH_K = 0.12f;      // one-pole per 32-frame block: about 6 ms

inline float hzToPos(float hz) { return clampf(100.0f * log10f(clampf(hz, 20.0f, 20000.0f) / 20.0f) / 3.0f, 0.0f, 100.0f); }
inline float posToHz(float p) { return 20.0f * powf(1000.0f, clampf(p, 0.0f, 100.0f) * 0.01f); }

// A factory preset in the stored units (positions for frequencies, percent for amounts)
void presetValues(const Preset &p, float *v) {
    v[P_DRIZZLE] = 100 * p.overall;
    v[P_TONE] = 100 * p.eqMix;
    v[P_GRIT] = 100 * p.satMix;
    v[P_SWIRL] = 100 * p.chMix;
    v[P_SPACE] = 100 * p.vbMix;
    v[P_DLY_TIME] = p.dlTimeMs;
    v[P_DLY_FB] = 100 * p.dlFeedback;
    v[P_DLY_MIX] = 100 * p.dlMix;
    v[P_AIR_RATE] = p.airRate;
    v[P_AIR_FREQ] = hzToPos(p.airFreq);
    v[P_AIR_MIX] = 100 * p.airMix;
    v[P_EQ_HPF] = hzToPos(p.hpf);
    v[P_EQ_LS_F] = hzToPos(p.lsFreq);
    v[P_EQ_LS_Q] = p.lsQ;
    v[P_EQ_LS_G] = p.lsGain;
    v[P_EQ_B1_F] = hzToPos(p.pkFreq0);
    v[P_EQ_B1_Q] = p.pkQ0;
    v[P_EQ_B1_G] = p.pkGain0;
    v[P_EQ_B2_F] = hzToPos(p.pkFreq1);
    v[P_EQ_B2_Q] = p.pkQ1;
    v[P_EQ_B2_G] = p.pkGain1;
    v[P_EQ_B3_F] = hzToPos(p.pkFreq2);
    v[P_EQ_B3_Q] = p.pkQ2;
    v[P_EQ_B3_G] = p.pkGain2;
    v[P_EQ_HS_F] = hzToPos(p.hsFreq);
    v[P_EQ_HS_Q] = p.hsQ;
    v[P_EQ_HS_G] = p.hsGain;
    v[P_EQ_LPF] = hzToPos(p.lpf);
    v[P_GR_DRIVE] = p.satDrive;
    v[P_GR_POS_IN] = p.posIn;
    v[P_GR_NEG_IN] = p.negIn;
    v[P_GR_POS_OUT] = p.posOut;
    v[P_GR_NEG_OUT] = p.negOut;
    v[P_GR_TRIM] = p.satTrim;
    v[P_GR_CUT] = hzToPos(p.satCut);
    v[P_SW_DELAY] = p.chDelayMs;
    v[P_SW_DEPTH] = 100 * p.chDepth;
    v[P_SW_WIDTH] = p.chStereoDeg;
    v[P_SW_RATE1] = p.chRate0;
    v[P_SW_RATE2] = p.chRate1;
    v[P_SW_RATE3] = p.chRate2;
    v[P_SW_SHAPE1] = floorf(clampf(p.chShape0, 0, 2));   // the core drops the fraction too
    v[P_SW_SHAPE2] = floorf(clampf(p.chShape1, 0, 2));
    v[P_SW_SHAPE3] = floorf(clampf(p.chShape2, 0, 2));
    v[P_SW_FILTER] = floorf(clampf(p.chFilter, 0, 2));
    v[P_SW_FREQ] = hzToPos(p.chFreq);
    v[P_SP_PRE] = p.vbPreMs;
    v[P_SP_SIZE] = 100 * p.vbSize;
    v[P_SP_DECAY] = 100 * p.vbDecay;
    v[P_SP_DENSITY] = 100 * p.vbDensity;
    v[P_SP_DAMP] = 100 * p.vbDamp;
    v[P_SP_LATE] = 100 * p.vbEarly;
    v[P_SP_TONE] = 100 * p.vbTone;
    v[P_LEVEL] = p.outDb;
}

Controls toControls(const float *v) {
    Controls c;
    c.overall = v[P_DRIZZLE] * 0.01f;
    c.eqMix = v[P_TONE] * 0.01f;
    c.satMix = v[P_GRIT] * 0.01f;
    c.chMix = v[P_SWIRL] * 0.01f;
    c.vbMix = v[P_SPACE] * 0.01f;
    c.dlTimeMs = v[P_DLY_TIME];
    c.dlFeedback = v[P_DLY_FB] * 0.01f;
    c.dlMix = v[P_DLY_MIX] * 0.01f;
    c.airRate = v[P_AIR_RATE];
    c.airFreq = posToHz(v[P_AIR_FREQ]);
    c.airMix = v[P_AIR_MIX] * 0.01f;
    c.hpf = posToHz(v[P_EQ_HPF]);
    c.lsFreq = posToHz(v[P_EQ_LS_F]);
    c.lsQ = v[P_EQ_LS_Q];
    c.lsGain = v[P_EQ_LS_G];
    for (int k = 0; k < 3; k++) {
        c.pkFreq[k] = posToHz(v[P_EQ_B1_F + 3 * k]);
        c.pkQ[k] = v[P_EQ_B1_Q + 3 * k];
        c.pkGain[k] = v[P_EQ_B1_G + 3 * k];
    }
    c.hsFreq = posToHz(v[P_EQ_HS_F]);
    c.hsQ = v[P_EQ_HS_Q];
    c.hsGain = v[P_EQ_HS_G];
    c.lpf = posToHz(v[P_EQ_LPF]);
    c.satDrive = v[P_GR_DRIVE];
    c.posIn = v[P_GR_POS_IN];
    c.negIn = v[P_GR_NEG_IN];
    c.posOut = v[P_GR_POS_OUT];
    c.negOut = v[P_GR_NEG_OUT];
    c.satTrim = v[P_GR_TRIM];
    c.satCut = posToHz(v[P_GR_CUT]);
    c.chDelayMs = v[P_SW_DELAY];
    c.chDepth = v[P_SW_DEPTH] * 0.01f;
    c.chStereoDeg = v[P_SW_WIDTH];
    for (int k = 0; k < 3; k++) {
        c.chRate[k] = v[P_SW_RATE1 + k];
        c.chShape[k] = v[P_SW_SHAPE1 + k];
    }
    c.chFilter = v[P_SW_FILTER];
    c.chFreq = posToHz(v[P_SW_FREQ]);
    c.vbPreMs = v[P_SP_PRE];
    c.vbSize = v[P_SP_SIZE] * 0.01f;
    c.vbDecay = v[P_SP_DECAY] * 0.01f;
    c.vbDensity = v[P_SP_DENSITY] * 0.01f;
    c.vbDamp = v[P_SP_DAMP] * 0.01f;
    c.vbEarly = v[P_SP_LATE] * 0.01f;
    c.vbTone = v[P_SP_TONE] * 0.01f;
    c.outDb = v[P_LEVEL];
    return c;
}

struct Engine {
    Core core;
    float target[NSTORED];      // what the host set, in stored units
    float now[NSTORED];         // smoothed
    bool moved[NSTORED];        // for mpc_engine_changed: a preset or style moved it
    bool anyMoved;
    bool dirty;                 // the core needs the controls again
    int preset;
    int style[NSTYLES];
};

int keyIndex(const char *k) {
    for (int i = 0; i < NSTORED; i++)
        if (!strcmp(k, DEFS[i].key)) return i;
    return -1;
}

int styleIndex(const char *k) {
    for (int s = 0; s < NSTYLES; s++)
        if (!strcmp(k, STYLE_KEYS[s])) return s;
    return -1;
}

void moveTo(Engine *e, int i, float v) {
    e->target[i] = v;
    e->moved[i] = true;
    e->anyMoved = true;
    e->dirty = true;
    if (DEFS[i].kind == 'o') e->now[i] = v;
}

void applyPreset(Engine *e, int n) {
    if (n < 0 || n >= kNumPresets) return;
    float v[NSTORED];
    presetValues(kPresets[n], v);
    for (int i = 0; i < NSTORED; i++) moveTo(e, i, v[i]);
    e->preset = n;
    for (int s = 0; s < NSTYLES; s++) e->style[s] = n;
}

void applyStyle(Engine *e, int s, int n) {
    if (s < 0 || s >= NSTYLES || n < 0 || n >= kNumPresets) return;
    float v[NSTORED];
    presetValues(kPresets[n], v);
    for (int i = STYLE_FIRST[s]; i <= STYLE_LAST[s]; i++) moveTo(e, i, v[i]);
    e->style[s] = n;
}

void *create(const char *) {
    Engine *e = new Engine();
    memset(e->moved, 0, sizeof e->moved);
    applyPreset(e, 0);
    memset(e->moved, 0, sizeof e->moved);
    e->anyMoved = false;
    memcpy(e->now, e->target, sizeof e->now);
    e->core.configure(toControls(e->now));
    e->dirty = false;
    return e;
}

void destroy(void *inst) { delete (Engine *)inst; }

void midi(void *, const uint8_t *, int) {}

void setParam(void *inst, const char *key, const char *val) {
    Engine *e = (Engine *)inst;
    if (!strcmp(key, "preset")) {
        applyPreset(e, atoi(val));
        return;
    }
    int s = styleIndex(key);
    if (s >= 0) {
        applyStyle(e, s, atoi(val));
        return;
    }
    if (!strcmp(key, "state")) {    // "key=value;..." from get_param("state"); unknown keys are skipped
        static char buf[4096];
        char *save = NULL;
        snprintf(buf, sizeof buf, "%s", val);
        e->target[P_LEVEL] = 0;    // a project saved before LEVEL existed plays as it did then
        for (char *t = strtok_r(buf, ";", &save); t; t = strtok_r(NULL, ";", &save)) {
            char *eq = strchr(t, '=');
            if (!eq) continue;
            *eq = 0;
            int i = keyIndex(t);
            if (i >= 0) {
                e->target[i] = (float)atof(eq + 1);
                if (DEFS[i].kind == 'o') e->now[i] = e->target[i];
            } else if (!strcmp(t, "preset")) {
                e->preset = atoi(eq + 1);    // only the names: the values came with the rest
            } else if ((s = styleIndex(t)) >= 0) {
                e->style[s] = atoi(eq + 1);
            }
        }
        e->dirty = true;
        return;
    }
    int i = keyIndex(key);
    if (i < 0) return;
    e->target[i] = (float)atof(val);
    if (DEFS[i].kind == 'o') e->now[i] = e->target[i];
    e->dirty = true;
}

int display(int i, float v, char *buf, int len) {
    switch (DEFS[i].kind) {
    case 'p': return snprintf(buf, len, "%.0f%%", v);
    case 'b': return snprintf(buf, len, "%+.0f%%", v);
    case 'f': {
        float hz = posToHz(v);
        return hz < 1000.0f ? snprintf(buf, len, "%.0f Hz", hz) : snprintf(buf, len, "%.2f kHz", hz * 0.001f);
    }
    case 'd': return snprintf(buf, len, "%+.1f dB", v);
    case 'q': return snprintf(buf, len, "Q %.2f", v);
    case 'm': return i == P_SW_DELAY ? snprintf(buf, len, "%.1f ms", v) : snprintf(buf, len, "%.0f ms", v);
    case 'h': return snprintf(buf, len, "%.1f Hz", v);
    case 'w': return snprintf(buf, len, "%.0f", v);
    }
    return snprintf(buf, len, "%.0f", v);
}

int getParam(void *inst, const char *key, char *buf, int len) {
    Engine *e = (Engine *)inst;
    if (!strcmp(key, "state")) {
        int n = 0;
        for (int i = 0; i < NSTORED && n < len; i++) n += snprintf(buf + n, len - n, "%s=%.4f;", DEFS[i].key, e->target[i]);
        for (int s = 0; s < NSTYLES && n < len; s++) n += snprintf(buf + n, len - n, "%s=%d;", STYLE_KEYS[s], e->style[s]);
        if (n < len) n += snprintf(buf + n, len - n, "preset=%d", e->preset);
        return n < len ? n : 0;
    }
    if (!strcmp(key, "preset")) return snprintf(buf, len, "%d", e->preset);
    if (!strncmp(key, "preset:", 7)) {         // a preset's name without loading it (the PRESET menu)
        int n = atoi(key + 7);
        return n >= 0 && n < kNumPresets ? snprintf(buf, len, "%s", kPresets[n].name) : 0;
    }
    int s = styleIndex(key);
    if (s >= 0) return snprintf(buf, len, "%d", e->style[s]);
    const char *suffix = strstr(key, "_display");
    if (suffix && suffix[8] == 0) {
        char k[64];
        int kl = (int)(suffix - key);
        if (kl <= 0 || kl >= (int)sizeof k) return 0;
        memcpy(k, key, kl);
        k[kl] = 0;
        int i = keyIndex(k);
        return i >= 0 ? display(i, e->target[i], buf, len) : 0;
    }
    int i = keyIndex(key);
    if (i < 0) return 0;
    if (DEFS[i].kind == 'o') return snprintf(buf, len, "%d", (int)(e->target[i] + 0.5f));
    return snprintf(buf, len, "%.4f", e->target[i]);
}

void render(void *, int16_t *out, int frames) { memset(out, 0, (size_t)frames * 2 * sizeof(int16_t)); }

// Reverb and echo tails decay towards denormals, which scalar VFP code can make slow: flush them to zero while this
// plugin runs, and give the host thread its own setting back afterwards.
struct FlushDenormals {
#if defined(__arm__)
    unsigned saved;
    FlushDenormals() {
        __asm__ volatile("vmrs %0, fpscr" : "=r"(saved));
        __asm__ volatile("vmsr fpscr, %0" : : "r"(saved | (1u << 24)));
    }
    ~FlushDenormals() { __asm__ volatile("vmsr fpscr, %0" : : "r"(saved)); }
#endif
};

void process(void *inst, const int16_t *in, int16_t *out, int frames) {
    Engine *e = (Engine *)inst;
    FlushDenormals ftz;
    (void)ftz;
    float l[kControlInterval], r[kControlInterval];
    for (int done = 0; done < frames; done += kControlInterval) {
        const int n = frames - done < kControlInterval ? frames - done : kControlInterval;
        for (int i = 0; i < n; i++) {
            l[i] = in[2 * (done + i)] * (1.0f / 32768.0f);
            r[i] = in[2 * (done + i) + 1] * (1.0f / 32768.0f);
        }
        if (e->dirty) {
            bool settling = false;
            for (int p = 0; p < NSTORED; p++) {
                float d = e->target[p] - e->now[p];
                if (d == 0) continue;
                if (fabsf(d) < 1e-3f) e->now[p] = e->target[p];
                else {
                    e->now[p] += d * SMOOTH_K;
                    settling = true;
                }
            }
            e->core.configure(toControls(e->now));
            e->dirty = settling;
        }
        e->core.process(l, r, n);
        for (int i = 0; i < n; i++) {
            out[2 * (done + i)] = (int16_t)lrintf(clampf(l[i], -1.0f, 0.99997f) * 32768.0f);
            out[2 * (done + i) + 1] = (int16_t)lrintf(clampf(r[i], -1.0f, 0.99997f) * 32768.0f);
        }
    }
}

const mpc_engine_t API = {create, destroy, midi, setParam, getParam, render, process, NULL};

}  // namespace

extern "C" {

__attribute__((visibility("default"))) const mpc_engine_t *mpc_engine(void) { return &API; }

// A preset or style moves many controls at once: report them so MPC's screen follows.
int mpc_engine_changed(void *inst, char *key, int len) {
    Engine *e = (Engine *)inst;
    if (!e->anyMoved) return 0;
    for (int i = 0; i < NSTORED; i++) {
        if (e->moved[i]) {
            e->moved[i] = false;
            snprintf(key, len, "%s", DEFS[i].key);
            return 1;
        }
    }
    e->anyMoved = false;
    return 0;
}

// Test hooks (test/engine_test.cpp): the stored keys in VST order
int drizzle_key_count(void) { return NSTORED; }
const char *drizzle_key(int i) { return i >= 0 && i < NSTORED ? DEFS[i].key : ""; }

}  // extern "C"
