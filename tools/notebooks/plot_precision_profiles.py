#!/usr/bin/env python3
"""plot_precision_profiles.py: overlay precision-vs-magnitude profiles (decimals of accuracy).

Reads the CSV files written by write_precision_csv() in
include/sw/universal/utility/precision_profile.hpp -- for example by
applications/mixed-precision/dsp/precision_profiles.cpp:

    dsp_precision_profiles out/
    python3 tools/notebooks/plot_precision_profiles.py out/int16.csv out/Q15.csv out/fp16.csv \
        out/lns_16_10_.csv out/bposit_16_5_2_.csv --dsp --output profiles16.png

Each CSV holds magnitude, log2_magnitude, decimals and fraction_bits for one type, with the
label and type in '#' comment lines.  The plot shows decimals of accuracy against log2 of the
magnitude, one curve per file (up to five read best).  --dsp adds the DSP markers: the signal
band [2^-15, 1), FFT growth to 2^log2(N) for N = 256, 1024 and 4096, and an SQNR axis at
6.02 dB per fraction bit.  The output format follows the --output extension (png, svg, pdf).

Requires matplotlib.
"""

import argparse
import csv
import math
import sys


def read_profile(path):
    meta = {}
    log2m, decimals = [], []
    with open(path, newline="") as f:
        rows = []
        for line in f:
            if line.startswith("#"):
                key, _, value = line[1:].partition(":")
                meta[key.strip()] = value.strip()
            else:
                rows.append(line)
        for row in csv.DictReader(rows):
            log2m.append(float(row["log2_magnitude"]))
            decimals.append(float(row["decimals"]))
    return meta.get("label", path), meta, log2m, decimals


def main():
    parser = argparse.ArgumentParser(description="Overlay precision-vs-magnitude profiles.")
    parser.add_argument("csv", nargs="+", help="profile CSV files written by write_precision_csv()")
    parser.add_argument("--output", "-o", default="precision_profiles.png", help="output file: .png, .svg or .pdf")
    parser.add_argument("--dsp", action="store_true", help="add DSP markers: signal band, FFT growth, SQNR axis")
    parser.add_argument("--xmin", type=float, default=None, help="smallest log2 magnitude to show")
    parser.add_argument("--xmax", type=float, default=None, help="largest log2 magnitude to show")
    parser.add_argument("--title", default="Decimals of accuracy, -log10(ulp / (2|x|))")
    args = parser.parse_args()

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        sys.exit("plot_precision_profiles.py needs matplotlib: pip install matplotlib")

    fig, ax = plt.subplots(figsize=(11, 6))
    for path in args.csv:
        label, meta, log2m, decimals = read_profile(path)
        if not log2m:
            continue
        # 0 decimals outside the type's range: close the curve at both ends
        xs = [log2m[0]] + log2m + [log2m[-1]]
        ys = [0.0] + decimals + [0.0]
        ax.plot(xs, ys, linewidth=1.2, label=label, drawstyle="steps-post" if meta.get("sampling") == "sampled" else "default")

    if args.dsp:
        ax.axvspan(-15, 0, color="0.85", zorder=0, label="signal band [2^-15, 1)")
        for n in (256, 1024, 4096):
            g = math.log2(n)
            ax.axvline(g, color="0.5", linestyle=":", linewidth=1)
            ax.text(g, ax.get_ylim()[1] * 0.98, f" N={n}", rotation=90, va="top", fontsize=8, color="0.35")
        # decimals d at the start of a binade = log10(2) * (bits + 1)  ->  SQNR = 6.02 * bits
        sqnr = ax.secondary_yaxis("right", functions=(lambda d: 6.02 * (d / math.log10(2.0) - 1.0),
                                                     lambda s: (s / 6.02 + 1.0) * math.log10(2.0)))
        sqnr.set_ylabel("SQNR (dB), 6.02 dB per fraction bit")

    if args.xmin is not None or args.xmax is not None:
        ax.set_xlim(args.xmin, args.xmax)
    ax.set_ylim(bottom=0.0)
    ax.set_xlabel("log2 magnitude")
    ax.set_ylabel("decimals of accuracy")
    ax.set_title(args.title)
    ax.grid(True, alpha=0.3)
    ax.legend(loc="best", fontsize=9)
    fig.tight_layout()
    fig.savefig(args.output)
    print(f"wrote {args.output}")


if __name__ == "__main__":
    main()
