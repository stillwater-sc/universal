#!/usr/bin/env python3
# generate_lns_plots.py
#
# Generates the SVG figures for docs/algorithmic-details/lns-log-add-sub.md.
# Each plot is rendered with matplotlib at a fixed size suitable for embedding
# in Starlight / GitHub markdown. ASCII-only labels and axis text.
#
# Usage:
#     python3 docs/algorithmic-details/scripts/generate_lns_plots.py
#
# Output directory: docs/img/algorithmic-details/
#
# The math here is the Python equivalent of the seven sb_add / sb_sub
# implementations in include/sw/universal/number/lns/lns_addsub_algorithms.hpp.
# It is intentionally a faithful reproduction rather than a wrapper around the
# C++ code: the goal is to show the *shape* and *envelope* of each algorithm,
# not to bit-exactly match the implementation.

import math
import os
import sys

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

OUT_DIR = os.path.normpath(
    os.path.join(os.path.dirname(__file__), "..", "..", "img", "algorithmic-details")
)
os.makedirs(OUT_DIR, exist_ok=True)


# ----------------------------------------------------------------------------
# Reference (oracle) transfer functions
# ----------------------------------------------------------------------------

def sb_add_ref(d):
    """log2(1 + 2^d) for d <= 0.  Always positive, monotone non-increasing."""
    return np.log2(1.0 + np.power(2.0, d))


def sb_sub_ref(d):
    """log2(1 - 2^d) for d < 0.  -> -inf as d -> 0-, -> 0 as d -> -inf."""
    return np.log2(1.0 - np.power(2.0, d))


# ----------------------------------------------------------------------------
# Algorithm: Lookup (Mitchell 1962 style)
# ----------------------------------------------------------------------------

def make_lookup(index_bits, rbits):
    """Return (sb_add, sb_sub) callables emulating LookupAddSub<Lns, index_bits>."""
    d_range = float(rbits) + 2.0
    n = 1 << index_bits
    step = d_range / n
    grid = -np.arange(n + 1) * step  # 0, -step, -2*step, ..., -d_range
    add_tbl = np.log2(1.0 + np.power(2.0, grid))
    # sub_tbl entry 0 is unused (d == 0); fill with 0 sentinel
    sub_tbl = np.zeros_like(add_tbl)
    sub_tbl[1:] = np.log2(1.0 - np.power(2.0, grid[1:]))

    def sb_add(d):
        d = np.asarray(d, dtype=float)
        out = np.zeros_like(d)
        mask = d > -d_range
        ad = -np.minimum(d[mask], 0.0)
        idx_f = ad / step
        idx = np.floor(idx_f).astype(int)
        idx = np.clip(idx, 0, n - 1)
        frac = idx_f - idx
        out[mask] = add_tbl[idx] + frac * (add_tbl[idx + 1] - add_tbl[idx])
        return out

    def sb_sub(d):
        d = np.asarray(d, dtype=float)
        out = np.zeros_like(d)
        mask = (d < 0.0) & (d > -d_range)
        ad = -d[mask]
        idx_f = ad / step
        idx = np.floor(idx_f).astype(int)
        # Lowest cell falls back to direct evaluation in the C++ implementation.
        # For the educational plot we mirror that: low cell -> reference, else
        # linear interpolation on the table.
        result = np.empty_like(ad)
        low = idx == 0
        # Direct eval for the lowest cell
        result[low] = sb_sub_ref(d[mask][low])
        # Linear interp elsewhere
        hi = ~low
        ih = idx[hi]
        ih = np.clip(ih, 1, n - 1)
        frac = idx_f[hi] - ih
        result[hi] = sub_tbl[ih] + frac * (sub_tbl[ih + 1] - sub_tbl[ih])
        out[mask] = result
        return out

    return sb_add, sb_sub


# ----------------------------------------------------------------------------
# Algorithm: Polynomial (degree-7 (1+x)/(1-x) substitution)
# ----------------------------------------------------------------------------

INV_LN2 = 1.0 / math.log(2.0)


def make_polynomial(rbits):
    d_floor = -(float(rbits) + 2.0)

    def sb_add(d):
        d = np.asarray(d, dtype=float)
        out = np.zeros_like(d)
        mask = (d <= 0.0) & (d >= d_floor)
        u = np.power(2.0, d[mask])
        x = u / (2.0 + u)
        x2 = x * x
        x3 = x * x2
        x5 = x3 * x2
        x7 = x5 * x2
        c1 = 2.0 * INV_LN2
        out[mask] = c1 * (x + x3 / 3.0 + x5 / 5.0 + x7 / 7.0)
        return out

    def sb_sub(d):
        d = np.asarray(d, dtype=float)
        out = np.zeros_like(d)
        mask = (d < 0.0) & (d >= d_floor)
        u = np.power(2.0, d[mask])
        # u > 0.5 (i.e. d > -1) falls back to direct evaluation -- cancellation.
        is_cancel = u > 0.5
        result = np.empty_like(u)
        result[is_cancel] = sb_sub_ref(d[mask][is_cancel])
        ok = ~is_cancel
        uu = u[ok]
        x = uu / (2.0 - uu)
        x2 = x * x
        x3 = x * x2
        x5 = x3 * x2
        x7 = x5 * x2
        c1 = 2.0 * INV_LN2
        result[ok] = -(c1 * (x + x3 / 3.0 + x5 / 5.0 + x7 / 7.0))
        out[mask] = result
        return out

    return sb_add, sb_sub


# ----------------------------------------------------------------------------
# Algorithm: ArnoldBailey (piecewise linear at integer knots)
# ----------------------------------------------------------------------------

def make_arnold_bailey():
    # Knots for sb_add at d = 0, -1, ..., -5; tail ramp to 0 by -6.
    knots_d = np.array([0.0, -1.0, -2.0, -3.0, -4.0, -5.0, -6.0])
    knots_f = np.log2(1.0 + np.power(2.0, knots_d))
    knots_f[-1] = 0.0  # tail ramp endpoint

    knots_g_d = np.array([-1.0, -2.0, -3.0, -4.0, -5.0, -6.0])
    knots_g = np.log2(1.0 - np.power(2.0, knots_g_d))
    knots_g = np.concatenate([knots_g, [0.0]])  # ramp to 0 at -6
    knots_g_d = np.concatenate([knots_g_d, [-6.0]])

    def sb_add(d):
        d = np.asarray(d, dtype=float)
        out = np.zeros_like(d)
        mask = (d <= 0.0) & (d > -6.0)
        dd = d[mask]
        # Use np.interp; knots_d is in *increasing* order for np.interp
        out[mask] = np.interp(dd, knots_d[::-1], knots_f[::-1])
        return out

    def sb_sub(d):
        d = np.asarray(d, dtype=float)
        out = np.zeros_like(d)
        mask = (d < 0.0) & (d > -6.0)
        dd = d[mask]
        # Cancellation regime: d in (-1, 0) -> direct evaluation.
        is_cancel = dd > -1.0
        res = np.empty_like(dd)
        res[is_cancel] = sb_sub_ref(dd[is_cancel])
        ok = ~is_cancel
        res[ok] = np.interp(dd[ok], knots_g_d[::-1], knots_g[::-1])
        out[mask] = res
        return out

    return sb_add, sb_sub


# ----------------------------------------------------------------------------
# Algorithm: CORDIC (hyperbolic, N iterations)
# ----------------------------------------------------------------------------

def cordic_repeats(i):
    # Hyperbolic CORDIC repeats at 4, 13, 40, ...; r_{k+1} = 3 r_k + 1.
    r = 4
    while r <= 60:
        if r == i:
            return True
        r = 3 * r + 1
    return False


def make_cordic(N):
    # Precompute atanh(2^-i)
    atanh_tbl = {}
    for i in range(1, N + 2):
        x = 2.0 ** (-i)
        atanh_tbl[i] = 0.5 * math.log((1.0 + x) / (1.0 - x))

    # Gain
    K = 1.0
    for i in range(1, N + 1):
        t = 2.0 ** (-i)
        factor = math.sqrt(1.0 - t * t)
        K *= factor
        if cordic_repeats(i):
            K *= factor
    K_INV = K
    K_H = 1.0 / K_INV

    def cordic_exp(z):
        x, y, zr = 1.0, 0.0, float(z)
        for i in range(1, N + 1):
            shift = 2.0 ** (-i)
            sigma = 1.0 if zr >= 0.0 else -1.0
            nx = x + sigma * y * shift
            ny = y + sigma * x * shift
            nz = zr - sigma * atanh_tbl[i]
            x, y, zr = nx, ny, nz
            if cordic_repeats(i):
                sigma = 1.0 if zr >= 0.0 else -1.0
                rx = x + sigma * y * shift
                ry = y + sigma * x * shift
                rz = zr - sigma * atanh_tbl[i]
                x, y, zr = rx, ry, rz
        return (x + y) * K_H

    def cordic_pow2(d):
        q = int(d)
        if d < 0.0 and d != float(q):
            q -= 1
        f = d - float(q)
        ef = cordic_exp(f * math.log(2.0))
        return ef * (2.0 ** q)

    def cordic_ln_in_unit(w):
        x, y, z = w + 1.0, w - 1.0, 0.0
        for i in range(1, N + 1):
            shift = 2.0 ** (-i)
            sigma = 1.0 if y < 0.0 else -1.0
            nx = x + sigma * y * shift
            ny = y + sigma * x * shift
            nz = z - sigma * atanh_tbl[i]
            x, y, z = nx, ny, nz
            if cordic_repeats(i):
                sigma = 1.0 if y < 0.0 else -1.0
                rx = x + sigma * y * shift
                ry = y + sigma * x * shift
                rz = z - sigma * atanh_tbl[i]
                x, y, z = rx, ry, rz
        return 2.0 * z

    def cordic_log2(w):
        if w <= 0.0:
            return float("-inf")
        k = 0
        m = w
        while m >= 2.0:
            m *= 0.5
            k += 1
        while m < 1.0:
            m *= 2.0
            k -= 1
        return (cordic_ln_in_unit(m) + k * math.log(2.0)) * INV_LN2

    def sb_add(d):
        out = np.zeros_like(d)
        d_floor = -(float(N) + 2.0)
        for j, dj in enumerate(d):
            if dj > 0.0:
                dj = 0.0
            if dj < d_floor:
                continue
            v = cordic_pow2(dj)
            w = 1.0 + v
            out[j] = cordic_ln_in_unit(w) * INV_LN2
        return out

    def sb_sub(d):
        out = np.zeros_like(d)
        d_floor = -(float(N) + 2.0)
        for j, dj in enumerate(d):
            if dj >= 0.0:
                continue
            if dj < d_floor:
                continue
            if dj > -1.0:
                out[j] = sb_sub_ref(np.array([dj]))[0]
                continue
            v = cordic_pow2(dj)
            w = 1.0 - v
            out[j] = cordic_log2(w)
        return out

    return sb_add, sb_sub


# ----------------------------------------------------------------------------
# Algorithm: Arnold Cotransformation Combination (educational sketch)
# ----------------------------------------------------------------------------

def make_arnold_cotr(rbits, split_j=None, index_bits=None):
    f = rbits
    if split_j is None:
        split_j = max(2, min(10, (f + 4) // 2))
    if index_bits is None:
        index_bits = min(rbits + 2, 10)

    ulp = 2.0 ** (-f)
    delta_h = 2.0 ** (split_j - f)
    e_F4 = float(f) + 2.0

    # sb_add Mitchell-style table (same as LookupAddSub)
    sb_entries = 1 << index_bits
    sb_d_range = float(f) + 2.0
    sb_step = sb_d_range / sb_entries
    sb_grid = -np.arange(sb_entries + 1) * sb_step
    sb_tbl = np.log2(1.0 + np.power(2.0, sb_grid))

    f3_entries = 1 << split_j
    F3 = np.full(f3_entries, -1.0e9)
    for i in range(1, f3_entries):
        z_l = i * ulp
        F3[i] = math.log2(1.0 - 2.0 ** (-z_l))

    f4_entries_uncapped = int(e_F4 * (2.0 ** (f - split_j)) + 0.99)
    f4_entries = min(f4_entries_uncapped, 4096)
    F4 = np.empty(f4_entries)
    for k in range(f4_entries):
        z_h = (k + 1) * delta_h
        F4[k] = math.log2(1.0 - 2.0 ** (-z_h))

    def sb_add_one(d):
        if d > 0.0:
            d = 0.0
        ad = -d
        if ad >= sb_d_range:
            return 0.0
        idx_f = ad / sb_step
        idx = int(idx_f)
        frac = idx_f - idx
        return sb_tbl[idx] + frac * (sb_tbl[idx + 1] - sb_tbl[idx])

    def sb_add(d):
        return np.array([sb_add_one(float(x)) for x in d])

    def sb_sub_one(d):
        if d >= 0.0:
            return 0.0
        d_floor = -(float(f) + 2.0)
        if d < d_floor:
            return 0.0
        z = -d
        idx_h_d = z / delta_h
        idx_h = int(idx_h_d)
        z_h = idx_h * delta_h
        z_l = z - z_h
        f3_idx_d = z_l / ulp
        f3_idx = int(f3_idx_d + 0.5)
        if f3_idx >= f3_entries:
            f3_idx -= f3_entries
            idx_h += 1
            z_h = idx_h * delta_h

        if idx_h == 0:
            return F3[f3_idx]

        f4_idx = idx_h - 1
        if f4_idx < f4_entries:
            F4_neg_zh = F4[f4_idx]
        elif z_h >= e_F4:
            F4_neg_zh = 0.0
        else:
            F4_neg_zh = math.log2(1.0 - 2.0 ** (-z_h))

        if f3_idx == 0:
            return F4_neg_zh

        F3_zl = F3[f3_idx]
        z_hat = F3_zl - F4_neg_zh - z_h
        return F4_neg_zh + sb_add_one(z_hat)

    def sb_sub(d):
        return np.array([sb_sub_one(float(x)) for x in d])

    return sb_add, sb_sub


# ----------------------------------------------------------------------------
# Algorithm: DoubleTrip (cast to double, native +, cast back)
# ----------------------------------------------------------------------------

def make_double_trip():
    # Modelled as identical to reference at the sb_add / sb_sub level.
    return sb_add_ref, sb_sub_ref


# ----------------------------------------------------------------------------
# Plot helpers
# ----------------------------------------------------------------------------

def fig_save(fig, name):
    out = os.path.join(OUT_DIR, name)
    fig.savefig(out, format="svg", bbox_inches="tight")
    plt.close(fig)
    print("  wrote {}".format(out))


def plot_sb_add_transfer():
    d = np.linspace(-10.0, 0.0, 1024)
    y = sb_add_ref(d)
    fig, ax = plt.subplots(figsize=(7.0, 4.0))
    ax.plot(d, y, color="C0", linewidth=2.0)
    ax.axhline(0.0, color="0.7", linewidth=0.6)
    ax.axhline(1.0, color="0.85", linewidth=0.6, linestyle=":")
    ax.set_xlabel("d = Lmin - Lmax  (log-domain difference)")
    ax.set_ylabel("sb_add(d) = log2(1 + 2^d)")
    ax.set_title("Log-add correction: sb_add(d) over d in [-10, 0]")
    ax.set_xlim(-10.0, 0.0)
    ax.set_ylim(-0.05, 1.1)
    ax.grid(True, alpha=0.3)
    ax.annotate("sb_add(0) = 1\n(a + a -> 2a, +1 in log2)",
                xy=(0.0, 1.0), xytext=(-3.5, 0.78),
                arrowprops=dict(arrowstyle="->", color="0.5"),
                fontsize=9, color="0.3")
    ax.annotate("as d -> -inf, sb_add -> 0\n(smaller operand vanishes)",
                xy=(-9.5, 0.0), xytext=(-7.5, 0.35),
                arrowprops=dict(arrowstyle="->", color="0.5"),
                fontsize=9, color="0.3")
    fig_save(fig, "sb_add_transfer.svg")


def plot_sb_sub_transfer():
    d = np.linspace(-10.0, -1e-4, 2048)
    y = sb_sub_ref(d)
    fig, ax = plt.subplots(figsize=(7.0, 4.0))
    ax.plot(d, y, color="C3", linewidth=2.0)
    ax.axhline(0.0, color="0.7", linewidth=0.6)
    ax.axvline(0.0, color="0.7", linewidth=0.6)
    ax.set_xlabel("d = Lmin - Lmax  (mixed-sign log-domain difference)")
    ax.set_ylabel("sb_sub(d) = log2(1 - 2^d)")
    ax.set_title("Log-sub correction: sb_sub(d) over d in [-10, 0)")
    ax.set_xlim(-10.0, 0.0)
    ax.set_ylim(-12.0, 0.5)
    ax.grid(True, alpha=0.3)
    ax.annotate("singularity:\nsb_sub(d) -> -inf\nas d -> 0-",
                xy=(-0.001, -10.0), xytext=(-3.5, -7.5),
                arrowprops=dict(arrowstyle="->", color="0.5"),
                fontsize=9, color="0.3")
    ax.annotate("as d -> -inf, sb_sub -> 0\n(operands differ in magnitude)",
                xy=(-9.5, -0.005), xytext=(-7.5, -3.0),
                arrowprops=dict(arrowstyle="->", color="0.5"),
                fontsize=9, color="0.3")
    fig_save(fig, "sb_sub_transfer.svg")


ALG_COLORS = {
    "DoubleTrip":       "C7",
    "DirectEvaluation": "C0",
    "Lookup":           "C1",
    "Polynomial":       "C2",
    "ArnoldBailey":     "C3",
    "CORDIC":           "C4",
    "ArnoldCotr":       "C5",
}
ALG_STYLES = {
    "DoubleTrip":       (3, (1, 1)),
    "DirectEvaluation": "-",
    "Lookup":           "--",
    "Polynomial":       "-.",
    "ArnoldBailey":     ":",
    "CORDIC":           (0, (3, 1, 1, 1)),
    "ArnoldCotr":       (0, (5, 1)),
}


def build_algorithms(rbits):
    lookup_add, lookup_sub = make_lookup(min(rbits + 2, 10), rbits)
    poly_add, poly_sub = make_polynomial(rbits)
    ab_add, ab_sub = make_arnold_bailey()
    cordic_add, cordic_sub = make_cordic(rbits)
    cotr_add, cotr_sub = make_arnold_cotr(rbits)
    dt_add, dt_sub = make_double_trip()
    algs = {
        "DoubleTrip":       (dt_add, dt_sub),
        "DirectEvaluation": (sb_add_ref, sb_sub_ref),
        "Lookup":           (lookup_add, lookup_sub),
        "Polynomial":       (poly_add, poly_sub),
        "ArnoldBailey":     (ab_add, ab_sub),
        "CORDIC":           (cordic_add, cordic_sub),
        "ArnoldCotr":       (cotr_add, cotr_sub),
    }
    return algs


def plot_algorithms_overlay(rbits):
    algs = build_algorithms(rbits)

    # sb_add overlay
    d = np.linspace(-6.0, 0.0, 1024)
    fig, (ax_top, ax_bot) = plt.subplots(2, 1, figsize=(8.0, 7.5), sharex=True)
    ref = sb_add_ref(d)
    for name, (add_fn, _sub_fn) in algs.items():
        y = add_fn(d)
        ax_top.plot(d, y, color=ALG_COLORS[name], linestyle=ALG_STYLES[name],
                    linewidth=1.6, label=name)
        ax_bot.plot(d, y - ref, color=ALG_COLORS[name], linestyle=ALG_STYLES[name],
                    linewidth=1.4, label=name)
    ax_top.set_ylabel("sb_add(d)")
    ax_top.set_title("sb_add(d) per algorithm  (rbits = {})".format(rbits))
    ax_top.grid(True, alpha=0.3)
    ax_top.legend(loc="upper left", fontsize=8, ncol=2)

    ax_bot.set_xlabel("d")
    ax_bot.set_ylabel("error = alg(d) - reference (log domain)")
    ax_bot.set_title("Log-domain error vs DirectEvaluation reference")
    ax_bot.axhline(0.0, color="0.5", linewidth=0.6)
    ax_bot.grid(True, alpha=0.3)
    fig_save(fig, "sb_add_overlay_rbits{}.svg".format(rbits))


def plot_sb_sub_overlay(rbits):
    algs = build_algorithms(rbits)
    d = np.linspace(-6.0, -1e-3, 1024)
    fig, (ax_top, ax_bot) = plt.subplots(2, 1, figsize=(8.0, 7.5), sharex=True)
    ref = sb_sub_ref(d)
    for name, (_add_fn, sub_fn) in algs.items():
        y = sub_fn(d)
        ax_top.plot(d, y, color=ALG_COLORS[name], linestyle=ALG_STYLES[name],
                    linewidth=1.6, label=name)
        err = y - ref
        # Clip very large errors so the plot stays readable.
        err = np.clip(err, -1.5, 1.5)
        ax_bot.plot(d, err, color=ALG_COLORS[name], linestyle=ALG_STYLES[name],
                    linewidth=1.4, label=name)
    ax_top.set_ylabel("sb_sub(d)")
    ax_top.set_title("sb_sub(d) per algorithm  (rbits = {})".format(rbits))
    ax_top.set_ylim(-12.0, 0.5)
    ax_top.grid(True, alpha=0.3)
    ax_top.legend(loc="lower right", fontsize=8, ncol=2)

    ax_bot.set_xlabel("d")
    ax_bot.set_ylabel("error = alg(d) - reference  (clipped to +/-1.5)")
    ax_bot.set_title("Log-domain error vs DirectEvaluation reference")
    ax_bot.axhline(0.0, color="0.5", linewidth=0.6)
    ax_bot.grid(True, alpha=0.3)
    fig_save(fig, "sb_sub_overlay_rbits{}.svg".format(rbits))


def plot_error_envelope_sweep():
    rbits_list = [4, 8, 16]
    fig, axes = plt.subplots(1, 3, figsize=(13.5, 4.0), sharey=True)
    d = np.linspace(-6.0, 0.0, 1024)
    ref = sb_add_ref(d)
    for ax, rbits in zip(axes, rbits_list):
        algs = build_algorithms(rbits)
        for name, (add_fn, _sub_fn) in algs.items():
            if name in ("DoubleTrip", "DirectEvaluation"):
                continue
            err = np.abs(add_fn(d) - ref)
            ax.semilogy(d, np.maximum(err, 1e-18),
                        color=ALG_COLORS[name], linestyle=ALG_STYLES[name],
                        linewidth=1.4, label=name)
        ulp = 2.0 ** (-rbits)
        ax.axhline(ulp, color="0.4", linewidth=0.6, linestyle=":",
                   label="1 ULP (log) = 2^-{}".format(rbits))
        ax.set_xlabel("d")
        ax.set_title("rbits = {}".format(rbits))
        ax.set_ylim(1e-10, 1.0)
        ax.grid(True, which="both", alpha=0.3)
    axes[0].set_ylabel("|alg(d) - ref(d)|  (log scale)")
    axes[-1].legend(loc="upper left", fontsize=8, bbox_to_anchor=(1.02, 1.0))
    fig.suptitle("sb_add log-domain error envelope vs rbits", y=1.02)
    fig_save(fig, "sb_add_error_envelope.svg")


def plot_cordic_iteration_curve():
    """Show how CORDIC error decreases with iteration count."""
    iters = [4, 8, 12, 16, 20, 24, 28, 32]
    d = np.linspace(-6.0, 0.0, 256)
    ref = sb_add_ref(d)
    max_err = []
    for n in iters:
        add_fn, _ = make_cordic(n)
        err = np.max(np.abs(add_fn(d) - ref))
        max_err.append(err)
    fig, ax = plt.subplots(figsize=(7.0, 4.0))
    ax.semilogy(iters, max_err, "o-", color="C4", linewidth=1.8, label="measured")
    # Engineering bound: 4 * 2^-N
    ax.semilogy(iters, [4.0 * 2.0 ** (-n) for n in iters],
                "--", color="0.5", linewidth=1.0,
                label="bound: 4 * 2^-N")
    ax.set_xlabel("CORDIC iterations N")
    ax.set_ylabel("max |sb_add(d) - ref(d)| over d in [-6, 0]")
    ax.set_title("Hyperbolic CORDIC: log-domain error vs iteration budget")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend(loc="upper right", fontsize=9)
    fig_save(fig, "cordic_iteration_curve.svg")


def main():
    print("Generating LNS algorithmic-details plots to {}".format(OUT_DIR))
    plot_sb_add_transfer()
    plot_sb_sub_transfer()
    plot_algorithms_overlay(rbits=8)
    plot_algorithms_overlay(rbits=16)
    plot_sb_sub_overlay(rbits=8)
    plot_sb_sub_overlay(rbits=16)
    plot_error_envelope_sweep()
    plot_cordic_iteration_curve()
    print("Done.")


if __name__ == "__main__":
    sys.exit(main() or 0)
