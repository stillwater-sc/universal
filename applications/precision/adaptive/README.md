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
