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
#include <cassert>
#include <cmath>
#include <iostream>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/elreal_oracle.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// exact_real / exact_block / exact_value: the shared exact-dyadic oracle (#1035).
using namespace sw::universal::elreal_oracle;

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
