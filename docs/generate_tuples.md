\# Generate floatcascade tuples for constexpr acceleration



\## Basic API



```cpp



int main() {

&nbsp;   generate\_double\_double("π/3", \[](mpfr\_t x, mpfr\_rnd\_t r) { mpfr\_const\_pi(x, r); mpfr\_div\_ui(x, x, 3, r); });

&nbsp;   generate\_double\_double("ln(2)", \[](mpfr\_t x, mpfr\_rnd\_t r) { mpfr\_log\_ui(x, 2, r); });

&nbsp;   generate\_double\_double("ζ(3)", \[](mpfr\_t x, mpfr\_rnd\_t r) { mpfr\_zeta\_ui(x, 3, r); });

&nbsp;   return 0;

}



```



\## Basic Approach



```cpp



\#include <iostream>

\#include <iomanip>

\#include <mpfr.h>



// Split a double into hi and lo such that hi + lo ≈ x

void split\_double\_double(double x, double\& hi, double\& lo) {

&nbsp;   constexpr double splitter = 134217729.0; // 2^27 + 1

&nbsp;   double temp = splitter \* x;

&nbsp;   double x\_hi = temp - (temp - x);

&nbsp;   double x\_lo = x - x\_hi;

&nbsp;   hi = x\_hi;

&nbsp;   lo = x\_lo;

}



// Generate double-double pair for a given MPFR constant

void generate\_double\_double(const char\* label, void (\*mpfr\_func)(mpfr\_t, mpfr\_rnd\_t)) {

&nbsp;   mpfr\_t val;

&nbsp;   mpfr\_init2(val, 106); // ~53 bits × 2

&nbsp;   mpfr\_func(val, MPFR\_RNDN);



&nbsp;   double approx = mpfr\_get\_d(val, MPFR\_RNDN);

&nbsp;   double hi, lo;

&nbsp;   split\_double\_double(approx, hi, lo);



&nbsp;   // Compute residual correction

&nbsp;   mpfr\_t hi\_mpfr, lo\_mpfr, sum\_mpfr;

&nbsp;   mpfr\_inits2(106, hi\_mpfr, lo\_mpfr, sum\_mpfr, nullptr);

&nbsp;   mpfr\_set\_d(hi\_mpfr, hi, MPFR\_RNDN);

&nbsp;   mpfr\_set\_d(lo\_mpfr, lo, MPFR\_RNDN);

&nbsp;   mpfr\_add(sum\_mpfr, hi\_mpfr, lo\_mpfr, MPFR\_RNDN);

&nbsp;   mpfr\_sub(val, val, sum\_mpfr, MPFR\_RNDN); // residual error



&nbsp;   double correction = mpfr\_get\_d(val, MPFR\_RNDN);

&nbsp;   lo += correction;



&nbsp;   std::cout << std::setprecision(17);

&nbsp;   std::cout << label << " double-double:\\n";

&nbsp;   std::cout << "  hi = " << hi << "\\n";

&nbsp;   std::cout << "  lo = " << lo << "\\n\\n";



&nbsp;   mpfr\_clears(val, hi\_mpfr, lo\_mpfr, sum\_mpfr, nullptr);

}



```



