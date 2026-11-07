// Renders every factory preset in a presets header through Drizzle's core, for tools/probe/audition.py.
//   g++ -O2 -std=gnu++11 -I../../dsp -include <presets.h> audition.cpp -o audition
//   ./audition <in.f32> <outdir> <tag>      -> <outdir>/<tag>NN.f32 (stereo float, input plus 6 s of tail)
#include "DrizzleDsp.h"

#include <stdio.h>
#include <vector>

using namespace drizzle;

Controls fromPreset(const Preset &p) {
    Controls c;
    c.overall = p.overall; c.eqMix = p.eqMix; c.satMix = p.satMix; c.chMix = p.chMix; c.vbMix = p.vbMix;
    c.dlMix = p.dlMix; c.dlTimeMs = p.dlTimeMs; c.dlFeedback = p.dlFeedback;
    c.airRate = p.airRate; c.airFreq = p.airFreq; c.airMix = p.airMix;
    c.hpf = p.hpf; c.lsFreq = p.lsFreq; c.lsQ = p.lsQ; c.lsGain = p.lsGain;
    c.pkFreq[0] = p.pkFreq0; c.pkQ[0] = p.pkQ0; c.pkGain[0] = p.pkGain0;
    c.pkFreq[1] = p.pkFreq1; c.pkQ[1] = p.pkQ1; c.pkGain[1] = p.pkGain1;
    c.pkFreq[2] = p.pkFreq2; c.pkQ[2] = p.pkQ2; c.pkGain[2] = p.pkGain2;
    c.hsFreq = p.hsFreq; c.hsQ = p.hsQ; c.hsGain = p.hsGain; c.lpf = p.lpf;
    c.satDrive = p.satDrive; c.posIn = p.posIn; c.negIn = p.negIn; c.posOut = p.posOut; c.negOut = p.negOut;
    c.satTrim = p.satTrim; c.satCut = p.satCut;
    c.chDelayMs = p.chDelayMs; c.chDepth = p.chDepth; c.chStereoDeg = p.chStereoDeg;
    c.chRate[0] = p.chRate0; c.chRate[1] = p.chRate1; c.chRate[2] = p.chRate2;
    c.chShape[0] = p.chShape0; c.chShape[1] = p.chShape1; c.chShape[2] = p.chShape2;
    c.chFilter = p.chFilter; c.chFreq = p.chFreq;
    c.vbPreMs = p.vbPreMs; c.vbSize = p.vbSize; c.vbDecay = p.vbDecay; c.vbDensity = p.vbDensity;
    c.vbDamp = p.vbDamp; c.vbEarly = p.vbEarly; c.vbTone = p.vbTone;
#ifdef HAS_LEVEL
    c.outDb = p.outDb;
#endif
    return c;
}

int main(int argc, char **argv) {
    if (argc < 4) return 2;
    FILE *fi = fopen(argv[1], "rb");
    if (!fi) return 1;
    std::vector<float> in;
    float s;
    while (fread(&s, sizeof s, 1, fi) == 1) in.push_back(s);
    fclose(fi);
    in.resize(in.size() + 2 * 6 * 44100, 0.0f);
    size_t frames = in.size() / 2;
    for (int p = 0; p < kNumPresets; p++) {
        static Core cores[64];
        Core &core = cores[p];
        core.configure(fromPreset(kPresets[p]));
        std::vector<float> out(in);
        float L[kControlInterval], R[kControlInterval];
        for (size_t at = 0; at < frames; at += kControlInterval) {
            int n = (int)(frames - at < (size_t)kControlInterval ? frames - at : kControlInterval);
            for (int k = 0; k < n; k++) { L[k] = out[2 * (at + k)]; R[k] = out[2 * (at + k) + 1]; }
            core.process(L, R, n);
            for (int k = 0; k < n; k++) { out[2 * (at + k)] = L[k]; out[2 * (at + k) + 1] = R[k]; }
        }
        char name[512];
        snprintf(name, sizeof name, "%s/%s%02d.f32", argv[2], argv[3], p);
        FILE *fo = fopen(name, "wb");
        fwrite(out.data(), sizeof(float), out.size(), fo);
        fclose(fo);
        printf("%s%02d %s\n", argv[3], p, kPresets[p].name);
    }
    return 0;
}
