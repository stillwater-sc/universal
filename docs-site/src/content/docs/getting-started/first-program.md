---
title: First Program
description: Write your first program using the Universal Numbers Library
sidebar:
  order: 3
---

This tutorial walks through writing your first program with the Universal Numbers Library.

## Hello, Universal

Create a file called `hello_universal.cpp`:

```cpp
#include <iostream>
#include <universal/number/posit/posit.hpp>

int main() {
    using namespace sw::universal;

    posit<32, 2> a, b, c;
    a = 1.5;
    b = 2.75;
    c = a + b;

    std::cout << "a = " << a << '\n';
    std::cout << "b = " << b << '\n';
    std::cout << "a + b = " << c << '\n';
    std::cout << "binary: " << to_binary(c) << '\n';

    return 0;
}
```

## Build and Run

```bash
g++ -std=c++20 -I/path/to/universal/include/sw hello_universal.cpp -o hello_universal
./hello_universal
```

Expected output:

```text
a = 1.5
b = 2.75
a + b = 4.25
binary = 0b0.10.00.010000000000000000000000000
```

## The Plug-in Pattern

The real power of Universal is writing algorithms that work with any number type:

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>

template<typename Real>
Real dot_product(const std::vector<Real>& a, const std::vector<Real>& b) {
    size_t n = std::min(a.size(), b.size());
    Real sum = 0.0;
    for (size_t i = 0; i < n; ++i)
        sum += a[i] * b[i];
    return sum;
}

int main() {
    using namespace sw::universal;

    std::vector<double> da = {1.0, 2.0, 3.0};
    std::vector<double> db = {4.0, 5.0, 6.0};
    const size_t N = da.size();

    // Same algorithm, different precisions
    std::vector<posit<32,2>> pa(N), pb(N);
    std::vector<half> ha(N), hb(N);

    for (size_t i = 0; i < N; ++i) {
        pa[i] = da[i]; pb[i] = db[i];
        ha[i] = da[i]; hb[i] = db[i];
    }

    std::cout << "posit<32,2>: " << dot_product(pa, pb) << '\n';
    std::cout << "half:        " << dot_product(ha, hb) << '\n';

    return 0;
}
```

## Next Steps

- Browse the [Number Systems Guide](/universal/number-systems/) to find the right type for your application
- Explore the [Command-Line Tools](/universal/getting-started/command-line-tools/) to inspect number representations
- Read the `api/api.cpp` test file in each number system's directory for comprehensive usage patterns
