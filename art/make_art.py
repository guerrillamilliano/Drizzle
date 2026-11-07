#!/usr/bin/env python3
"""Drizzle's skin: writes the artwork in art/ and layout.conf. Edit this, not its outputs.

A plate fading from black to grey with brushed-steel knobs. Everything is drawn here (no photos, no other product's art). Needs numpy, scipy and Pillow, from ports/drizzle:
  docker run --rm -v <port>:/p -w /p python:3.11-slim sh -c "pip install -q numpy scipy pillow; python3 art/make_art.py"
"""
import math
import os

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageFont
from scipy import ndimage

HERE = os.path.dirname(os.path.abspath(__file__))
PORT = os.path.join(HERE, "..")
FONTS = os.path.join(PORT, "..", "..", "tools", "html_art", "fonts")
W, H = 1280, 628           # the plugin area; layout y = art y + 86
Y0 = 86

TOP = np.array([0.012, 0.012, 0.014])      # black at the top ...
BOTTOM = np.array([0.30, 0.30, 0.31])      # ... fading to grey at the bottom
LIGHT = np.array([-0.45, -0.60, 0.66])     # from the upper left, toward the viewer
LIGHT = LIGHT / np.linalg.norm(LIGHT)
rng = np.random.default_rng(73)


def plate(w, h):
    y = np.linspace(0, 1, h)[:, None, None] ** 1.15
    img = TOP * (1 - y) + BOTTOM * y + np.zeros((h, w, 3))
    img += rng.normal(0, 0.005, (h, w, 1))          # a faint grain, so the gradient doesn't band
    return img


def panel(img, x0, y0, x1, y1, lift=0.03, rad=22):
    """A slightly lifted, rounded well behind a group of controls, with a soft light rim."""
    h, w = img.shape[:2]
    m = Image.new("L", (w, h), 0)
    ImageDraw.Draw(m).rounded_rectangle((x0, y0, x1, y1), rad, fill=255)
    m = np.asarray(m.filter(ImageFilter.GaussianBlur(2)), np.float64) / 255.0
    rim = Image.new("L", (w, h), 0)
    ImageDraw.Draw(rim).rounded_rectangle((x0, y0, x1, y1), rad, outline=255, width=2)
    rim = np.asarray(rim.filter(ImageFilter.GaussianBlur(1)), np.float64) / 255.0
    img += lift * m[..., None]
    img += 0.06 * rim[..., None]
    return img


def save(img, name):
    Image.fromarray((np.clip(img, 0, 1) * 255 + 0.5).astype(np.uint8), "RGB").save(os.path.join(HERE, name), optimize=True)


def steel_knob():
    """Brushed steel. knob_base.png (still): the cap with concentric brushing, the light catching it in two broad
    bands, a bevelled rim and a drop shadow. knob.png (turns): only the pointer groove, so the lighting stays put."""
    N = 480                      # drawn at 2x of the 240 px images
    c = N / 2
    yy, xx = np.mgrid[0:N, 0:N] + 0.5
    dx, dy = xx - c, yy - c
    rr = np.sqrt(dx * dx + dy * dy)
    ang = np.arctan2(dy, dx)
    R = N * 0.40                 # cap radius
    # concentric brushing: noise that varies with radius only, plus a little along the circle
    rad_noise = rng.normal(0, 1, int(R) + 8)
    rad_noise = ndimage.gaussian_filter1d(rad_noise, 0.8)
    fine = rad_noise[np.clip(rr.astype(int), 0, len(rad_noise) - 1)]
    fine += 0.25 * ndimage.gaussian_filter(rng.normal(0, 1, (N, N)), 0.7)
    # anisotropic reflection: brushed metal lights up in bands across the light direction
    light_ang = math.atan2(-0.6, -0.45)
    band = 0.5 + 0.5 * np.cos(2 * (ang - light_ang))
    steel = 0.50 + 0.30 * band ** 3 + 0.035 * fine
    steel *= 0.92 + 0.08 * (1 - rr / R)                         # a touch brighter toward the centre
    cap = np.clip((R - rr) * 1.0, 0, 1)
    # rim: a bevel ring, light on the upper left, dark on the lower right
    bevel_w = N * 0.035
    ring = np.clip((rr - (R - bevel_w)) / bevel_w, 0, 1) * cap
    facing = np.clip(-(dx * LIGHT[0] + dy * LIGHT[1]) / np.maximum(rr, 1), -1, 1)
    steel = steel * (1 - ring) + ring * (0.45 + 0.35 * facing)
    # a dark skirt below the cap and a soft shadow on the plate
    skirt_R = N * 0.455
    skirt = np.clip((skirt_R - rr), 0, 1) * (1 - cap)
    skirt_col = 0.12 + 0.10 * np.clip(-(dx * LIGHT[0] + dy * LIGHT[1]) / np.maximum(rr, 1), 0, 1)
    sh = np.clip(1 - np.sqrt((dx - N * 0.02) ** 2 + (dy - N * 0.035) ** 2) / (N * 0.49), 0, 1) ** 1.5 * 0.65
    rgb = np.zeros((N, N, 3))
    tint = np.array([0.98, 1.0, 1.04])                           # steel leans very slightly blue
    rgb += (steel * cap)[..., None] * tint
    rgb += (skirt_col * skirt)[..., None]
    alpha = np.clip(cap + skirt + sh * (1 - cap - skirt), 0, 1)
    rgb = rgb / np.maximum(alpha, 1e-3)[..., None] * np.clip(cap + skirt, 0, 1)[..., None]
    base = np.dstack([np.clip(rgb, 0, 1), alpha])
    Image.fromarray((base * 255 + 0.5).astype(np.uint8), "RGBA").resize((240, 240), Image.LANCZOS).save(
        os.path.join(HERE, "knob_base.png"), optimize=True)
    # the pointer: an engraved groove from near the centre to the rim, pointing up
    p = Image.new("RGBA", (N, N), (0, 0, 0, 0))
    dp = ImageDraw.Draw(p)
    gw = N * 0.022
    dp.rounded_rectangle((c - gw, c - R * 0.93, c + gw, c - R * 0.35), gw, fill=(18, 18, 20, 255))
    dp.rounded_rectangle((c - gw * 0.35, c - R * 0.92, c + gw * 0.9, c - R * 0.36), gw * 0.5, fill=(150, 150, 155, 120))
    p.resize((240, 240), Image.LANCZOS).save(os.path.join(HERE, "knob.png"), optimize=True)
    return Image.fromarray((base * 255 + 0.5).astype(np.uint8), "RGBA"), p


RADII = (38, 40, 44, 50, 56, 58)   # every knob size the layout uses; MPC breaks a strip at r 59 (16384 px tall)
ARC = (242, 166, 35)


def knob_strips(base, pointer):
    """The knobs MPC actually shows: one 128-frame strip per size (frame = 2r+10 px, minimum first, stacked down).
    Each frame is the still steel cap with the pointer turned to its value, inside a dim 270-degree track that a
    honey arc fills up to the value. skin_post.py puts them in place of the builder's strips (same names, same sizes)."""
    for r in RADII:
        s = 2 * r + 10
        S = 4                                           # the ring is drawn 4x and reduced
        rw = max(2.4, s / 34.0)                         # ring width
        rr = s / 2.0 - rw * 1.6 - 1.0                   # ring radius (room outside it for the glow)
        kd = int(round(2 * (rr - rw / 2 - 2.0) / 0.91)) # the skirt (0.455 of the drawing) sits just inside the ring
        cap = base.resize((kd, kd), Image.LANCZOS)
        strip = Image.new("RGBA", (s, s * 128), (0, 0, 0, 0))
        for f in range(128):
            t = f / 127.0
            ang = -135 + 270 * t                        # clockwise from straight up
            frame = Image.new("RGBA", (s, s), (0, 0, 0, 0))
            big = Image.new("RGBA", (s * S, s * S), (0, 0, 0, 0))
            d = ImageDraw.Draw(big)
            c = s * S / 2.0

            def arc(a0, a1, width, fill):
                n = max(2, int(abs(a1 - a0) / 2) + 2)
                pts = []
                for i in range(n):
                    a = math.radians(a0 + (a1 - a0) * i / (n - 1) - 90)
                    pts.append((c + rr * S * math.cos(a), c + rr * S * math.sin(a)))
                d.line(pts, fill=fill, width=int(width * S), joint="curve")
                for x, y in (pts[0], pts[-1]):          # round ends
                    h = width * S / 2
                    d.ellipse((x - h, y - h, x + h, y + h), fill=fill)

            arc(-135, 135, rw, (58, 40, 16, 255))          # the unlit track: dark amber, like LEDs that are off
            if t > 0:                                       # the lit part: a wide bloom, a tighter glow, the light, a hot core
                for width, alpha, blur in ((rw * 4.0, 70, 1.5), (rw * 2.0, 150, 0.6)):
                    glow = Image.new("RGBA", big.size, (0, 0, 0, 0))
                    d = ImageDraw.Draw(glow)
                    arc(-135, ang, width, (255, 170, 40, alpha))
                    big = Image.alpha_composite(big, glow.filter(ImageFilter.GaussianBlur(rw * S * blur)))
                d = ImageDraw.Draw(big)
                arc(-135, ang, rw, (255, 184, 56, 255))
                arc(-135, ang, rw * 0.42, (255, 240, 190, 255))
            frame = Image.alpha_composite(frame, big.resize((s, s), Image.LANCZOS))
            k = Image.new("RGBA", (s, s), (0, 0, 0, 0))
            k.alpha_composite(cap, ((s - kd) // 2, (s - kd) // 2))
            pt = pointer.rotate(-ang, resample=Image.BICUBIC).resize((kd, kd), Image.LANCZOS)
            k.alpha_composite(pt, ((s - kd) // 2, (s - kd) // 2))
            frame = Image.alpha_composite(frame, k)
            strip.alpha_composite(frame, (0, f * s))
        strip.save(os.path.join(HERE, "knob_r%d.png" % r), optimize=True)


# --------------------------------------------------------------------------------------------------- layout numbers
SEC_X = (190, 480, 800, 1090)     # the four section dials
SEC_Y = 266
BIG = (640, 575)                  # the main dial
ECHO = ((135, 616), (245, 536), (355, 616))
AIR = ((925, 616), (1035, 536), (1145, 616))


SCRIPT = os.path.join(HERE, "fonts", "GreatVibes-Regular.ttf")
GOLD = np.array([0.95, 0.65, 0.14])
GOLD_DEEP = np.array([0.62, 0.33, 0.04])
LIGHT = np.array([-0.45, -0.60, 0.66])
LIGHT = LIGHT / np.linalg.norm(LIGHT)


def wet_word(word, size, runs, seed):
    """The word in calligraphy as wet paint, with thin runs falling from the lowest points of its strokes.

    Returns (rgb, alpha, shadow) at 1x, sized to the word plus room for the runs. Drawn 3x: the letters and runs are one
    outline, softened so each run pulls out of its stroke with a fillet, turned into a height (thicker away from the
    edges, fattest in the beads), lit from the upper left and given a glossy highlight."""
    r = np.random.default_rng(seed)
    S = 3
    f = ImageFont.truetype(SCRIPT, size * S)
    l, t, rgt, b = f.getbbox(word)
    pad = 10 * S
    w, h = rgt - l + 2 * pad, b - t + pad + 60 * S
    m = Image.new("L", (w, h), 0)
    d = ImageDraw.Draw(m)
    d.text((pad - l, pad // 2 - t), word, font=f, fill=255)
    a = np.asarray(m) > 127
    # where the ink ends, column by column; runs start at the lowest points (local maxima of that profile)
    bottom = np.array([np.max(np.nonzero(a[:, x])[0]) if a[:, x].any() else -1 for x in range(w)])
    cands = [x for x in range(4 * S, w - 4 * S) if bottom[x] > 0 and bottom[x] == bottom[x - 4 * S:x + 4 * S + 1].max()]
    picks = []
    for x in sorted(cands, key=lambda x: -bottom[x]):     # the lowest first, spread apart
        if all(abs(x - p) > 34 * S for p in picks):
            picks.append(x)
        if len(picks) >= runs:
            break
    for x in picks:
        y0 = bottom[x] - 2 * S
        length = r.uniform(14, 44) * S
        wd = r.uniform(1.6, 2.6) * S                      # thin runs
        n = 24
        left, right = [], []
        for i in range(n + 1):
            k = i / n
            half = wd * (1 - 0.25 * k) * (1 + 1.4 * max(0, 0.15 - k) / 0.15)   # flares where it leaves the letter
            yy = y0 + k * length
            xx = x + 0.8 * S * math.sin(k * 4 + x)
            left.append((xx - half, yy))
            right.append((xx + half, yy))
        d.polygon(left + right[::-1], fill=255)
        bx, by = (left[-1][0] + right[-1][0]) / 2, y0 + length
        rb = wd * 1.55
        d.ellipse((bx - rb, by - rb, bx + rb, by + rb * 1.2), fill=255)
        if r.random() < 0.3:
            dy = by + rb * 3 + r.random() * 8 * S
            d.ellipse((bx - rb * 0.7, dy - rb * 0.8, bx + rb * 0.7, dy + rb * 0.7), fill=255)
    mm = np.asarray(m.filter(ImageFilter.GaussianBlur(0.9 * S)), np.float64) / 255.0
    mm = (mm > 0.5).astype(np.float64)
    dist = ndimage.distance_transform_edt(mm)
    hgt = ndimage.gaussian_filter(np.sqrt(np.clip(dist / (3.0 * S), 0, 1)) * 3.0 * S, 0.8 * S)
    gy, gx = np.gradient(hgt)
    nrm = np.dstack([-gx, -gy, np.full_like(gx, 1.2)])
    nrm /= np.linalg.norm(nrm, axis=2, keepdims=True)
    diff = np.clip((nrm * LIGHT).sum(2), 0, 1)
    half_v = LIGHT + np.array([0, 0, 1.0])
    half_v /= np.linalg.norm(half_v)
    spec = np.clip((nrm * half_v).sum(2), 0, 1) ** 50
    thick = np.clip(dist / (5.0 * S), 0, 1)[..., None]
    rgb = (GOLD * (1 - 0.4 * thick) + GOLD_DEEP * 0.4 * thick) * (0.45 + 0.75 * diff[..., None]) + 0.85 * spec[..., None]
    small = lambda x: np.asarray(Image.fromarray(np.clip(x * 255, 0, 255).astype(np.uint8)).resize((w // S, h // S), Image.LANCZOS), np.float64) / 255.0
    alpha = small(mm)
    out = np.dstack([small(rgb[..., c] * mm) for c in range(3)]) / np.maximum(alpha, 1e-3)[..., None]
    shadow = small(ndimage.gaussian_filter(np.roll(np.roll(mm, 3 * S, 0), 2 * S, 1), 2.5 * S))
    return np.clip(out, 0, 1), alpha, shadow


def place(img, rgb, a, shadow, x, y):
    h, w = a.shape
    sub = img[y:y + h, x:x + w]
    sub *= (1 - 0.55 * shadow)[..., None]
    sub[:] = sub * (1 - a[..., None]) + rgb * a[..., None]
    return img


TITLE_X, TITLE_Y = 22, 2          # the word's box on the MAIN plate (art pixels)


def main_page():
    img = plate(W, H)
    img = panel(img, 40, 470 - Y0, 450, 712 - Y0, 0.03)
    img = panel(img, 830, 470 - Y0, 1240, 712 - Y0, 0.03)
    rgb, a, sh = wet_word("Drizzle", 92, 5, 21)
    img = place(img, rgb, a, sh, TITLE_X, TITLE_Y)
    print("title box: %d x %d at %d,%d" % (a.shape[1], a.shape[0], TITLE_X, TITLE_Y))
    save(img, "bg_main.png")


def detail_page():
    img = plate(W, H)
    img = panel(img, 24, 160 - Y0, 1256, 712 - Y0, 0.025)
    save(img, "bg_detail.png")


def tile():
    w, h = 270, 110
    img = plate(w, h)
    im = Image.fromarray((np.clip(img, 0, 1) * 255).astype(np.uint8), "RGB")
    d = ImageDraw.Draw(im)
    f = ImageFont.truetype(SCRIPT, 50)
    tw = d.textlength("Drizzle", font=f)
    d.text(((w - tw) / 2, 18), "Drizzle", font=f, fill=(242, 166, 35))
    f2 = ImageFont.truetype(os.path.join(FONTS, "TitilliumWeb-SemiBold.ttf"), 13)
    tw2 = d.textlength("F X   C H A I N", font=f2)
    d.text(((w - tw2) / 2, 84), "F X   C H A I N", font=f2, fill=(170, 170, 176))
    im.save(os.path.join(HERE, "tile.png"), optimize=True)


# --------------------------------------------------------------------------------------------------- layout.conf
THEME = """theme_bg=111113
theme_panel=111113
theme_line=3a3a3e
theme_ink=ececee
theme_ink_dim=c4c4c8
theme_ink_faint=8c8c92
theme_accent=f2a623
theme_accent_hi=ffc14d
theme_knob_face=b8b8bc
theme_knob_ring=101012
theme_knob_dot=1a1a1c
theme_lcd=08080a
theme_seg_active=f2a623
theme_seg_inactive=2e2e32
theme_seg_active_tx=111113
theme_btn_text=ececee
knob_img=art/knob.png
knob_base=art/knob_base.png"""

MAIN_Q = "drizzle,tone,grit,swirl,space,dly_time,dly_fb,dly_mix,air_rate,air_freq,air_mix,level"


def title(lines, word, sub, big=False):
    if big:      # the plugin's name on MAIN: larger, in the upper left
        # the word itself is painted into bg_main.png (wet_word); only the small line beside it is text
        lines.append('text cx=368 cy=150 label="%s" size=1.1 weight=600 color=a8a8ae spacing=4' % sub)
        return
    lines.append('text cx=128 cy=114 label="%s" size=2.4 weight=700 color=f2a623 spacing=4' % word)
    lines.append('text cx=128 cy=144 label="%s" size=0.9 color=8c8c92 spacing=3' % sub)


def style_stepper(lines, key):
    lines.append('text cx=812 cy=124 label="STYLE" size=1.1 color=8c8c92 spacing=2')
    lines.append('stepper cx=1040 cy=124 w=380 h=44 label="" key=%s talign=center tsize=22 tcolor=ececee tweight=bold' % key)


def knob(lines, x, y, r, key, name, vs=16, bw=190, ns=15):
    lines.append('knob cx=%d cy=%d r=%d label="%s" key=%s ns=%d vs=%d bw=%d' % (x, y, r, name, key, ns if name else 0, vs, bw))


def layout():
    L = ["# Drizzle, written by art/make_art.py (edit that, not this).", THEME, "qlinks_track = " + MAIN_Q]

    L.append("[tab MAIN]")
    L.append('art file="art/bg_main.png" x=0 y=86 w=1280 h=628')
    title(L, "DRIZZLE", "FX CHAIN", big=True)
    L.append('stepper cx=640 cy=124 w=440 h=48 label="" key=preset talign=center tsize=26 tcolor=ececee tweight=bold')
    knob(L, 1230, 136, 38, "level", "", vs=16, bw=96)      # the processed voice's level, beside the preset name
    L.append('text cx=1146 cy=136 label="LEVEL" size=1.0 weight=600 color=c4c4c8 spacing=2')
    for x, key, word in zip(SEC_X, ("tone", "grit", "swirl", "space"), ("TONE", "GRIT", "SWIRL", "SPACE")):
        knob(L, x, SEC_Y, 58, key, "", vs=18, bw=180)
        L.append('stepper cx=%d cy=388 w=250 h=40 label="" key=%s_style talign=center tsize=18 tcolor=f2c879' % (x, key))
        L.append('text cx=%d cy=428 label="%s" size=1.4 weight=700 spacing=2' % (x, word))
    knob(L, BIG[0], BIG[1], 58, "drizzle", "", vs=20, bw=240)
    L.append('text cx=640 cy=694 label="Drizzle" size=3.0 weight=400 color=f2a623 fontfile=art/fonts/GreatVibes-Regular.ttf')
    for group, title_x, keys, words in (
            (ECHO, 245, ("dly_time", "dly_fb", "dly_mix"), ("TIME", "FEEDBACK", "MIX")),
            (AIR, 1035, ("air_rate", "air_freq", "air_mix"), ("RATE", "AIR", "MIX"))):
        L.append('text cx=%d cy=488 label="%s" size=1.2 weight=700 color=f2a623 spacing=3' % (title_x, "ECHO" if group is ECHO else "AIR"))
        for (x, y), key, word in zip(group, keys, words):
            knob(L, x, y, 38, key, "", vs=16, bw=110)
            L.append('text cx=%d cy=%d label="%s" size=1.0 weight=600 color=c4c4c8' % (x, y + 76, word))
    L.append('qlinks "MAIN" = ' + MAIN_Q)

    L.append("[tab TONE]")
    L.append('art file="art/bg_detail.png" x=0 y=86 w=1280 h=628')
    title(L, "TONE", "EQUALISER")
    style_stepper(L, "tone_style")
    cols = (120, 330, 540, 750, 960, 1170)
    heads = ("CUTS", "LOW SHELF", "BELL 1", "BELL 2", "BELL 3", "HIGH SHELF")
    for x, hd in zip(cols, heads):
        L.append('text cx=%d cy=192 label="%s" size=1.2 weight=700 color=f2a623 spacing=2' % (x, hd))
    rows = (282, 445, 608)
    knob(L, cols[0], rows[0], 40, "eq_hpf", "LOW CUT")
    knob(L, cols[0], rows[1], 40, "eq_lpf", "HIGH CUT")
    knob(L, cols[0], rows[2], 40, "tone", "AMOUNT")
    for c, pre in zip(cols[1:], ("eq_ls", "eq_b1", "eq_b2", "eq_b3", "eq_hs")):
        knob(L, c, rows[0], 40, pre + "_f", "FREQ")
        knob(L, c, rows[1], 40, pre + "_g", "GAIN")
        knob(L, c, rows[2], 40, pre + "_q", "Q")
    L.append('qlinks "TONE" = eq_hpf,eq_ls_f,eq_ls_g,eq_b1_f,eq_b1_g,eq_b2_f,eq_b2_g,eq_b3_f,eq_b3_g,eq_hs_f,eq_hs_g,eq_lpf,'
             'eq_b1_q,eq_b2_q,eq_b3_q,tone')

    grid_x = (200, 493, 787, 1080)
    grid_y = (300, 530)

    L.append("[tab GRIT]")
    L.append('art file="art/bg_detail.png" x=0 y=86 w=1280 h=628')
    title(L, "GRIT", "SATURATION")
    style_stepper(L, "grit_style")
    for (x, y), (key, name) in zip([(x, y) for y in grid_y for x in grid_x], (
            ("grit", "AMOUNT"), ("gr_drive", "DRIVE"), ("gr_trim", "TRIM"), ("gr_cut", "TONE CUT"),
            ("gr_pos_in", "PUSH +"), ("gr_neg_in", "PUSH -"), ("gr_pos_out", "OUT +"), ("gr_neg_out", "OUT -"))):
        knob(L, x, y, 56, key, name, vs=18, bw=230, ns=17)
    L.append('qlinks "GRIT" = grit,gr_drive,gr_trim,gr_cut,gr_pos_in,gr_neg_in,gr_pos_out,gr_neg_out')

    L.append("[tab SWIRL]")
    L.append('art file="art/bg_detail.png" x=0 y=86 w=1280 h=628')
    title(L, "SWIRL", "CHORUS")
    style_stepper(L, "swirl_style")
    for x, key, name in ((150, "swirl", "AMOUNT"), (360, "sw_delay", "DELAY"), (570, "sw_depth", "DEPTH"), (780, "sw_width", "WIDTH")):
        knob(L, x, 290, 50, key, name, vs=17, bw=200, ns=16)
    L.append('text cx=985 cy=226 label="FILTER" size=1.1 weight=700 color=f2a623 spacing=2')
    L.append('enum_v cx=985 cy=290 label="" key=sw_filter sw=150')
    knob(L, 1170, 290, 44, "sw_freq", "FREQ", vs=16, bw=150)
    for x, n in ((250, 1), (640, 2), (1030, 3)):
        knob(L, x, 500, 50, "sw_rate%d" % n, "LFO %d" % n, vs=17, bw=220, ns=16)
        L.append('enum_h cx=%d cy=650 label="" key=sw_shape%d sw=92' % (x, n))
    L.append('qlinks "SWIRL" = swirl,sw_delay,sw_depth,sw_width,sw_rate1,sw_rate2,sw_rate3,sw_freq,'
             'sw_shape1,sw_shape2,sw_shape3,sw_filter')

    L.append("[tab SPACE]")
    L.append('art file="art/bg_detail.png" x=0 y=86 w=1280 h=628')
    title(L, "SPACE", "REVERB")
    style_stepper(L, "space_style")
    for (x, y), (key, name) in zip([(x, y) for y in grid_y for x in grid_x], (
            ("space", "AMOUNT"), ("sp_size", "SIZE"), ("sp_decay", "DECAY"), ("sp_pre", "PRE-DELAY"),
            ("sp_density", "DENSITY"), ("sp_damp", "DAMPING"), ("sp_late", "LATE MIX"), ("sp_tone", "TONE"))):
        knob(L, x, y, 56, key, name, vs=18, bw=230, ns=17)
    L.append('qlinks "SPACE" = space,sp_size,sp_decay,sp_pre,sp_density,sp_damp,sp_late,sp_tone')

    open(os.path.join(PORT, "layout.conf"), "w").write("\n".join(L) + "\n")


knob_strips(*steel_knob())
main_page()
detail_page()
tile()
layout()
print("art: bg_main.png bg_detail.png tile.png knob.png knob_base.png, layout.conf")
