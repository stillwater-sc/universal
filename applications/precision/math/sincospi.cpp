// sincospi.cpp: sinpi/cospi experiment
//
// sinpi/cospi trigonometric functions
// inspired by: https://stackoverflow.com/questions/42792939/implementation-of-sinpi-and-cospi-using-standard-c-math-library
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal number project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <universal/number/areal/areal.hpp>
#include <universal/number/posit/posit.hpp>

/*
For simplicity, I will focus on sincospi(), which simultaneously provides both the sine and the cosine results. 
sinpi and cospi can then be constructed as wrapper functions that discard unneeded data. In many applications, 
the handling of floating-point flags (see fenv.h) is not required, nor do we need errno error reporting 
most of the time, so I will omit these.

The basic algorithmic structure is straightforward. As very large arguments are always even integers, 
and therefore thus multiples of 2π, their sine and cosine values are well-known. Other arguments are 
folded into range [-¼,+¼] while recording quadrant information. Polynomial minimax approximations 
are used to compute sine and cosine on the primary approximation interval. Finally, quadrant data 
is used to map the preliminary results to the final result by cyclical exchange of results and sign change.

The correct handling of special operands (in particular -0, infinities, and NaNs) requires the 
compiler to apply only optimizations that comply with IEEE-754 rules. It may not transform 
x*0.0 into 0.0 (this is not correct for -0, infinities, and NaNs) nor may it optimize 0.0-x 
into -x as negation is a bit-level operation according to section 5.5.1 of IEEE-754 
(yielding different results for zeros and NaNs). Most compilers will offer a flag that 
enforces the use of "safe" transformations, e.g. -fp-model=precise for the Intel C/C++ compiler.

One additional caveat applies to the use of the nearbyint function during argument reduction. 
Like rint, this function is specified to round according to the current rounding mode. 
When fenv.h isn't used, the rounding mode defaults to round "to-nearest-or-even". 
When it is used, there is a risk that a directed rounding mode is in effect. This could 
be worked around by the use of round, which always provides the rounding mode "round 
to nearest, ties away from zero" independent of current rounding mode. However, this 
function will tend to be slower since it is not supported by an equivalent machine 
instruction on most processor architectures.

A note on performance: The C99 code below relies heavily on the use of fma(), 
which implements a fused multiply-add operation. On most modern hardware architectures, 
this is directly supported by a corresponding hardware instruction. Where this is 
not the case, the code may experience significant slow-down due to generally slow FMA emulation.
*/

#include <math.h>
#include <stdint.h>

/* Writes result sine result sin(πa) to the location pointed to by sp
   Writes result cosine result cos(πa) to the location pointed to by cp

   In extensive testing, no errors > 0.97 ulp were found in either the sine
   or cosine results, suggesting the results returned are faithfully rounded.
*/
void my_sincospi (double a, double *sp, double *cp)
{
    double c, r, s, t, az;
    int64_t i;

    az = a * 0.0; // must be evaluated with IEEE-754 semantics
    /* for |a| >= 2**53, cospi(a) = 1.0, but cospi(Inf) = NaN */
    a = (fabs (a) < 9.0071992547409920e+15) ? a : az;  // 0x1.0p53
    /* reduce argument to primary approximation interval (-0.25, 0.25) */
    r = nearbyint (a + a); // must use IEEE-754 "to nearest" rounding
    i = (int64_t)r;
    t = fma (-0.5, r, a);
    /* compute core approximations */
    s = t * t;
    /* Approximate cos(pi*x) for x in [-0.25,0.25] */
    r =            -1.0369917389758117e-4;
    r = fma (r, s,  1.9294935641298806e-3);
    r = fma (r, s, -2.5806887942825395e-2);
    r = fma (r, s,  2.3533063028328211e-1);
    r = fma (r, s, -1.3352627688538006e+0);
    r = fma (r, s,  4.0587121264167623e+0);
    r = fma (r, s, -4.9348022005446790e+0);
    c = fma (r, s,  1.0000000000000000e+0);
    /* Approximate sin(pi*x) for x in [-0.25,0.25] */
    r =             4.6151442520157035e-4;
    r = fma (r, s, -7.3700183130883555e-3);
    r = fma (r, s,  8.2145868949323936e-2);
    r = fma (r, s, -5.9926452893214921e-1);
    r = fma (r, s,  2.5501640398732688e+0);
    r = fma (r, s, -5.1677127800499516e+0);
    s = s * t;
    r = r * s;
    s = fma (t, 3.1415926535897931e+0, r);
    /* map results according to quadrant */
    if (i & 2) {
        s = 0.0 - s; // must be evaluated with IEEE-754 semantics
        c = 0.0 - c; // must be evaluated with IEEE-754 semantics
    }
    if (i & 1) { 
        t = 0.0 - s; // must be evaluated with IEEE-754 semantics
        s = c;
        c = t;
    }
    /* IEEE-754: sinPi(+n) is +0 and sinPi(-n) is -0 for positive integers n */
    if (a == floor (a)) s = az;
    *sp = s;
    *cp = c;
}

/* Writes result sine result sin(πa) to the location pointed to by sp
   Writes result cosine result cos(πa) to the location pointed to by cp

   In exhaustive testing, the maximum error in sine results was 0.96677 ulp,
   the maximum error in cosine results was 0.96563 ulp, meaning results are
   faithfully rounded.
*/
void my_sincospif (float a, float *sp, float *cp)
{
    float az, t, c, r, s;
    int32_t i;

    az = a * 0.0f; // must be evaluated with IEEE-754 semantics
    /* for |a| > 2**24, cospi(a) = 1.0f, but cospi(Inf) = NaN */
    a = (fabsf (a) < 0x1.0p24f) ? a : az;
    r = nearbyintf (a + a); // must use IEEE-754 "to nearest" rounding
    i = (int32_t)r;
    t = fmaf (-0.5f, r, a);
    /* compute core approximations */
    s = t * t;
    /* Approximate cos(pi*x) for x in [-0.25,0.25] */
    r =              0x1.d9e000p-3f;
    r = fmaf (r, s, -0x1.55c400p+0f);
    r = fmaf (r, s,  0x1.03c1cep+2f);
    r = fmaf (r, s, -0x1.3bd3ccp+2f);
    c = fmaf (r, s,  0x1.000000p+0f);
    /* Approximate sin(pi*x) for x in [-0.25,0.25] */
    r =             -0x1.310000p-1f;
    r = fmaf (r, s,  0x1.46737ep+1f);
    r = fmaf (r, s, -0x1.4abbfep+2f);
    r = (t * s) * r;
    s = fmaf (t, 0x1.921fb6p+1f, r);
    if (i & 2) {
        s = 0.0f - s; // must be evaluated with IEEE-754 semantics
        c = 0.0f - c; // must be evaluated with IEEE-754 semantics
    }
    if (i & 1) {
        t = 0.0f - s; // must be evaluated with IEEE-754 semantics
        s = c;
        c = t;
    }
    /* IEEE-754: sinPi(+n) is +0 and sinPi(-n) is -0 for positive integers n */
    if (a == floorf (a)) s = az;
    *sp = s;
    *cp = c;
}

/*
To the extent that you expressly depend on IEEE 754 semantics, 
how do you get around the fact that the C standard does not require 
implementations' floating-point representations or arithmetic to 
comply with IEEE 754 (at all)? – John Bollinger Mar 14 '17 at 18:42 

@JohnBollinger I don't. If a tool chain offers sufficient control 
of floating-point formats and transformations in accordance with 
IEEE-754 rules, then this code works correctly with respect to 
IEEE-754 (best I could test it). Conversely, if a tool chain 
generally does not conform to IEEE-754, there should be no 
expectation (nor do I see a necessity) for this code to comply 
with all requirements of IEEE-754 either. – njuffa Mar 14 '17 at 18:55

Out of curiosity, why do you use hex floats and decimal doubles? – rici Mar 14 '17 at 19:39

In the last step of the calculation of the sine, instead of computing 
s = s * t; r = r * s; s = fma (t, π, r); which amounts to computing s = π*t + t^3, 
a multiplication by t can be factored out so that a fma and a 
further multiplication suffice: s = fma (r, s,  3.1415926535897931e+0); s = s * t. – Matías Giovannini May 25 '18 at 15:19

@MatíasGiovannini This re-ordering causes maximum ulp error to increase 
(anecdotally to ~ 1.5 ulp), so the implementation is no longer faithfully 
rounded (which was a design goal of mine). This may be acceptable in 
some contexts. – njuffa May 25 '18 at 15:57
*/

// conditional compilation
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main() 
try {
	using namespace sw::universal;

#if MANUAL_TESTING


#else // MANUAL_TESTING


#if STRESS_TESTING

#endif // STRESS_TESTING
#endif // MANUAL_TESTING


	return EXIT_SUCCESS;
}
catch (char const* msg) {
    std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
    return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
    std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
    std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
    std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
    return EXIT_FAILURE;
}
