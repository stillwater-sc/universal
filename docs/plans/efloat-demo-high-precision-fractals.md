# `efloat` Demonstration: High-Precision Fractal Visualization

*   **Related Epic**: [efloat-implementation-epic.md](efloat-implementation-epic.md)

## Goal

Create a visually compelling demonstration of `efloat`'s power by rendering a deep zoom of the Mandelbrot set, a region where `double` precision is insufficient.

## Demonstration

1.  **The Mandelbrot Set**: The set is defined by the behavior of the sequence $z_{n+1} = z_n^2 + c$, where `c` is a complex number. A point `c` is in the set if the sequence does not escape to infinity.

2.  **The "Deep Zoom" Problem**: As one zooms deeper into the fractal, the distance between the complex coordinates of adjacent pixels becomes incredibly small. Eventually, this distance becomes smaller than the machine epsilon of `double`, and the entire image becomes pixelated or blocky because the `double` type can no longer distinguish between the points.

3.  **Implementation**:
    *   Create a program that renders the Mandelbrot set to a PPM or other simple image file.
    *   The program should take as input the center coordinates and the width of the view window.
    *   **Step 1 (Double Precision Failure)**:
        *   Choose a deep zoom target (e.g., a region in the "Seahorse Valley").
        *   Render the view using `complex<double>`. The output image should look blocky and corrupted.
    *   **Step 2 (efloat Success)**:
        *   Render the exact same view, but use `complex<efloat>` with high precision (e.g., 256 bits) for the coordinates and calculations.
        *   The output image should reveal the intricate and beautiful fractal details that were lost in the `double` version.

## Expected Outcome

The user will be able to see two images:
1.  A pixelated, low-detail image showing the failure of `double` precision at extreme zoom levels.
2.  A crisp, detailed image of the same region, rendered correctly using `efloat`.

This provides a powerful and intuitive visual argument for the necessity of arbitrary-precision arithmetic.

## Acceptance Criteria

*   A C++ source file is created in `applications/efloat_demonstrations/high_precision_fractals.cpp`.
*   The program can generate two images (e.g., `mandelbrot_double.ppm` and `mandelbrot_efloat.ppm`) representing the same deep zoom view.
*   The `README.md` for the demonstration explains how to run the program and what the expected visual difference is.
