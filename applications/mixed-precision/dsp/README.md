# examples/dsp

Digital Signal Processing examples

## How to build

The examples are automatically build by cmake.

## FIR filter

This is a Finite Impulse Response filter using posits that are custom fitted to an AD converter acquisition pipeline. 
It is a demonstration of the benefits of custom posit configurations and the simplest example of error-free execution.


## Precision profiles: choosing a number type

`precision_profiles.cpp` compares the decimals of accuracy, -log10(ulp / (2|x|)), of int, fixed-point, float, lns and bposit types at 16 and 32 bits. It reports them across every magnitude, and at the points a DSP pipeline cares about:
- the signal band [0.5, 1);
- small coefficients at 2^-15;
- FFT growth at 2^10.

Run it with an output directory to write one CSV per type, and plot them with `tools/notebooks/plot_precision_profiles.py --dsp`. The results and what they imply for type selection are written up in [bposit.md](../../../docs/number-systems/bposit.md#choosing-a-b-posit-for-dsp-and-fft-pipelines).
