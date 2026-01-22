# Position Paper Analysis: Mixed-Precision Algorithm Design for Physical-World Computing

## Executive Summary

This document analyzes David Bailey's influential paper on high-precision arithmetic, inventories the current state of the Universal library, and proposes a framework for a position paper arguing for mixed-precision algorithm design as the key enabler for all computing that interfaces with the physical world—from data acquisition and signal processing to real-time control and embodied AI.

**Key Insight**: Mixed-precision is not merely an optimization for AI workloads. It is the *natural* computational paradigm for any system that interfaces with the physical world, where sensor measurements inherently arrive as small integers (6-16 bits), and outputs drive actuators with finite resolution. The mismatch between 64-bit general-purpose arithmetic and the actual information content of physical-world data represents an enormous waste of energy, bandwidth, and silicon area.

---

## Part 1: Analysis of Bailey's "High-Precision Floating-Point Arithmetic in Scientific Computation"

### Paper Structure and Rhetorical Strategy

Bailey's 2005 paper in *Computing in Science and Engineering* follows a classic position paper structure:

1. **Establish the Status Quo**: IEEE 64-bit floating-point is sufficient for *most* scientific applications
2. **Identify the Gap**: A "rapidly growing body" of important applications requires higher precision
3. **Categorize the Need**: Three tiers of precision requirements:
   - Double the standard (double-double, ~32 decimal digits)
   - Four times standard (quad-double, ~64 decimal digits)
   - Hundreds or more digits (arbitrary precision)
4. **Provide Compelling Examples**: Real scientific applications where high-precision yielded new results
5. **Present the Solution**: Software packages (QD library) with high-level language support
6. **Demonstrate Impact**: New scientific discoveries enabled by the approach

### Key Applications Cited by Bailey

| Domain | Application | Precision Need |
|--------|-------------|----------------|
| Climate Science | Climate modeling, atmospheric simulations | Reproducibility, stability |
| Astrophysics | Supernova simulations | Accuracy over long time scales |
| Quantum Physics | Coulomb N-body atomic systems | Fine structure constant studies |
| Particle Physics | Quark/gluon/boson scattering amplitudes | Precision QCD calculations |
| Mathematics | Experimental mathematics, integral evaluation | Discovery of closed forms |
| Dynamical Systems | Periodic orbits, separatrix splitting | Chaos detection |

### Bailey's Core Thesis

> "In the future, the numeric precision used for a scientific computation may be as important to the program design as are the algorithms and data structures."

### Why Bailey's Paper Succeeded

1. **Timing**: Growing computational power made high-precision feasible
2. **Concrete Results**: New scientific discoveries, not just theoretical benefits
3. **Accessible Tools**: QD library with Fortran-90 and C++ interfaces
4. **Diverse Applications**: Cross-disciplinary appeal
5. **Clear Categorization**: Helped readers identify their own precision needs

---

## Part 1B: The Physical World Interface—The Fundamental Driver for Mixed-Precision

### The Measurement Reality

Every computational system that interfaces with the physical world begins with **data acquisition**. The precision of this data is fundamentally constrained by physics, not by computational convenience:

| Sensor Type | Typical Resolution | Effective Bits | Physical Limitation |
|-------------|-------------------|----------------|---------------------|
| **ADC (Audio)** | 16-24 bit | 12-20 ENOB | Thermal noise, DNL/INL |
| **ADC (Industrial)** | 12-16 bit | 10-14 ENOB | Speed/resolution tradeoff |
| **ADC (High-speed)** | 8-12 bit | 6-10 ENOB | Bandwidth limits resolution |
| **Image Sensor (Consumer)** | 10-12 bit | 8-10 effective | Shot noise, read noise |
| **Image Sensor (Scientific)** | 14-16 bit | 12-14 effective | Dark current, well depth |
| **LIDAR** | 8-12 bit | 8-10 effective | Photon counting statistics |
| **RADAR** | 12-14 bit | 10-12 effective | Phase noise, clutter |
| **IMU (Accelerometer)** | 16 bit | 12-14 effective | Vibration, bias drift |
| **IMU (Gyroscope)** | 16 bit | 10-14 effective | Angular random walk |
| **Temperature** | 12-16 bit | 10-14 effective | Sensor nonlinearity |
| **Pressure** | 12-24 bit | 10-20 effective | Hysteresis, temperature |
| **Force/Torque** | 12-16 bit | 10-14 effective | Strain gauge noise |

**Key Observation**: No physical sensor delivers more than ~20 effective bits of information. Processing this data with 64-bit floating-point arithmetic wastes 3-4x the necessary bits at every operation.

### The Signal Processing Pipeline

Physical-world data flows through a processing pipeline where each stage has distinct precision requirements:

```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│   Sensor    │───▶│   Signal    │───▶│   Feature   │───▶│  Decision/  │───▶│  Actuator   │
│ Acquisition │    │Conditioning │    │ Extraction  │    │  Control    │    │   Output    │
│   6-16 bit  │    │  8-24 bit   │    │  16-32 bit  │    │  16-64 bit  │    │   8-16 bit  │
└─────────────┘    └─────────────┘    └─────────────┘    └─────────────┘    └─────────────┘
```

#### Stage 1: Sensor Acquisition (6-16 bits)

Raw sensor data arrives as small integers. Operations at this stage should use native integer or small fixed-point arithmetic:

| Operation | Optimal Representation | Rationale |
|-----------|----------------------|-----------|
| Dark frame subtraction | Integer (sensor width) | Exact, no precision loss |
| Offset correction | Integer + small constant | Preserve LSBs |
| Gain normalization | Fixed-point (sensor width + guard bits) | Avoid truncation |
| Decimation/averaging | Integer accumulator | Preserve full precision |

#### Stage 2: Signal Conditioning (8-24 bits)

Filtering, interpolation, and cleanup operations. Precision grows modestly:

| Operation | Optimal Representation | Rationale |
|-----------|----------------------|-----------|
| **FIR Filtering** | Fixed-point with accumulator | Coefficients typically 8-16 bit |
| **IIR Filtering** | Fixed-point or 16-bit float | Feedback requires care |
| **Bayer Demosaicing** | 10-14 bit fixed-point | Interpolation of raw pixel data |
| **Missing Pixel Interpolation** | Native sensor width + 2 bits | Local neighborhood operation |
| **Noise Reduction** | 12-16 bit | Spatial/temporal averaging |
| **Gamma Correction** | LUT or 16-bit LNS | Nonlinear, compressive |
| **Resampling/Interpolation** | 16-24 bit fixed or float | Polynomial evaluation |
| **DC Removal** | Accumulator + subtraction | Running average |
| **Detrending** | 16-32 bit float | Polynomial fitting |

#### Stage 3: Feature Extraction (16-32 bits)

Transform-domain and statistical operations. This is where precision requirements grow:

| Operation | Optimal Representation | Rationale |
|-----------|----------------------|-----------|
| **FFT** | 16-32 bit float or fixed | Twiddle factors, butterfly |
| **DCT/DWT** | 16-32 bit float | Transform coefficients |
| **Correlation** | 32-bit accumulator | Sum of products |
| **Histogram** | Integer counters | Exact counting |
| **Moments (mean, variance)** | 32-48 bit accumulator | Running statistics |
| **Edge Detection** | 16-bit fixed | Gradient computation |
| **Morphological Ops** | Binary or small integer | Min/max operations |
| **Template Matching** | 16-32 bit accumulator | Sum of absolute differences |

#### Stage 4: Decision/Control (16-64 bits)

Higher-level processing where precision requirements are most variable:

| Operation | Optimal Representation | Rationale |
|-----------|----------------------|-----------|
| **Classification** | 8-16 bit quantized | Neural network inference |
| **State Estimation (Kalman)** | 32-bit float | Matrix operations, stability |
| **Sensor Fusion** | 32-bit float | Covariance propagation |
| **PID Control** | 16-32 bit fixed | Deterministic, bounded |
| **Model Predictive Control** | 32-64 bit float | Optimization, conditioning |
| **Trajectory Planning** | 32-64 bit float | Numerical integration |
| **Inverse Kinematics** | 32-bit float | Trigonometric, Jacobian |

#### Stage 5: Actuator Output (8-16 bits)

The output stage returns to low precision constrained by physical actuators:

| Actuator Type | Typical Resolution | Output Precision Needed |
|---------------|-------------------|------------------------|
| **PWM Motor Control** | 8-12 bit | 10-14 bit (dithering) |
| **DAC (Audio)** | 16-24 bit | 16-24 bit |
| **DAC (Industrial)** | 12-16 bit | 12-18 bit |
| **Servo Position** | 10-16 bit | 12-18 bit |
| **Stepper Motor** | Discrete steps | Integer step count |
| **Valve/Solenoid** | Often binary | 1-8 bit |
| **Display (per channel)** | 8-10 bit | 8-12 bit |

### Image Processing: A Case Study in Mixed-Precision

Image processing exemplifies the mixed-precision opportunity:

#### Raw Image Pipeline

```
Raw Sensor    Linearization    White Balance    Demosaic    Color Correct    Gamma    Output
 10-14 bit  →   12-16 bit   →    12-16 bit   →  12-16 bit →   16-24 bit   → 8-12 bit → 8-10 bit
 (integer)    (fixed-point)   (fixed-point)  (fixed-pt)   (float/fixed)    (LUT)    (integer)
```

| Processing Stage | Current Practice | Optimal Practice | Savings |
|-----------------|------------------|------------------|---------|
| **Bayer Demosaic** | 32-bit float | 12-bit fixed-point | 2.7x memory, 4x compute |
| **White Balance** | 32-bit float | 16-bit fixed-point | 2x memory, 3x compute |
| **Color Matrix** | 32-bit float | 16-bit fixed-point | 2x memory, 3x compute |
| **Noise Reduction** | 32-bit float | 16-bit fixed-point | 2x memory, 2x compute |
| **Sharpening** | 32-bit float | 12-bit fixed-point | 2.7x memory, 3x compute |
| **Gamma/Tone** | 32-bit float | LUT (8→12 bit) | 10x+ compute |

#### Computational Photography

Modern computational photography stacks multiple exposures and applies complex algorithms:

| Algorithm | Frames | Per-Frame Precision | Accumulator Precision | Output |
|-----------|--------|--------------------|-----------------------|--------|
| **HDR Merge** | 3-9 | 12-14 bit | 20-24 bit | 10-16 bit |
| **Focus Stack** | 5-20 | 10-12 bit | 16-20 bit | 10-12 bit |
| **Super Resolution** | 4-16 | 10-12 bit | 16-24 bit | 12-16 bit |
| **Temporal Denoise** | 4-8 | 10-12 bit | 16-20 bit | 10-12 bit |
| **Burst Photography** | 8-16 | 10-12 bit | 16-24 bit | 10-12 bit |

### Real-Time Control Systems

Control systems have stringent timing and determinism requirements that favor fixed-point:

#### Precision Requirements by Control Loop Rate

| Loop Rate | Typical Application | Precision Strategy |
|-----------|--------------------|--------------------|
| **>100 kHz** | Motor commutation, power electronics | 8-16 bit fixed-point, hardware |
| **10-100 kHz** | Current control, servo loops | 16-bit fixed-point |
| **1-10 kHz** | Position/velocity control | 16-32 bit fixed-point |
| **100-1000 Hz** | Robot joint control | 32-bit fixed or float |
| **10-100 Hz** | Mobile robot navigation | 32-bit float |
| **1-10 Hz** | High-level planning | 32-64 bit float |

#### Control Algorithm Precision Analysis

| Algorithm | Computation | Accumulation Risk | Recommended Precision |
|-----------|-------------|-------------------|----------------------|
| **PID Controller** | 3 multiplies, 2 adds | Integral windup | 16-32 bit fixed, guarded integrator |
| **Lead/Lag Compensator** | IIR filter | Limit cycle oscillation | 16-32 bit fixed with proper scaling |
| **State Space** | Matrix multiply | State growth | 32-bit fixed or float |
| **LQR/LQG** | Matrix operations | Riccati stability | 32-64 bit float |
| **MPC** | QP solver | Ill-conditioning | 32-64 bit float |
| **Sliding Mode** | Switching logic | Chattering | 16-32 bit, careful discontinuity |

### Audio and Communication Signal Processing

#### Audio Processing Chain

```
Microphone   Preamp/ADC   Filtering   Processing   Mixing   DAC/Amp   Speaker
  Analog   →  16-24 bit  → 24-32 bit → 24-48 bit → 32-48 bit → 16-24 bit → Analog
                                         ↓
                              (Dynamics, EQ, Effects)
```

| Audio Operation | Minimum Precision | Headroom Needed | Recommended |
|-----------------|-------------------|-----------------|-------------|
| **Gain/Volume** | 16 bit | +24 dB | 24-bit fixed |
| **EQ (IIR)** | 24 bit | Coefficient sensitivity | 32-bit float or 24-bit fixed |
| **Dynamics (Compressor)** | 24 bit | Envelope detection | 32-bit float |
| **Reverb** | 24 bit | Accumulation | 32-48 bit accumulators |
| **Sample Rate Conversion** | 24 bit | Interpolation | 32-bit float |
| **Mixing (N channels)** | 24 bit | log2(N) bits headroom | 32-48 bit accumulator |

#### Communications/Software-Defined Radio

| Processing Block | Input Bits | Processing Precision | Output Bits |
|------------------|------------|---------------------|-------------|
| **ADC Samples** | 8-16 | — | — |
| **DDC (NCO + Filter)** | 8-16 | 16-18 bit complex | 16 bit I/Q |
| **Channel Filter** | 16 | 24-32 bit | 16 bit |
| **Timing Recovery** | 16 | 32 bit (interpolator) | 16 bit |
| **Carrier Recovery** | 16 | 16-32 bit (PLL) | Phase estimate |
| **Equalization** | 16 | 16-32 bit | 16 bit |
| **Symbol Detection** | 16 | 16-24 bit (soft) | 3-8 bit LLR |
| **Error Correction** | 3-8 | Integer (Viterbi/LDPC) | 1 bit |

### Industrial and Scientific Instrumentation

| Instrument Type | Measurement Precision | Processing Needs | Output Precision |
|-----------------|----------------------|------------------|------------------|
| **Oscilloscope** | 8-12 bit @ GHz | Triggering, FFT | 8-12 bit display |
| **Spectrum Analyzer** | 12-16 bit | FFT, averaging | 0.01 dB accuracy |
| **Lock-in Amplifier** | 16-24 bit | Correlation, averaging | Sub-ppm sensitivity |
| **Mass Spectrometer** | 12-16 bit | Peak detection, integration | ppm mass accuracy |
| **NMR/MRI** | 14-16 bit | FFT, image recon | 12-16 bit images |
| **Flow Cytometer** | 16-24 bit | Pulse detection, classification | Event counts |

### The Efficiency Opportunity

Consider a typical embedded vision system processing 4K video at 60fps:

| Approach | Bits/Pixel | Memory BW | Compute | Power |
|----------|------------|-----------|---------|-------|
| **Naive (FP32)** | 32 | 7.4 GB/s | Baseline | Baseline |
| **Mixed-Precision** | ~12 avg | 2.8 GB/s | ~0.4x | ~0.3x |
| **Efficiency Gain** | — | **2.6x** | **2.5x** | **3.3x** |

For a 10W embedded vision processor, this translates to:
- **Mixed-precision**: 10W → 3W
- **Battery life**: 3.3x improvement
- **Thermal**: Enables fanless operation
- **Cost**: Smaller memory, simpler power delivery

---

## Part 2: Proposed Position Paper Framework

### Title Options (Updated)

- "Mixed-Precision Arithmetic: The Key to Energy-Efficient Embodied Intelligence"
- "Right-Sizing Computation: Mixed-Precision Algorithms for the Embedded AI Era"
- "From High-Precision to Right-Precision: A Paradigm Shift for Embodied Computing"

### Proposed Structure (Mirroring Bailey)

#### 1. Establish the New Status Quo
- IEEE 754 single/double precision dominates general-purpose computing
- Deep learning has normalized lower precision (FP16, BF16, INT8)
- But: one-size-fits-all precision is wasteful for embedded systems

#### 2. Identify the Emerging Gap
- Embodied AI (robotics, autonomous vehicles, edge devices) faces:
  - Strict power budgets (milliwatts to watts)
  - Real-time latency requirements
  - Limited memory bandwidth
  - Thermal constraints
- Current AI models are 1000x too computationally expensive for true embedded deployment
- The 1960s parallel: computers too large to embed → computational requirements too large to embed

#### 3. The Mixed-Precision Thesis

> "For embodied and embedded intelligence, the optimal precision for each computational kernel is as important to system efficiency as the choice of algorithm itself. Mixed-precision design—using the minimum precision that achieves required accuracy—is the key to achieving the 100-1000x efficiency gains needed for truly autonomous embedded systems."

#### 4. Categorize Precision Requirements by Task

| Task Category | Precision Need | Rationale |
|---------------|----------------|-----------|
| Sensor preprocessing | 8-16 bit fixed-point | Limited sensor resolution |
| Feature extraction | 16-bit float (posit/cfloat) | Moderate dynamic range |
| Neural network inference | 4-8 bit quantized | Proven sufficient for inference |
| Control loops | 16-32 bit fixed-point | Deterministic, bounded |
| State estimation | 32-bit float | Accumulation over time |
| Planning/optimization | 32-64 bit float | Numerical conditioning |
| Safety-critical validation | 64+ bit or interval | Verification requirements |

#### 5. Compelling Application Examples

**Autonomous Vehicles:**
- Perception: 8-bit quantized CNNs (10-100x energy savings)
- Sensor fusion: 16-bit posit (better dynamic range than float)
- Path planning: 32-bit with interval validation
- Control: Fixed-point with proven bounds

**Robotics:**
- Inverse kinematics: Mixed 16/32-bit cascade
- Force control: Fixed-point for determinism
- SLAM: Adaptive precision based on uncertainty

**Edge AI:**
- Keyword spotting: 4-bit weights sufficient
- Anomaly detection: Logarithmic number system (LNS)
- Time series: Fixed-point with accumulator management

**Scientific Instruments:**
- Spectral analysis: Double-double for accumulation
- Signal processing: Mixed fixed/float pipelines
- Calibration: High-precision offline, low-precision online

#### 6. The Universal Solution

Position Universal as the comprehensive toolkit for:
- Exploring precision/accuracy tradeoffs
- Prototyping mixed-precision algorithms
- Validating numerical properties
- Generating specifications for custom hardware

#### 7. Demonstrate Impact

- Energy efficiency benchmarks across number systems
- Accuracy preservation with reduced precision
- Case studies from applications/ directory
- Hardware co-design examples

---

## Part 3: Universal Library Inventory

### Number Systems Available (37 implementations)

#### Static (Fixed-Size) - Hardware Targetable

| Category | Types | Use Case |
|----------|-------|----------|
| **Tapered Float** | posit, posito, posit2, takum | Dynamic range efficiency |
| **Classic Float** | cfloat, bfloat16, areal | IEEE-compatible, ML accelerators |
| **Multi-component** | dd, qd, dd_cascade, td_cascade, qd_cascade | High-precision accumulation |
| **Fixed-Point** | fixpnt, integer, decimal | Deterministic embedded |
| **Logarithmic** | lns, dbns | Multiplication-heavy workloads |
| **Interval** | valid, sorn | Verified computing |
| **Rational** | rational | Exact arithmetic |

#### Elastic (Adaptive-Precision) - Software Oracles

| Type | Use Case |
|------|----------|
| einteger | Cryptography, exact integer arithmetic |
| erational | Symbolic computation |
| edecimal | Financial, decimal-exact |
| efloat | Arbitrary-precision validation |
| ereal | Multi-component adaptive |

### Mixed-Precision SDK Status

| Component | Status | Completeness |
|-----------|--------|--------------|
| Root finding | Implemented | secant, rpoly |
| Integration | Implemented | Simpson, trapezoidal |
| Interpolation | Implemented | spline |
| Approximation | Implemented | Taylor series |
| Optimization | Partial | secant method only |
| Tensor/DNN | Implemented | CG solver variants, inference |

### Benchmark Infrastructure

| Category | Focus | Position Paper Relevance |
|----------|-------|--------------------------|
| accuracy/ | Numerical accuracy | Precision/accuracy tradeoffs |
| energy/ | Energy consumption | Core efficiency argument |
| error/ | Error propagation | Understanding precision needs |
| performance/ | Speed benchmarks | Performance comparisons |
| range/ | Dynamic range | Number system selection |
| reproducibility/ | Exact results | Scientific computing needs |

### Application Examples by Domain

| Domain | Applications | Position Paper Fit |
|--------|--------------|-------------------|
| DNN/ML | MNIST, conv2d, RBF, inference | Primary: Embodied AI |
| DSP | FIR filters, ADC | Primary: Embedded systems |
| Control | ODE solvers, PDE (Laplace, CG) | Primary: Robotics |
| Science | Physics constants, chemistry | Secondary: Validation |
| Cryptography | Factorization, Pollard rho | Secondary: Security |

---

## Part 4: Gap Analysis

### Gaps for Position Paper Support

#### 1. **Energy Benchmarking Infrastructure** (Critical)
- **Current**: Limited energy/ benchmarks
- **Needed**: Comprehensive energy measurements per operation per number system
- **Action**: Expand benchmark/energy/ with standardized methodology

#### 2. **Embodied AI Application Examples** (Critical)
- **Current**: DNN inference, basic DSP
- **Needed**:
  - Complete robotics pipeline (perception → planning → control)
  - Autonomous vehicle perception stack
  - Edge AI deployment examples
- **Action**: Add applications/embodied-ai/ directory

#### 3. **Mixed-Precision Algorithm Patterns** (High)
- **Current**: Ad-hoc examples in mixedprecision/
- **Needed**:
  - Documented design patterns
  - Precision selection guidelines
  - Automatic precision tuning examples
- **Action**: Expand mixedprecision/ with pattern library

#### 4. **Hardware Co-Design Examples** (High)
- **Current**: Mentioned but not demonstrated
- **Needed**:
  - FPGA synthesis examples
  - ASIC area/power estimates
  - Custom accelerator specifications
- **Action**: Add hardware/ or codesign/ directory

#### 5. **Interval/Verified Computing** (Medium)
- **Current**: valid, sorn types exist
- **Needed**:
  - Complete interval arithmetic for safety-critical validation
  - Integration with other number systems for error bounds
- **Action**: Complete valid type, add verification examples

#### 6. **Quantization Toolkit** (Medium)
- **Current**: Basic quantization in benchmarks
- **Needed**:
  - Systematic quantization-aware training support
  - Post-training quantization tools
  - Quantization error analysis
- **Action**: Add tools/quantization/

#### 7. **Real-Time/Deterministic Guarantees** (Medium)
- **Current**: Fixed-point exists
- **Needed**:
  - WCET (Worst-Case Execution Time) analysis
  - Overflow/underflow bounds certification
  - Fixed-point scaling methodology
- **Action**: Document deterministic computing patterns

### Gaps in Number System Coverage

| Gap | Description | Priority |
|-----|-------------|----------|
| **Blocked floating-point** | For SIMD-friendly mixed precision | Medium |
| **Stochastic rounding** | For training stability | Low |
| **Gradient compression** | For distributed training | Low |
| **Posit quire integration** | Complete super-accumulator support | High |

### Documentation Gaps

| Gap | Current State | Needed |
|-----|---------------|--------|
| Mixed-precision methodology | Scattered examples | Comprehensive guide |
| Number system selection | Implicit knowledge | Decision flowchart |
| Energy optimization guide | Not documented | Best practices document |
| Hardware targeting guide | Not documented | Synthesis guidelines |

---

## Part 5: Recommended Action Plan

### Phase 1: Foundation (supports position paper writing)

1. **Create Mixed-Precision Methodology Document**
   - Document precision selection criteria
   - Catalog error propagation patterns
   - Provide decision framework

2. **Expand Energy Benchmarks**
   - Standardize energy measurement methodology
   - Benchmark all number systems uniformly
   - Create comparison tables for paper

3. **Add Embodied AI Examples**
   - Simple robotics pipeline (perception → control)
   - Edge inference example with multiple precisions
   - Sensor fusion with mixed number systems

### Phase 2: Demonstration (supports position paper arguments)

4. **Create End-to-End Mixed-Precision Case Study**
   - Pick compelling application (e.g., drone navigation)
   - Implement with uniform precision
   - Optimize with mixed precision
   - Document energy/accuracy tradeoffs

5. **Hardware Co-Design Proof of Concept**
   - Generate FPGA-synthesizable subset
   - Measure actual vs. estimated efficiency
   - Document methodology

### Phase 3: Publication

6. **Write Position Paper**
   - Follow Bailey's structure
   - Include Universal benchmarks
   - Provide concrete efficiency numbers
   - Call to action for the community

7. **Target Venue**
   - IEEE Computing in Science & Engineering (like Bailey)
   - Or: Communications of the ACM (broader reach)
   - Or: Nature Computational Science (high impact)

---

## References

- Bailey, D.H. (2005). "High-precision floating-point arithmetic in scientific computation." *Computing in Science & Engineering*, 7(3), 54-61.
- Bailey, D.H., Hida, Y., Li, X.S., Thompson, B. (2002). "ARPREC: An arbitrary precision computation package."
- Hida, Y., Li, X.S., Bailey, D.H. (2001). "Algorithms for quad-double precision floating point arithmetic."
- Omtzigt, E.T.L., Quinlan, J. (2022). "Universal Numbers Library: Multi-format Variable Precision Arithmetic Library." *Journal of Open Source Software*.

---

## Sources

- [Bailey's Paper (IEEE)](https://ieeexplore.ieee.org/document/1425396/)
- [Bailey's Papers Directory](https://www.davidhbailey.com/dhbpapers/)
- [High-precision and exascale computing](https://www.osti.gov/biblio/964380)
