# Adaptive-precision demonstrations (`efloat`)

Small, self-contained programs that show where fixed-precision `double` breaks
down and adaptive-precision `efloat` keeps computing correctly. Each runs the
**same templated kernel** on both `double` and `efloat` -- the only difference
is the working precision.

| Program | What it shows |
|---------|---------------|
| `catastrophic_cancellation` | `(1-cos x)/x^2` for small `x`: `double` cancels to 0, `efloat` returns 1/2 (#1096) |
| `ill_conditioned_systems`   | Hilbert-matrix solve: `double` error ~100% at n=12, `efloat` is exact (#1097) |
| `high_precision_fractals`   | Mandelbrot deep zoom: `double` pixelates, `efloat` resolves the detail (#1098) |
| `mathematical_identities`   | BBP series for pi: `double` stalls at ~15 digits, `efloat` verifies ~150 (#1099) |
| `polynomial_roots`          | Wilkinson's polynomial: `double` roots drift off the integers, `efloat` recovers them (#1100) |

## High-precision fractal visualization

`high_precision_fractals.cpp` renders a deep-zoom view of the Mandelbrot set
(`z_{n+1} = z_n^2 + c`) around a spiral in the seahorse-valley neighbourhood.

At the chosen zoom the pixel spacing is `dx ~ 3.75e-17`. That is only about
`0.34` of a `double` ulp at `CENTER_X ~ -0.73` (ulp `~1.11e-16`), so **`double`
cannot give adjacent columns distinct x coordinates** -- about two thirds of the
columns collapse onto their neighbour and the image degrades into blocky
vertical bands. (`CENTER_Y ~ 0.19` sits in a smaller binade, ulp `~2.78e-17`, so
`dx` is `~1.35` ulp there and the y coordinates stay distinct -- the collapse is
x-only.) The `efloat` (256-bit) render computes every pixel's coordinate exactly
and reveals the true fractal structure.

### Build and run

Configure with applications enabled, then run the target:

```bash
cmake -DUNIVERSAL_BUILD_APPLICATIONS=ON ..
make adaptive_high_precision_fractals
./applications/precision/adaptive/adaptive_high_precision_fractals
```

It prints a quantitative summary and writes two images in the current directory:

- `mandelbrot_double.ppm` -- the `double` render (blocky / corrupted)
- `mandelbrot_efloat.ppm` -- the `efloat` render (correct)

View them with any image viewer, or convert to PNG:

```bash
# ImageMagick
convert mandelbrot_efloat.ppm mandelbrot_efloat.png
# or Python / Pillow
python3 -c "from PIL import Image; Image.open('mandelbrot_efloat.ppm').save('mandelbrot_efloat.png')"
```

### Expected output

```text
  pixel spacing dx = 3.750e-17   double ulp at CENTER_X = 1.110e-16   dx/ulp = 0.34
  efloat (oracle) escape range: [295, 1000]  <- real fractal structure
  pixels where double disagrees with efloat: 4835 / 6400  (75.5%)
```

About **75% of pixels** in the `double` image are wrong relative to the `efloat`
oracle. Side by side, the `efloat` image shows coherent fractal filaments while
the `double` image is broken into vertical bands where the x coordinates
collapsed.

### Tuning

The view is controlled by the constants at the top of the source
(`CENTER_X`, `CENTER_Y`, `VIEW_W`, `IMG_W`, `IMG_H`, `MAXITER`). The defaults
keep the render fast (~2 s for the `efloat` pass); raise `IMG_W` / `IMG_H` /
`MAXITER` for a higher-resolution picture (the `efloat` render time grows
roughly linearly with the pixel count and iteration budget).

## Polynomial root finding (Wilkinson's polynomial)

`polynomial_roots.cpp` finds the roots of **Wilkinson's polynomial**
`W(x) = (x-1)(x-2)...(x-20)`, whose roots are obviously the integers 1..20.

The catch is in the *coefficients*. Expanded into power form, `W` has enormous
coefficients (up to ~1.4e19); several exceed `2^53`, so they cannot be stored
exactly in `double`. Wilkinson's polynomial is the textbook example of a problem
whose roots are **hypersensitive to coefficient perturbation** -- a tiny change
in a coefficient moves the roots substantially (the middle roots most of all).

The program expands `W` exactly in `efloat`, rounds the coefficients to `double`,
and then runs the *same* Newton kernel from each true root location `i`, asking:
is `i` still a root of the stored polynomial?

- With **exact `efloat` coefficients**, yes -- Newton stays at `i`; every root is
  recovered to full precision.
- With **rounded `double` coefficients**, no -- the roots have moved, and Newton
  drifts away from the integers by up to ~`1e-2` (worst for the middle roots
  13..17), so `double` cannot recover the integer roots.

### Build and run

```bash
cmake -DUNIVERSAL_BUILD_APPLICATIONS=ON ..
make adaptive_polynomial_roots
./applications/precision/adaptive/adaptive_polynomial_roots
```

The output prints the largest coefficient rounding error, then a table of all 20
roots with the `double` and `efloat` results side by side and their distance from
the true integer. `double`'s worst error is ~`1e-2`; `efloat`'s is `0`.
