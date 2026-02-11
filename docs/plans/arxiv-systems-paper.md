# Plan: Write Full Draft of Systems Paper

## Context

The LaTeX scaffolding exists at `papers/systems-paper/paper/main.tex` with `% TODO` placeholders throughout. The user wants a **complete reviewable draft** with real prose replacing all placeholders. The paper must highlight **at least 6 application domains** where mixed-precision yields 2x+ benefits in energy, bandwidth, performance, latency, or cost — with AI inference being just one example among many.

## 8 Application Domains (AI inference = 1 of 8)

| # | Domain | Benefit Type | Quantitative Claim |
|---|--------|-------------|-------------------|
| 1 | **Deep Learning Inference** | Throughput, energy | INT8 quantization: 4x bandwidth, 11x energy reduction vs FP32 |
| 2 | **Embedded Vision / Computational Photography** | Bandwidth, power | 4K@60fps: 2.6x bandwidth, 3.3x power savings (12-bit avg vs 32-bit) |
| 3 | **Scientific Data Compression (Climate/Weather)** | Storage, bandwidth | ZFP at 8 bpv: 4x compression, 53 dB SNR on simulation grids |
| 4 | **Robotics / Autonomous Systems** | System power | Sensor-to-actuator: 6-16 bit inputs, mixed pipeline cuts power 3x |
| 5 | **Iterative Solvers in HPC** | Energy, time | 3-precision IR: FP16 factor + FP32 work + FP64 residual, 3.5x energy savings |
| 6 | **Signal Processing / DSP** | Bandwidth, latency | FIR on 12-bit ADC data: posit<16,1> matches float at 2x bandwidth savings |
| 7 | **Financial / Low-Latency Computing** | Latency, determinism | Smaller types reduce cache pressure; fixed-point eliminates rounding variance |
| 8 | **Edge AI / IoT Sensor Networks** | Battery life | INT8 FMA 26.5x less energy than FP64; enables years vs months of battery |

## Revised Section Structure

```
1. Introduction (~1.5 pages)
   - Precision mismatch argument (Bailey inversion + Horowitz energy data)
   - Preview all 8 use cases in a motivating paragraph
   - Three contributions

2. Background and Related Work (~1.5 pages)
   - 2.1 Number systems taxonomy
   - 2.2 Block floating-point formats (MX, NVFP4, ZFP specs)
   - 2.3 Related libraries (MPFR, FloatX, ZFP, microxcaling)
   - 2.4 Mixed precision in numerical linear algebra

3. Library Architecture (~2 pages)
   - 3.1 Plug-in replacement pattern (with code listing)
   - 3.2 Number system categories (table of 37 types)
   - 3.3 Block format design (unified API)

4. Block Format Implementations (~2.5 pages)
   - 4.1 mxblock (OCP MX v1.0) — math + storage layout
   - 4.2 nvblock (NVIDIA NVFP4) — two-level scaling
   - 4.3 zfpblock (ZFP codec) — transform pipeline
   - 4.4 Unified API comparison (populated table)
   - 4.5 Quantization accuracy comparison (RMSE/SNR table)

5. Application Domains (~3 pages) ← NEW SECTION
   - 5.1 Deep learning inference
   - 5.2 Embedded vision and computational photography
   - 5.3 Scientific data compression
   - 5.4 Robotics and autonomous systems
   - 5.5 High-performance computing: iterative solvers
   - 5.6 Signal processing and DSP
   - 5.7 Financial and low-latency computing
   - 5.8 Edge AI and IoT sensor networks
   - Energy cost summary table (Horowitz/Skylake data)

6. Mixed-Precision Solver Case Studies (~2.5 pages)
   - 6.1 Iterative refinement (Carson & Higham LU-IR)
   - 6.2 Conjugate gradient (SPD systems)
   - 6.3 IDR(s) (non-symmetric systems)
   - 6.4 Cross-cutting analysis

7. Discussion (~1 page)
   - When to use which format/type
   - Limitations (software emulation, energy estimates not measurements)
   - The precision selection problem

8. Conclusion (~0.5 pages)

Appendix A: Number System Inventory (37 types table)
```

## File to Modify

**`papers/systems-paper/paper/main.tex`** — Complete rewrite of body content:
- Keep LaTeX preamble (packages, lstset, hypersetup)
- Keep title/author block
- Replace ALL `% TODO` comments with actual prose
- Uncomment and populate all tables
- Add code listings
- Cite all 29 bib entries at least once
- Add Section 5 "Application Domains" with 8 subsections

## Key Data Sources

- **Energy costs**: Horowitz 2014 (45nm baseline) + Skylake model from `include/sw/universal/energy/`
- **Sensor table**: Position paper draft Section 2.1 (sensor ENOB data)
- **Block format accuracy**: Benchmark data (mxfp4/nvfp4/zfp RMSE/SNR values)
- **Vision pipeline**: Position paper Section 2.3 (4K@60fps analysis)
- **Solver results**: Placeholder tables with representative data; marked with footnote that exact numbers come from running case study code

## Verification

1. Python brace-balance check
2. All `\begin`/`\end` pairs match
3. No remaining `% TODO` lines
4. All 29 bib keys cited
5. At least 8 use case domains with quantitative 2x+ claims
