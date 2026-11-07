// Drizzle's 24 factory settings, written by tools/gen_presets.py (edit that, not this).
// Designed for Drizzle in the units of drizzle::Controls; none comes from another plugin.
#pragma once

namespace drizzle {

struct Preset {
    const char *name;
    float overall, eqMix, satMix, chMix, vbMix, dlMix, dlTimeMs, dlFeedback, airRate, airFreq, airMix, hpf, lsFreq, lsQ, lsGain, pkFreq0, pkQ0, pkGain0, pkFreq1, pkQ1, pkGain1, pkFreq2, pkQ2, pkGain2, hsFreq, hsQ, hsGain, lpf, satDrive, posIn, negIn, posOut, negOut, satTrim, satCut, chDelayMs, chDepth, chStereoDeg, chRate0, chRate1, chRate2, chShape0, chShape1, chShape2, chFilter, chFreq, vbPreMs, vbSize, vbDecay, vbDensity, vbDamp, vbEarly, vbTone, outDb;
};

const int kNumPresets = 24;

const Preset kPresets[kNumPresets] = {
    // The all-rounder: clean lows, a lift at presence and air, light grit, a little echo and plate.
    {"First Pour", 1.0f, 1.0f, 0.25f, 0.15f, 0.18f, 0.12f, 333.0f, 0.22f, 0.0f, 10000.0f, 0.25f, 85.0f, 200.0f, 0.7f, 0.0f, 320.0f, 1.3f, -2.5f, 2800.0f, 1.0f, 1.0f, 5200.0f, 1.2f, 1.0f, 11000.0f, 0.7f, 1.5f, 20000.0f, 4.0f, 0.0f, -2.0f, -10.0f, -8.0f, 0.0f, 15000.0f, 11.0f, 0.25f, 0.0f, 0.35f, 0.21f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 20.0f, 0.4f, 0.4f, 0.75f, 0.4f, 0.8f, 0.65f, 3.5f},
    // Smooth R&B: warm body, softened bite, a slow shimmer on top and a soft hall.
    {"Silk Ribbon", 1.0f, 1.0f, 0.2f, 0.25f, 0.26f, 0.14f, 500.0f, 0.3f, 0.12f, 12000.0f, 0.3f, 90.0f, 180.0f, 0.7f, 0.5f, 380.0f, 1.2f, -2.0f, 3200.0f, 1.6f, -1.0f, 7000.0f, 1.0f, 1.0f, 12000.0f, 0.7f, 2.0f, 19000.0f, 2.0f, 0.0f, -1.0f, -8.0f, -7.0f, 0.0f, 12000.0f, 14.0f, 0.2f, 0.0f, 0.22f, 0.13f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 300.0f, 30.0f, 0.6f, 0.55f, 0.75f, 0.55f, 0.85f, 0.55f, 6.0f},
    // Dry and in your face for rap: mud and honk out, presence up, grit, a slap and a small room.
    {"Front Row", 1.0f, 1.0f, 0.35f, 0.0f, 0.1f, 0.1f, 110.0f, 0.05f, 0.0f, 9000.0f, 0.2f, 100.0f, 200.0f, 0.7f, 0.0f, 280.0f, 1.4f, -3.0f, 1100.0f, 1.4f, -1.0f, 3500.0f, 1.0f, 2.5f, 9000.0f, 0.7f, 1.5f, 20000.0f, 6.0f, 0.0f, -3.0f, -12.0f, -9.0f, 0.0f, 14000.0f, 10.0f, 0.25f, 0.0f, 0.35f, 0.21f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 5.0f, 0.15f, 0.3f, 0.75f, 0.5f, 0.35f, 0.55f, 3.0f},
    // Lush pop: a wide three-LFO chorus, quarter-note echo and a big soft hall.
    {"Velvet", 1.0f, 1.0f, 0.15f, 0.3f, 0.3f, 0.18f, 667.0f, 0.35f, 0.08f, 11000.0f, 0.3f, 90.0f, 200.0f, 0.7f, 0.0f, 300.0f, 1.0f, -2.0f, 1800.0f, 1.0f, 0.0f, 4000.0f, 0.9f, 1.5f, 12000.0f, 0.7f, 2.5f, 20000.0f, 2.0f, 0.0f, -2.0f, -8.0f, -6.0f, 0.0f, 16000.0f, 12.0f, 0.35f, 0.0f, 0.45f, 0.27f, 0.08f, 0.0f, 2.0f, 0.0f, 1.0f, 150.0f, 35.0f, 0.75f, 0.6f, 0.75f, 0.5f, 0.9f, 0.6f, 6.5f},
    // Warm soul: thick lows, rounded top, tape-style grit and a dark plate.
    {"Honey Glaze", 1.0f, 1.0f, 0.45f, 0.18f, 0.2f, 0.12f, 333.0f, 0.2f, 0.0f, 10000.0f, 0.0f, 75.0f, 160.0f, 0.7f, 1.0f, 400.0f, 1.2f, -1.5f, 2500.0f, 1.0f, 1.0f, 6000.0f, 1.0f, -1.0f, 10000.0f, 0.7f, -1.0f, 14000.0f, 5.0f, 0.0f, -2.0f, -11.0f, -9.0f, 0.0f, 9000.0f, 7.0f, 0.2f, 0.0f, 0.6f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 12.0f, 0.35f, 0.45f, 0.75f, 0.6f, 0.8f, 0.45f, 6.0f},
    // Bright airy pop: lifted presence and top, a moving air band and a clear room.
    {"Daylight", 1.0f, 1.0f, 0.15f, 0.2f, 0.2f, 0.12f, 500.0f, 0.25f, 0.25f, 12000.0f, 0.35f, 110.0f, 200.0f, 0.7f, 0.0f, 250.0f, 1.0f, -2.0f, 900.0f, 1.5f, -1.0f, 5000.0f, 0.8f, 2.0f, 9000.0f, 0.7f, 3.0f, 20000.0f, 2.0f, 0.0f, -2.0f, -8.0f, -6.0f, 0.0f, 16000.0f, 10.0f, 0.25f, 0.0f, 0.35f, 0.21f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 18.0f, 0.5f, 0.45f, 0.75f, 0.3f, 0.8f, 0.8f, 5.5f},
    // Close and intimate: no boom, soft sibilance, a breath of air and a small plate.
    {"Whisper", 1.0f, 1.0f, 0.15f, 0.12f, 0.14f, 0.0f, 333.0f, 0.25f, 0.0f, 13000.0f, 0.3f, 130.0f, 200.0f, 0.7f, 0.0f, 250.0f, 1.2f, -2.0f, 2000.0f, 1.0f, 0.0f, 6500.0f, 1.5f, -1.5f, 12000.0f, 0.7f, 2.0f, 20000.0f, 2.0f, 0.0f, -2.0f, -8.0f, -6.0f, 0.0f, 16000.0f, 9.0f, 0.2f, 0.0f, 0.35f, 0.21f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 10.0f, 0.3f, 0.4f, 0.75f, 0.45f, 0.8f, 0.55f, 2.5f},
    // Lo-fi record: thin and boxy, crunchy, a warbling chorus and a small dark room.
    {"Dusty Wax", 1.0f, 1.0f, 0.7f, 0.3f, 0.15f, 0.1f, 167.0f, 0.15f, 0.0f, 10000.0f, 0.0f, 220.0f, 200.0f, 0.7f, 0.0f, 300.0f, 1.2f, 0.0f, 1300.0f, 0.9f, 3.0f, 3500.0f, 1.0f, -2.0f, 10000.0f, 0.7f, 0.0f, 4800.0f, 8.0f, 0.0f, -4.0f, -14.0f, -10.0f, 0.0f, 6000.0f, 5.0f, 0.15f, 0.0f, 0.7f, 5.5f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 220.0f, 5.0f, 0.25f, 0.4f, 0.75f, 0.7f, 0.6f, 0.3f, 6.5f},
    // Telephone voice: a narrow mid band, driven hard, with a short slap.
    {"Phone Line", 1.0f, 1.0f, 0.6f, 0.0f, 0.08f, 0.08f, 90.0f, 0.0f, 0.0f, 10000.0f, 0.0f, 450.0f, 200.0f, 0.7f, 0.0f, 300.0f, 1.2f, 0.0f, 1500.0f, 1.2f, 5.0f, 4500.0f, 1.0f, 0.0f, 10000.0f, 0.7f, 0.0f, 3400.0f, 10.0f, 0.0f, -2.0f, -16.0f, -14.0f, 0.0f, 4500.0f, 10.0f, 0.25f, 0.0f, 0.35f, 0.21f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 0.0f, 0.1f, 0.2f, 0.75f, 0.45f, 0.2f, 0.4f, 3.0f},
    // Tape: warm and soft on top, saturated, with a gentle wow and flutter.
    {"Cassette", 1.0f, 1.0f, 0.55f, 0.3f, 0.1f, 0.0f, 333.0f, 0.25f, 0.0f, 10000.0f, 0.0f, 50.0f, 120.0f, 0.7f, 1.5f, 300.0f, 1.2f, 0.0f, 1800.0f, 1.0f, 0.0f, 4500.0f, 1.0f, 0.0f, 9000.0f, 0.7f, -2.0f, 13000.0f, 4.0f, 0.0f, -2.0f, -10.0f, -8.0f, 0.0f, 11000.0f, 5.0f, 0.12f, 0.0f, 0.5f, 7.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 220.0f, 18.0f, 0.25f, 0.35f, 0.75f, 0.6f, 0.8f, 0.45f, 5.0f},
    // Bullhorn: a hard mid band clipped hard, with an outdoor slap.
    {"Megaphone", 1.0f, 1.0f, 0.9f, 0.0f, 0.15f, 0.2f, 140.0f, 0.2f, 0.0f, 10000.0f, 0.0f, 600.0f, 200.0f, 0.7f, 0.0f, 300.0f, 1.2f, 0.0f, 1800.0f, 1.5f, 6.0f, 4500.0f, 1.0f, 0.0f, 10000.0f, 0.7f, 0.0f, 5000.0f, 14.0f, 0.0f, -2.0f, -20.0f, -18.0f, 0.0f, 4500.0f, 10.0f, 0.25f, 0.0f, 0.35f, 0.21f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 10.0f, 0.3f, 0.35f, 0.75f, 0.45f, 0.5f, 0.45f, 8.5f},
    // Aggressive: driven hard, forward mids and bite, a quick echo.
    {"Hot Sauce", 1.0f, 1.0f, 0.8f, 0.1f, 0.12f, 0.15f, 167.0f, 0.3f, 0.0f, 10000.0f, 0.0f, 120.0f, 200.0f, 0.7f, 0.0f, 300.0f, 1.2f, -3.0f, 1200.0f, 1.0f, 2.0f, 3800.0f, 1.0f, 1.5f, 10000.0f, 0.7f, 0.0f, 12000.0f, 9.0f, 0.0f, -4.0f, -15.0f, -11.0f, 0.0f, 7000.0f, 10.0f, 0.25f, 0.0f, 0.35f, 0.21f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 18.0f, 0.3f, 0.35f, 0.75f, 0.45f, 0.6f, 0.6f, 5.0f},
    // Gritty boom bap: low warmth, a rough top cut short, an eighth echo and a room.
    {"Gravel", 1.0f, 1.0f, 0.6f, 0.0f, 0.1f, 0.1f, 333.0f, 0.15f, 0.0f, 10000.0f, 0.0f, 100.0f, 150.0f, 0.7f, 0.5f, 400.0f, 1.2f, -2.0f, 1800.0f, 1.0f, 0.0f, 2800.0f, 1.0f, 1.0f, 10000.0f, 0.7f, 0.0f, 10000.0f, 8.0f, 0.0f, -3.0f, -14.0f, -11.0f, 0.0f, 8000.0f, 10.0f, 0.25f, 0.0f, 0.35f, 0.21f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 8.0f, 0.25f, 0.35f, 0.75f, 0.45f, 0.5f, 0.5f, 3.5f},
    // For samples and drum loops: weight, gentle tape grit, a soft top and a tight room.
    {"Crate Glue", 1.0f, 1.0f, 0.5f, 0.0f, 0.1f, 0.0f, 333.0f, 0.25f, 0.0f, 10000.0f, 0.15f, 30.0f, 90.0f, 0.7f, 2.0f, 350.0f, 1.2f, -1.5f, 1800.0f, 1.0f, 0.0f, 5000.0f, 1.0f, 1.0f, 10000.0f, 0.7f, 0.0f, 15000.0f, 5.0f, 0.0f, -2.0f, -11.0f, -9.0f, 0.0f, 12000.0f, 10.0f, 0.25f, 0.0f, 0.35f, 0.21f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 0.0f, 0.2f, 0.25f, 0.75f, 0.45f, 0.3f, 0.5f, 2.0f},
    // A huge, long, soft hall with a slow echo behind it.
    {"Cathedral", 1.0f, 1.0f, 0.1f, 0.15f, 0.4f, 0.1f, 667.0f, 0.3f, 0.05f, 11000.0f, 0.2f, 100.0f, 200.0f, 0.7f, 0.0f, 300.0f, 1.2f, -2.0f, 1800.0f, 1.0f, 0.0f, 4500.0f, 1.0f, 0.0f, 9000.0f, 0.7f, 2.5f, 20000.0f, 2.0f, 0.0f, -2.0f, -8.0f, -6.0f, 0.0f, 16000.0f, 13.0f, 0.25f, 0.0f, 0.35f, 0.21f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 45.0f, 0.95f, 0.72f, 0.85f, 0.55f, 0.95f, 0.5f, 4.0f},
    // Moody: a slow wide chorus, dotted echoes trailing off, a dark hall.
    {"Night Drive", 1.0f, 1.0f, 0.25f, 0.3f, 0.25f, 0.22f, 500.0f, 0.45f, 0.0f, 10000.0f, 0.15f, 100.0f, 200.0f, 0.7f, 0.0f, 350.0f, 1.2f, -2.0f, 1800.0f, 1.0f, 0.0f, 3000.0f, 1.0f, 1.5f, 10000.0f, 0.7f, 0.0f, 15000.0f, 3.0f, 0.0f, -2.0f, -9.0f, -7.0f, 0.0f, 16000.0f, 13.0f, 0.3f, 0.0f, 0.25f, 0.15f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 25.0f, 0.6f, 0.6f, 0.75f, 0.65f, 0.85f, 0.4f, 7.0f},
    // Eighth-note echoes that keep going, over a short room.
    {"Echo Alley", 1.0f, 1.0f, 0.2f, 0.0f, 0.15f, 0.3f, 333.0f, 0.55f, 0.0f, 10000.0f, 0.2f, 120.0f, 200.0f, 0.7f, 0.0f, 300.0f, 1.2f, -2.0f, 1800.0f, 1.0f, 0.0f, 3500.0f, 1.0f, 1.5f, 10000.0f, 0.7f, 0.0f, 20000.0f, 3.0f, 0.0f, -2.0f, -9.0f, -7.0f, 0.0f, 16000.0f, 10.0f, 0.25f, 0.0f, 0.35f, 0.21f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 18.0f, 0.35f, 0.4f, 0.75f, 0.45f, 0.7f, 0.6f, 4.5f},
    // Fifties slap: one quick repeat, some grit, a small room.
    {"Slapback", 1.0f, 1.0f, 0.4f, 0.0f, 0.12f, 0.28f, 115.0f, 0.08f, 0.0f, 10000.0f, 0.0f, 90.0f, 200.0f, 0.7f, 0.0f, 300.0f, 1.2f, -2.0f, 1800.0f, 1.0f, 0.0f, 3000.0f, 1.0f, 2.0f, 10000.0f, 0.7f, 0.0f, 16000.0f, 5.0f, 0.0f, -2.0f, -11.0f, -9.0f, 0.0f, 16000.0f, 10.0f, 0.25f, 0.0f, 0.35f, 0.21f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 5.0f, 0.2f, 0.35f, 0.75f, 0.45f, 0.4f, 0.6f, 3.5f},
    // Ethereal: thin lows, bright air, a wide slow chorus, long echoes and a bright hall.
    {"Halo", 1.0f, 1.0f, 0.0f, 0.3f, 0.38f, 0.15f, 667.0f, 0.4f, 0.15f, 11000.0f, 0.4f, 140.0f, 200.0f, 0.7f, 0.0f, 300.0f, 1.2f, -3.0f, 1800.0f, 1.0f, 0.0f, 4500.0f, 1.0f, 0.0f, 8000.0f, 0.7f, 3.0f, 20000.0f, 3.0f, 0.0f, -2.0f, -9.0f, -7.0f, 0.0f, 16000.0f, 15.0f, 0.3f, 0.0f, 0.18f, 0.11f, 0.05f, 0.0f, 2.0f, 0.0f, 1.0f, 200.0f, 40.0f, 0.8f, 0.7f, 0.75f, 0.3f, 0.9f, 0.8f, 6.0f},
    // A dark, frozen wash: a near-endless hall with long echoes inside it.
    {"Deep Freeze", 1.0f, 1.0f, 0.0f, 0.2f, 0.35f, 0.12f, 1000.0f, 0.5f, 0.0f, 10000.0f, 0.0f, 150.0f, 200.0f, 0.7f, 0.0f, 300.0f, 1.2f, -2.0f, 1800.0f, 1.0f, 0.0f, 4500.0f, 1.0f, 0.0f, 10000.0f, 0.7f, 0.0f, 12000.0f, 3.0f, 0.0f, -2.0f, -9.0f, -7.0f, 0.0f, 16000.0f, 14.0f, 0.3f, 0.0f, 0.15f, 0.09f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 60.0f, 1.0f, 0.88f, 0.9f, 0.7f, 1.0f, 0.35f, 5.5f},
    // A second take beside the first: a slow, slightly late copy, spread wide.
    {"Doubler", 1.0f, 1.0f, 0.15f, 0.45f, 0.1f, 0.0f, 333.0f, 0.25f, 0.0f, 11000.0f, 0.2f, 90.0f, 200.0f, 0.7f, 0.0f, 300.0f, 1.2f, -1.5f, 1800.0f, 1.0f, 0.0f, 4500.0f, 1.0f, 0.0f, 10000.0f, 0.7f, 0.0f, 20000.0f, 2.0f, 0.0f, -2.0f, -8.0f, -6.0f, 0.0f, 16000.0f, 22.0f, 0.1f, 0.0f, 0.25f, 0.15f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 180.0f, 18.0f, 0.25f, 0.35f, 0.75f, 0.45f, 0.6f, 0.6f, 5.0f},
    // Wide: a broad chorus with the lows kept in the middle, eighth echoes and a medium hall.
    {"Wide Screen", 1.0f, 1.0f, 0.15f, 0.4f, 0.22f, 0.14f, 333.0f, 0.3f, 0.0f, 11000.0f, 0.25f, 90.0f, 200.0f, 0.7f, 0.0f, 300.0f, 1.2f, -2.0f, 1800.0f, 1.0f, 0.0f, 4500.0f, 1.0f, 0.0f, 11000.0f, 0.7f, 1.5f, 20000.0f, 2.0f, 0.0f, -2.0f, -8.0f, -6.0f, 0.0f, 16000.0f, 16.0f, 0.2f, 0.0f, 0.4f, 0.6f, 0.1f, 0.0f, 2.0f, 0.0f, 1.0f, 150.0f, 18.0f, 0.55f, 0.5f, 0.75f, 0.45f, 0.8f, 0.6f, 6.0f},
    // An effect: deep vibrato on the whole voice, in a room.
    {"Seasick", 1.0f, 1.0f, 0.0f, 0.75f, 0.2f, 0.0f, 333.0f, 0.25f, 0.0f, 10000.0f, 0.0f, 100.0f, 200.0f, 0.7f, 0.0f, 300.0f, 1.2f, -2.0f, 1800.0f, 1.0f, 0.0f, 4500.0f, 1.0f, 0.0f, 10000.0f, 0.7f, 0.0f, 20000.0f, 3.0f, 0.0f, -2.0f, -9.0f, -7.0f, 0.0f, 16000.0f, 6.0f, 0.5f, 0.0f, 4.5f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 220.0f, 18.0f, 0.4f, 0.45f, 0.75f, 0.45f, 0.8f, 0.6f, 3.0f},
    // An effect: the top end pulses quickly, like a shimmering light.
    {"Flicker", 1.0f, 1.0f, 0.15f, 0.0f, 0.2f, 0.12f, 333.0f, 0.25f, 5.0f, 5000.0f, 0.7f, 100.0f, 200.0f, 0.7f, 0.0f, 300.0f, 1.2f, -2.0f, 1800.0f, 1.0f, 0.0f, 4500.0f, 1.0f, 0.0f, 10000.0f, 0.7f, 0.0f, 20000.0f, 2.0f, 0.0f, -2.0f, -8.0f, -6.0f, 0.0f, 16000.0f, 10.0f, 0.25f, 0.0f, 0.35f, 0.21f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, 220.0f, 18.0f, 0.45f, 0.45f, 0.75f, 0.45f, 0.8f, 0.6f, 2.5f},
};

}  // namespace drizzle
