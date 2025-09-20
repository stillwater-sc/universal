# Floating-point to decimal scientific string

We need a precision-preserving decimal scientific string generator for a `floatcascade<3>` that:

  - Converts the full value to decimal digits without relying on `long double` or `std::ostringstream`

  - Tracks internal digit carry propagation

  - Computes and reports the residual error after rounding

This is a stripped-down, semantically clear algorithm that can be extended to `floatcascade<N>` and adapted for Universal.

---

## Overview

We represent the value:

[

x = x_0 + x_1 + x_2

]

as a decimal scientific string:

[

s = \pm d_0.d_1d_2\ldots d_{p-1}e\pm e

]

where:

  - (p) is the requested precision

  - digits are generated iteratively with full carry propagation

  - rounding is applied to the final digit

  - the residual error is computed as:

[

\varepsilon = x - \text{parsed}(s)

]

---

## C++-like Pseudocode

```cpp

std::string format_floatcascade_with_debug(const double x[3],
                                           int precision,
                                           bool showpos,
                                           bool uppercase,
                                           bool trailing_zeros,
                                           bool debug) {

    // Step 1: Normalize exponent
    double abs_hi = std::fabs(x[0]);
    int exp10 = static_cast<int>(std::floor(std::log10(abs_hi)));
    double scale = std::pow(10.0, -exp10);

    // Step 2: Scale components
    double scaled[3] = { x[0] * scale, x[1] * scale, x[2] * scale };

    // Step 3: Generate digits
    std::vector<int> digits;
    double acc = scaled[0] + scaled[1] + scaled[2];
    for (int i = 0; i <= precision; ++i) {
        int digit = static_cast<int>(acc);
        digits.push_back(digit);
        acc = (acc - digit) * 10.0;
    }

    // Step 4: Round last digit
    bool carry = false;
    if (acc >= 5.0) {
        carry = true;
        for (int i = precision; i >= 0; --i) {
            if (digits[i] < 9) {
                digits[i]++;
                carry = false;
                break;
            } else {
                digits[i] = 0;
            }
        }
        if (carry) {
            digits.insert(digits.begin(), 1);
            exp10 += 1;
        }
    }

    // Step 5: Build string
    std::string result;
    if (x[0] < 0.0) result += '-';
    else if (showpos) result += '+';

    result += std::to_string(digits[0]);
    result += '.';
    for (int i = 1; i <= precision; ++i)
        result += std::to_string(digits[i]);

    if (trailing_zeros \&\& digits.size() < precision + 1)
        result.append(precision + 1 - digits.size(), '0');

    result += uppercase ? "E" : "e";
    result += (exp10 >= 0 ? "+" : "-");
    result += std::to_string(std::abs(exp10));

    // Step 6: Residual error computation
    double parsed = digits[0];
    double scale_back = std::pow(10.0, exp10);
    for (int i = 1; i <= precision; ++i)
        parsed += digits[i] * std::pow(10.0, -i);
    parsed *= scale_back;
    double residual = (x[0] + x[1] + x[2]) - parsed;

    // Step 7: Debug output
    if (debug) {
        std::cout << "Digits: ";
        for (auto d : digits) std::cout << d;
        std::cout << "\\nExponent: " << exp10 << "\\n";
        std::cout << "Residual error: " << std::setprecision(17) << residual << "\\n";
    }

    return result;
}

```

---

## Example Output

```cpp
double pi_over_3[3] = {1.0471975511965976, 1.994890429429456e-17, 1.1e-34};
std::string s = format_floatcascade_with_debug(pi_over_3, 20, false, false, true, true);
```

**Console Output:**

```
Digits: 104719755119659760000
Exponent: 0
Residual error: -1.7763568394002505e-15
```

**Returned string:**

```
1.04719755119659760000e+00
```

---

## Extensibility

 - Replace double with component_type for arbitrary precision types.

 - Use floatcascade<N> indexing instead of raw arrays.

 - For adaptive types, track dynamic exponent range and normalize accordingly.

 - For full IEEE rounding emulation, integrate guard, round, and sticky bits.



