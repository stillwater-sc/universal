// addition.cpp: tests for the lazy ZBCL add() combinator (dissertation 4.2.2).
//
// Verifies:
//   - 0-overlap: every adjacent pair in the output stream satisfies the
//     ZBCL_k 0-overlap property.
//   - Value approximation: the sum of the first N emitted blocks
//     approximates a + b in long double.
//   - No-renormalisation: take(n) is a prefix of take(n+k) for all k.
//   - Empty stream handling.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// Exact value of a binary float v as a dyadic, with no precision loss at any
// significand width (shared with the #1022 oracle; see its exact_real for the
// full rationale). p <= 53: double is exact. p > 53 (quad and up): extract the
// full integer significand directly, 24 bits at a time, using only exact
// power-of-two shifts / bit-exact floor / sub-2^24 chunks -- never through double.
template <typename T>
sw::universal::dyadic exact_real(T v) {
    using namespace sw::universal;
    if (v == T(0)) return dyadic();
    if constexpr (std::numeric_limits<T>::digits <= 53) {
        return dyadic::from_double(static_cast<double>(v));
    } else {
        static_assert(std::numeric_limits<T>::max_exponent
                          > std::numeric_limits<T>::digits,
            "exact_real wide path needs exponent range > significand width.");
        using std::frexp; using std::ldexp; using std::floor;
        int e = 0;
        T m = frexp(v, &e);
        constexpr int p = std::numeric_limits<T>::digits;
        T scaled = ldexp(m, p);
        bool neg = scaled < T(0);
        if (neg) scaled = -scaled;
        dyadic::bigint M(0);
        const T CHUNK = ldexp(T(1), 24);
        int shift = 0;
        while (scaled > T(0)) {
            T hi = floor(scaled / CHUNK);
            T lo = scaled - hi * CHUNK;
            dyadic::bigint chunk(static_cast<long long>(static_cast<double>(lo)));
            chunk <<= shift;
            M = M + chunk;
            scaled = hi;
            shift += 24;
        }
        if (neg) M = -M;
        return dyadic(M, e - p);
    }
}

// Exact value of a block as a dyadic rational (value(b) = v * 2^exp), shared
// with the #1022 oracle.
template <typename FpType>
sw::universal::dyadic exact_block(const sw::universal::block<FpType>& b) {
    using namespace sw::universal;
    if (b.is_zero_block()) return dyadic();
    dyadic d = exact_real(b.v);
    d.scale += b.exp;
    return d;
}

template <typename FpType>
sw::universal::dyadic exact_value(const sw::universal::ZBCL<FpType>& z) {
    using namespace sw::universal;
    dyadic acc;
    // 32-block window, matching the #1022 oracle's ZBCL_EXACT_WINDOW so both
    // files agree on the exact value of the same stream; finite test sums settle
    // well within it.
    for (const auto& blk : z.take(32)) acc = acc + exact_block(blk);
    return acc;
}

// Value preservation, validated against an INDEPENDENT exact dyadic oracle
// (not long double -- see #1022). double and float are round-to-nearest, so
// add() must reproduce the exact sum of the REPRESENTED operands bit-for-bit:
//   exact(add(za, zb)) == exact(za) + exact(zb).
// The reference uses the represented values exact(za)/exact(zb), not the
// original double literals, so it is correct for the float host too (the old
// 4-ulp band masked that distinction).
template <typename FpType>
int verify_value(double a, double b, const std::string& tag) {
    using namespace sw::universal;
    int nrFailures = 0;
    auto za = from_native<FpType>(a);
    auto zb = from_native<FpType>(b);
    dyadic ref = exact_value(za) + exact_value(zb);
    dyadic got = exact_value(add(za, zb));
    if (ref != got) {
        std::cout << tag << " value mismatch (exact dyadic): a=" << a
                  << " b=" << b << '\n';
        ++nrFailures;
    }
    return nrFailures;
}

template <typename FpType>
int verify_zero_overlap(double a, double b, const std::string& tag) {
    using namespace sw::universal;
    int nrFailures = 0;
    auto z = add(from_native<FpType>(a), from_native<FpType>(b));
    auto blocks = z.take(8);
    for (std::size_t i = 0; i + 1 < blocks.size(); ++i) {
        if (!blocks[i].is_zero_block() && !blocks[i + 1].is_zero_block()
            && !zero_overlap(blocks[i], blocks[i + 1])) {
            std::cout << tag << " 0-overlap FAILED at position " << i << '\n';
            ++nrFailures;
        }
    }
    return nrFailures;
}

template <typename FpType>
int verify_no_renorm(double a, double b, const std::string& tag) {
    using namespace sw::universal;
    int nrFailures = 0;
    auto z = add(from_native<FpType>(a), from_native<FpType>(b));
    auto first3 = z.take(3);
    auto first6 = z.take(6);
    for (std::size_t i = 0; i < first3.size() && i < first6.size(); ++i) {
        if (first3[i].v != first6[i].v || first3[i].exp != first6[i].exp) {
            std::cout << tag << " take(3) and take(6) disagree at " << i << '\n';
            ++nrFailures;
        }
    }
    return nrFailures;
}

template <typename FpType>
int verify_add(const std::string& tag) {
    int nrFailures = 0;
    // Representative cases
    nrFailures += verify_value<FpType>(1.0, 0.25, tag + " 1+0.25");
    nrFailures += verify_value<FpType>(3.5, -1.5, tag + " 3.5-1.5");
    nrFailures += verify_value<FpType>(1e3, 1.0, tag + " 1000+1");
    nrFailures += verify_value<FpType>(-2.5, -7.5, tag + " -2.5-7.5");

    nrFailures += verify_zero_overlap<FpType>(1.0, 0.25, tag + " 1+0.25");
    nrFailures += verify_zero_overlap<FpType>(7.0, 13.0, tag + " 7+13");
    nrFailures += verify_zero_overlap<FpType>(1e3, 1.0, tag + " 1e3+1");

    nrFailures += verify_no_renorm<FpType>(1.5, 2.5, tag + " 1.5+2.5");
    nrFailures += verify_no_renorm<FpType>(100.0, 0.0625, tag + " 100+0.0625");

    // Empty + non-empty
    {
        using namespace sw::universal;
        auto z = add(empty<FpType>(), from_native<FpType>(3.25));
        auto blocks = z.take(4);
        if (blocks.empty()) {
            std::cout << tag << " empty+nonempty produced empty\n"; ++nrFailures;
        }
    }
    // Empty + empty
    {
        using namespace sw::universal;
        auto z = add(empty<FpType>(), empty<FpType>());
        if (!z.is_empty()) {
            std::cout << tag << " empty+empty not empty\n"; ++nrFailures;
        }
    }
    return nrFailures;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal add() lazy ZBCL combinator (dissertation 4.2.2)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_add<double>("add<double>");
    nrOfFailedTestCases += verify_add<float>("add<float>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
