# Mixed-precision algorithm design and optimization

## Computational Mathematics
When designing custom compute solutions significant benefits in terms of performance and power efficiency are
possible by exploiting the different arithmetic requirements of the control flow and the data flow.
Even within the data flow we observe many different requirements with respect to precision 
and required dynamic range of the arithmetic operations. 

The goal of mixed-precision algorithm design and optimization is to formally and structurally identify and
explot opportunities to right-scale the number systems used for key computational paths that represent the
bottleneck of the execution. When these algorithms are incorporated in embedded devices and custom hardware
engines, we can guarantee optimal performance and power efficiency.

In mixed-precision algorithms, we need to study the computational mathematics and quantify the following
metrics:

- accuracy
- efficiency
- robustness
- stability

## Progression of computational algorithms for science and engineering

- root finding
- interpolation
- integration
- approximation
- fourier transforms
- ODE solvers
- linear-algebra

