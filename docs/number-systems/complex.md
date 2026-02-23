# Complex: Complex Number Arithmetic for Universal Types

## Why

Scientific computing, signal processing, and quantum mechanics all require complex number arithmetic. The C++ standard library provides `std::complex<T>`, but it only works with `float`, `double`, and `long double`. If you want complex arithmetic with posits, cfloats, fixed-point, or any other Universal number type, you need a complex type that is generic over the scalar.

The Universal `complex<T>` type provides complex number arithmetic for any Universal scalar type. This lets you use the same complex algorithms with posit precision, bfloat16 storage efficiency, or fixed-point determinism -- simply change the scalar type.

## What

`complex<T>` is a complex number generic over any Universal scalar:

| Parameter | Type | Description |
|-----------|------|-------------|
| `T` | typename | Any arithmetic type (posit, cfloat, fixpnt, bfloat16, dd, qd, etc.) |

### Internal Structure

```cpp
template<typename T>
class complex {
    T _re;  // Real part
    T _im;  // Imaginary part
};
```

### Key Properties

- **Generic scalar**: works with any Universal number type
- **Standard complex API**: `real()`, `imag()`, `abs()`, `arg()`, `conj()`, `norm()`
- **Full arithmetic**: +, -, *, / with proper complex multiplication rules
- **Math functions**: `exp()`, `log()`, `sin()`, `cos()`, `sqrt()`, `pow()`
- **Drop-in replacement**: same API as `std::complex<T>` but for Universal types

## How to Use It

### Include

```cpp
#include <universal/number/posit/posit.hpp>  // or any scalar type
#include <universal/math/complex.hpp>
using namespace sw::universal;
```

### Complex Posit Arithmetic

```cpp
using Real = posit<32, 2>;
using Complex = complex<Real>;

Complex z1(Real(1.0), Real(2.0));   // 1 + 2i
Complex z2(Real(3.0), Real(4.0));   // 3 + 4i

Complex sum = z1 + z2;              // 4 + 6i
Complex product = z1 * z2;          // -5 + 10i
Complex quotient = z1 / z2;         // 0.44 + 0.08i

std::cout << "|z1| = " << abs(z1) << std::endl;      // sqrt(5)
std::cout << "arg(z1) = " << arg(z1) << std::endl;    // atan2(2,1)
std::cout << "conj(z1) = " << conj(z1) << std::endl;  // 1 - 2i
```

### FFT with Custom Precision

```cpp
template<typename Complex>
void fft(std::vector<Complex>& x) {
    // Cooley-Tukey FFT works with any complex type
    using Real = typename Complex::value_type;
    size_t N = x.size();
    if (N <= 1) return;
    // ... standard FFT butterfly operations ...
}

// FFT with posit precision
std::vector<complex<posit<32, 2>>> signal_posit(1024);
fft(signal_posit);

// FFT with double-double precision
std::vector<complex<dd>> signal_dd(1024);
fft(signal_dd);
```

## Problems It Solves

| Problem | How complex Solves It |
|---------|-----------------------|
| `std::complex` only works with float/double/long double | Generic over any Universal scalar type |
| Need complex FFT with posit precision | `complex<posit<32,2>>` as drop-in replacement |
| Signal processing with custom floating-point | Complex arithmetic for cfloat, bfloat16, etc. |
| Quantum computation needs high-precision complex numbers | `complex<dd>` or `complex<qd>` for extended precision |
| Complex fixed-point for DSP | `complex<fixpnt<16,8>>` for deterministic DSP |
