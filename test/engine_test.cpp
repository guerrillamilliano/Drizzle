// Checks of the MPC adapter that the toolkit's host test does not reach: the key list against params.json, presets and
// styles reporting their moves, state round trips, value text, a clean bypass at Drizzle 0, and every preset staying
// finite and inside 16 bits. Built with src/drizzle_engine.cpp (see tools/test.sh), run from ports/drizzle.
extern "C" {
#include "engine.h"
int drizzle_key_count(void);
const char *drizzle_key(int i);
}
#include "DrizzlePresets.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

static int failures = 0;
static void check(int ok, const char *what) {
    printf("%-4s %s\n", ok ? "ok" : "FAIL", what);
    if (!ok) failures++;
}

static float value(const mpc_engine_t *api, void *e, const char *key) {
    char b[64] = "";
    api->get_param(e, key, b, sizeof b);
    return (float)atof(b);
}

static std::string text(const mpc_engine_t *api, void *e, const char *key) {
    char b[128] = "";
    api->get_param(e, key, b, sizeof b);
    return b;
}

static int drain(void *e, std::vector<std::string> *keys = NULL) {
    char key[64];
    int n = 0;
    while (mpc_engine_changed(e, key, sizeof key) > 0) {
        if (keys) keys->push_back(key);
        n++;
    }
    return n;
}

int main() {
    const mpc_engine_t *api = mpc_engine();
    void *e = api->create(NULL);

    // the engine's keys, in order, are params.json's minus the option pickers
    FILE *f = fopen("params.json", "rb");
    std::vector<std::string> jsonKeys;
    if (f) {
        static char buf[1 << 16];
        size_t n = fread(buf, 1, sizeof buf - 1, f);
        buf[n] = 0;
        fclose(f);
        for (char *p = buf; (p = strstr(p, "\"key\": \"")) != NULL;) {
            p += 8;
            char *q = strchr(p, '"');
            std::string k(p, q - p);
            if (k.compare(0, 6, "preset") != 0 && k.find("_style") == std::string::npos) jsonKeys.push_back(k);
            p = q;
        }
    }
    bool same = (int)jsonKeys.size() == drizzle_key_count();
    for (int i = 0; same && i < drizzle_key_count(); i++) same = jsonKeys[i] == drizzle_key(i);
    check(same, "engine keys match params.json, in order");

    check(drain(e) == 0, "a new instance reports nothing");
    check(fabsf(value(api, e, "tone") - 100 * drizzle::kPresets[0].eqMix) < 0.01f, "default sound is the first preset");

    api->set_param(e, "preset", "8");
    check(drain(e) == drizzle_key_count(), "a preset reports every control to the host");
    check(value(api, e, "preset") == 8.0f, "the preset is remembered");
    check(value(api, e, "tone_style") == 8.0f, "a preset sets the four style names with it");

    float toneMix = value(api, e, "tone");
    std::vector<std::string> moved;
    api->set_param(e, "tone_style", "3");
    drain(e, &moved);
    bool onlyEq = !moved.empty();
    for (size_t i = 0; i < moved.size(); i++) onlyEq = onlyEq && moved[i].compare(0, 3, "eq_") == 0;
    check(onlyEq && moved.size() == 17, "a tone style moves the 17 EQ controls and nothing else");
    check(value(api, e, "tone") == toneMix, "a style leaves its section's amount alone");
    check(value(api, e, "tone_style") == 3.0f, "the style is remembered");

    api->set_param(e, "sw_rate2", "7.5");
    char st[8192];
    check(api->get_param(e, "state", st, sizeof st) > 0, "state saves");
    void *b = api->create(NULL);
    api->set_param(b, "state", st);
    check(fabsf(value(api, b, "sw_rate2") - 7.5f) < 0.001f, "state keeps a tweak (no preset is applied over it)");
    check(value(api, b, "preset") == 8.0f && value(api, b, "tone_style") == 3.0f, "state keeps the preset and style names");
    check(drain(b) == 0, "restoring state reports nothing");
    check(fabsf(value(api, b, "level") - drizzle::kPresets[8].outDb) < 0.001f, "state keeps the preset's LEVEL");
    void *old = api->create(NULL);
    api->set_param(old, "state", "drizzle=100.0000;tone=50.0000;preset=2");
    check(value(api, old, "level") == 0.0f, "a project saved before LEVEL existed opens at 0 dB");
    api->destroy(old);

    api->set_param(e, "air_freq", "100");
    check(text(api, e, "air_freq_display") == "20.00 kHz", "a frequency knob at the top reads 20.00 kHz");
    api->set_param(e, "air_freq", "0");
    check(text(api, e, "air_freq_display") == "20 Hz", "and at the bottom 20 Hz");
    api->set_param(e, "eq_b1_g", "3.6");
    check(text(api, e, "eq_b1_g_display") == "+3.6 dB", "gains read in dB with a sign");
    check(text(api, e, "preset:23") == drizzle::kPresets[23].name, "the PRESET menu gets each name");

    // Drizzle at 0 passes the input untouched
    void *d = api->create(NULL);
    api->set_param(d, "drizzle", "0");
    int16_t in[256], out[256];
    int worst = 0;
    double ph = 0;
    for (int k = 0; k < 400; k++) {
        for (int i = 0; i < 128; i++) {
            ph += 220.0 / 44100.0;
            in[2 * i] = (int16_t)(12000 * sin(2 * M_PI * ph));
            in[2 * i + 1] = (int16_t)(9000 * sin(2 * M_PI * ph * 1.5));
        }
        api->process(d, in, out, 128);
        if (k > 100)
            for (int i = 0; i < 256; i++) worst = abs(out[i] - in[i]) > worst ? abs(out[i] - in[i]) : worst;
    }
    check(worst <= 1, "Drizzle at 0 is the dry signal (within one step)");

    // every preset on a loud phrase: finite, inside 16 bits, and quiet again after the tail
    bool allOk = true;
    for (int p = 0; p < 24; p++) {
        void *x = api->create(NULL);
        char v[16];
        snprintf(v, sizeof v, "%d", p);
        api->set_param(x, "preset", v);
        drain(x);
        int16_t o[256];
        long long loud = 0, tail = 0;
        for (int k = 0; k < 1400; k++) {
            for (int i = 0; i < 128; i++) {
                ph += 330.0 / 44100.0;
                double a = k < 400 ? 0.7 * sin(2 * M_PI * ph) * (1 + 0.3 * sin(2 * M_PI * ph * 0.01)) : 0.0;
                in[2 * i] = in[2 * i + 1] = (int16_t)(32767 * a);
            }
            api->process(x, in, o, 128);
            for (int i = 0; i < 256; i++) {
                if (k < 400) loud += (long long)o[i] * o[i];
                if (k >= 1300) tail += (long long)o[i] * o[i];
            }
        }
        bool infinite = false;   // presets whose echo or room is meant to hang on (none at present)
        if (loud == 0 || (!infinite && tail > loud / 50)) {
            printf("     preset %d: loud %lld tail %lld\n", p, loud, tail);
            allOk = false;
        }
        api->destroy(x);
    }
    check(allOk, "every preset makes sound and dies away (allowing for long tails)");

    api->destroy(e);
    api->destroy(b);
    api->destroy(d);
    printf("%s\n", failures ? "FAILED" : "PASSED");
    return failures ? 1 : 0;
}
