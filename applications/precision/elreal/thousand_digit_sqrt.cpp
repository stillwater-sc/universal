// thousand_digit_sqrt.cpp: one kernel, two host types, a thousand digits.
//
// The claim this demonstrates: for elreal, the host floating-point type is the
// SIZE OF A LIMB, not a ceiling on precision. A block is (v: FpType, exp) with the
// exponent held in a wide integer, so an expansion's reach is a budget you choose
// rather than a property of FpType. Run the same Newton iteration over
// elreal<float> and elreal<double> and both converge to the same 1000+ decimal
// digits of sqrt(2) -- float just needs more limbs to hold them.
//
// Contrast that with the native double the kernel is also instantiated on, which
// stalls near 16 digits however many iterations it is given.
//
// The kernel itself is ordinary templated numeric code -- no elreal in it:
//
//     template <typename Real>
//     Real newton_sqrt(const Real& a, int iterations) {
//         Real x = a / Real(2.0);
//         for (int i = 0; i < iterations; ++i) x = (x + a / x) / Real(2.0);
//         return x;
//     }
//
// This is a DEMONSTRATION, not a regression test: it prints and explains rather
// than asserting, and it is deliberately not registered with ctest (its CMakeLists
// passes "false"), because converging a float host to 1000 digits takes ~25s.
//
// Historical note on why this became possible: division by a multi-block value used
// to cap at ~271 decimal digits on a double host and ~22 on float, so the Newton
// step here could not have worked at all. Three defects had to go -- a host-derived
// depth constant in div_online (#1371), and a streaming-summation branch that
// dropped a term whenever the accumulator cancelled to zero (#1373), on top of the
// operand-normalisation work in v4.9.0 (#1362).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/elreal_reference_digits.hpp>   // zbcl_to_dyadic

namespace {

using namespace sw::universal;
using clk = std::chrono::steady_clock;

// sqrt(2) to 1200 decimal digits, computed in exact integer arithmetic outside this
// library (isqrt(2 * 10^2400)), so it is an independent yardstick.
const char* s_sqrt2 =
    "1.4142135623730950488016887242096980785696718753769480731766797379907324784621070388503875343276"
    "415727350138462309122970249248360558507372126441214970999358314132226659275055927557999505011527"
    "820605714701095599716059702745345968620147285174186408891986095523292304843087143214508397626036"
    "279952514079896872533965463318088296406206152583523950547457502877599617298355752203375318570113"
    "543746034084988471603868999706990048150305440277903164542478230684929369186215805784631115966687"
    "130130156185689872372352885092648612494977154218334204285686060146824720771435854874155657069677"
    "653720226485447015858801620758474922657226002085584466521458398893944370926591800311388246468157"
    "082630100594858704003186480342194897278290641045072636881313739855256117322040245091227700226941"
    "127573627280495738108967504018369868368450725799364729060762996941380475654823728997180326802474"
    "420629269124859052181004459842150591120249441341728531478105803603371077309182869314710171111683"
    "916581726889419758716582152128229518488472089694633862891562882765952635140542267653239694617511"
    "291602408715510135150455381287560052631468017127402653969470240300517495318862925631385188163478"
    "00156936917688185237868405228783762938921430065586";

// ---------------------------------------------------------------- the kernel
// Ordinary templated numeric code. It knows nothing about elreal, and is
// instantiated below on double, elreal<float>, and elreal<double> alike.
template <typename Real>
Real newton_sqrt(const Real& a, int iterations) {
    Real x = a / Real(2.0);
    for (int i = 0; i < iterations; ++i) x = (x + a / x) / Real(2.0);
    return x;
}

// ---------------------------------------------------------------- utilities
template <typename BI> std::string bigint_to_string(const BI& v) {
    std::ostringstream o; o << v; return o.str();
}

// Render a positive ZBCL as `digits` significant decimal digits (truncated).
// A ZBCL is a sum of blocks, hence an exact dyadic rational num/2^s; one big
// division by 10^digits' worth of scaling produces the decimal expansion.
template <typename FpType>
std::string decimal_digits(const ZBCL<FpType>& z, int digits) {
    using bigint = dyadic::bigint;
    dyadic v = zbcl_to_dyadic(z);
    bigint num = v.numerator, den(1);
    if (v.scale >= 0) num <<= v.scale; else den <<= (-v.scale);
    bigint p10; p10.assign("1" + std::string(static_cast<std::size_t>(digits), '0'));
    const std::string all = bigint_to_string((num * p10) / den);
    const std::size_t ilen = (all.size() > static_cast<std::size_t>(digits))
                           ? all.size() - static_cast<std::size_t>(digits) : 1;
    return all.substr(0, ilen) + "." + all.substr(ilen);
}

// How many leading characters of a rendered value match the reference.
// Counts characters, so the '.' is included -- close enough for a display figure,
// and it never overstates by more than one.
int matching_digits(const std::string& got) {
    const std::string ref(s_sqrt2);
    std::size_t n = 0;
    while (n < got.size() && n < ref.size() && got[n] == ref[n]) ++n;
    return static_cast<int>(n);
}

template <typename FpType>
int digits_of(const ZBCL<FpType>& z, int render) {
    return matching_digits(decimal_digits<FpType>(z, render));
}

// ------------------------------------------------- part 1: the native baseline
void native_double_baseline() {
    std::cout << "\n1. The same kernel on a native double\n"
              << "   -----------------------------------\n";
    for (int iters : { 4, 8, 16, 64 }) {
        const double r = newton_sqrt<double>(2.0, iters);
        std::ostringstream o; o << std::setprecision(17) << std::fixed << r;
        std::cout << "   " << std::setw(3) << iters << " iterations: " << o.str()
                  << "   matching digits: " << matching_digits(o.str()) << "\n";
    }
    std::cout << "   More iterations buy nothing: the type ran out of room, not the algorithm.\n";
}

// ------------------------------- part 2: the same kernel, elreal host types
template <typename FpType>
void elreal_fixed_precision(const char* host, std::size_t precision, int iterations) {
    using Real = elreal<FpType>;
    elreal_precision_guard guard(precision);
    const auto t0 = clk::now();
    Real r = newton_sqrt<Real>(Real(2.0), iterations);
    r.precision(precision);
    const std::size_t blocks = r.stream().take(4 * precision).size();
    const auto t1 = clk::now();
    std::cout << "   elreal<" << std::left << std::setw(7) << host << "> precision "
              << std::setw(4) << precision << " blocks " << std::setw(4) << blocks
              << "  digits " << std::setw(5) << digits_of<FpType>(r.stream(), 1190)
              << "  " << std::fixed << std::setprecision(1)
              << std::chrono::duration<double>(t1 - t0).count() << "s\n";
}

// ------------------- part 3: spending precision only as the iterate earns it
// Newton doubles the number of correct BITS per step, so the early steps do not
// need -- and should not pay for -- the final precision. This is the one place the
// demo uses an elreal-specific API: precision() per object, and collapsing the lazy
// expansion so the unforced graph does not compound across iterations.
template <typename FpType>
elreal<FpType> newton_sqrt_progressive(double a_, std::size_t target, const char* host) {
    using Real = elreal<FpType>;
    Real a(a_);
    Real x(std::sqrt(a_));                       // host-double seed: ~53 bits correct
    std::cout << "   elreal<" << host << ">:";
    for (std::size_t p = 2; ; p = (2 * p < target) ? 2 * p : target) {
        a.precision(p);
        x.precision(p);
        x = (x + a / x) / Real(2.0);
        x = Real(zbcl_from_blocks<FpType>(x.stream().take(p)), p);   // collapse
        std::cout << " " << digits_of<FpType>(x.stream(), 1190);
        if (p == target) break;
    }
    std::cout << " digits\n";
    return x;
}

} // anonymous

int main()
try {
    using namespace sw::universal;

    std::cout << "elreal: one kernel, two host types, a thousand digits\n"
              << "=====================================================\n"
              << "\nComputing sqrt(2) by Newton's method:  x <- (x + a/x) / 2\n";

    native_double_baseline();

    std::cout << "\n2. The same kernel over elreal, unmodified\n"
              << "   --------------------------------------\n"
              << "   A modest target here so the demo stays quick; part 3 goes to a thousand.\n";
    // block counts chosen so both hosts land on the SAME digit count: a double limb
    // carries 53 bits and a float limb 24, so float needs ~2.2x the blocks for it.
    elreal_fixed_precision<double>("double", 16, 11);
    elreal_fixed_precision<float>("float", 35, 11);
    std::cout << "   Same kernel, same answer, both near 300 digits -- float simply needed\n"
              << "   ~2.2x the blocks to hold them, its limb carrying 24 bits to double's 53.\n";

    std::cout << "\n3. Spending precision only as the iterate earns it\n"
              << "   ----------------------------------------------\n"
              << "   Newton doubles the correct bits per step, so the digit count per\n"
              << "   iteration should double too:\n";
    const auto t0 = clk::now();
    elreal<double> rd = newton_sqrt_progressive<double>(2.0, 70, "double");
    const auto t1 = clk::now();
    elreal<float>  rf = newton_sqrt_progressive<float>(2.0, 150, "float ");
    const auto t2 = clk::now();
    std::cout << "   double " << std::fixed << std::setprecision(1)
              << std::chrono::duration<double>(t1 - t0).count() << "s, float "
              << std::chrono::duration<double>(t2 - t1).count()
              << "s.  Driving the SAME targets at fixed full precision instead of\n"
              << "   ramping costs 25s and 206s respectively -- measured, not estimated.\n";

    std::cout << "\n4. The digits, computed on a FLOAT host\n"
              << "   ------------------------------------\n";
    const std::string got = decimal_digits<float>(rf.stream(), 1190);
    const int agree = matching_digits(got);
    for (int line = 0; line < 6; ++line) {
        const std::size_t off = static_cast<std::size_t>(line) * 80;
        if (off >= got.size()) break;
        std::cout << "   " << got.substr(off, 80) << "\n";
    }
    std::cout << "   ... " << agree << " leading characters match a reference computed in\n"
              << "   exact integer arithmetic outside this library.\n";

    std::cout << "\n   From part 3:\n"
              << "     elreal<float>  reached " << digits_of<float>(rf.stream(), 1190) << " digits\n"
              << "     elreal<double> reached " << digits_of<double>(rd.stream(), 1190) << " digits\n"
              << "     native double  reached 16\n\n"
              << "The host type set the limb size and the cost. It did not set the precision.\n\n";
    return EXIT_SUCCESS;
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
