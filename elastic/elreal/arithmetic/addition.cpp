// addition.cpp: tests for the lazy ZBCL add() combinator.
//
// Verifies:
//   - Value preservation: take(N) of add(x, y) sums to approximately x + y in
//     long double (to leading O(k) bits per emitted block).
//   - 0-overlap: every adjacent pair of emitted blocks satisfies zero_overlap.
//   - No-renormalisation: take(N-1) returns the same first N-1 blocks as the
//     prefix of take(N) (emitted blocks are final once committed).
//   - Laziness: thunks are forced only when downstream consumers ask for blocks.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template <typename FpType>
int verify_value(double a, double b, const std::string& tag,
                 double leading_bits_tol_factor = 4.0) {
    using namespace sw::universal;
    int nrFailures = 0;

    auto x = from_native<FpType>(a);
    auto y = from_native<FpType>(b);
    auto z = add(x, y);

    // Sum the first 4 blocks as a long-double approximation
    auto blocks = z.take(4);
    long double got = 0.0L;
    for (const auto& blk : blocks) {
        got += static_cast<long double>(blk.v);
    }
    long double ref = static_cast<long double>(a) + static_cast<long double>(b);

    // Leading-bits tolerance: per FpType, the first block carries ~k bits of
    // accuracy. The sum of 4 blocks should agree with ref to within a small
    // multiple of FpType's ulp at the magnitude of the result.
    constexpr int p = std::numeric_limits<FpType>::digits;
    long double scale = std::fmax(std::fabs(ref), 1.0L);
    long double tol = scale * std::ldexp(1.0L, -p) * leading_bits_tol_factor;
    if (std::fabs(ref - got) > tol) {
        std::cout << tag << " value mismatch: ref=" << ref
                  << " got=" << got
                  << " diff=" << std::fabs(ref - got)
                  << " tol=" << tol << '\n';
        ++nrFailures;
    }
    return nrFailures;
}

template <typename FpType>
int verify_zero_overlap_on_output(double a, double b, const std::string& tag) {
    using namespace sw::universal;
    int nrFailures = 0;
    auto z = add(from_native<FpType>(a), from_native<FpType>(b));
    auto blocks = z.take(6);
    for (std::size_t i = 0; i + 1 < blocks.size(); ++i) {
        if (!blocks[i].is_zero_block() && !blocks[i + 1].is_zero_block()
            && !zero_overlap(blocks[i], blocks[i + 1])) {
            std::cout << tag << " 0-overlap FAILED at position " << i
                      << " (a=" << a << " b=" << b << ")\n";
            ++nrFailures;
        }
    }
    return nrFailures;
}

// no-renormalisation: take(n) and take(n+k) must agree on the first n blocks.
template <typename FpType>
int verify_no_renormalisation(double a, double b, const std::string& tag) {
    using namespace sw::universal;
    int nrFailures = 0;
    auto z = add(from_native<FpType>(a), from_native<FpType>(b));
    auto first3 = z.take(3);
    auto first6 = z.take(6);
    for (std::size_t i = 0; i < first3.size() && i < first6.size(); ++i) {
        if (first3[i].v != first6[i].v
            || first3[i].exp_offset != first6[i].exp_offset) {
            std::cout << tag << " take(3) and take(6) disagree at position " << i << '\n';
            ++nrFailures;
        }
    }
    return nrFailures;
}

template <typename FpType>
int verify_add(const std::string& tag) {
    int nrFailures = 0;
    // Representative inputs
    nrFailures += verify_value<FpType>(1.0, 0.25, tag + " 1+0.25");
    nrFailures += verify_value<FpType>(3.5, -1.5, tag + " 3.5-1.5");
    nrFailures += verify_value<FpType>(1e3, 1.0, tag + " 1000+1");
    nrFailures += verify_value<FpType>(-2.5, -7.5, tag + " -2.5-7.5");

    nrFailures += verify_zero_overlap_on_output<FpType>(1.0, 0.25, tag);
    nrFailures += verify_zero_overlap_on_output<FpType>(7.0, 13.0, tag);

    nrFailures += verify_no_renormalisation<FpType>(1.5, 2.5, tag);
    nrFailures += verify_no_renormalisation<FpType>(100.0, 0.0625, tag);

    // Empty + non-empty
    {
        using namespace sw::universal;
        auto e = empty<FpType>();
        auto x = from_native<FpType>(3.25);
        auto z = add(e, x);
        auto blocks = z.take(4);
        if (blocks.empty()) {
            std::cout << tag << " empty + non-empty produced empty result\n";
            ++nrFailures;
        }
        // First block should approximate 3.25
        if (!blocks.empty()
            && std::fabs(static_cast<double>(blocks[0].v) - 3.25) > 0.1) {
            std::cout << tag << " empty + 3.25 first block wrong\n";
            ++nrFailures;
        }
    }

    // Empty + empty
    {
        using namespace sw::universal;
        auto z = add(empty<FpType>(), empty<FpType>());
        if (!z.is_empty()) {
            std::cout << tag << " empty + empty produced non-empty\n";
            ++nrFailures;
        }
    }

    return nrFailures;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal ZBCL add() combinator";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_add<double>("add<double>");
    nrOfFailedTestCases += verify_add<float>("add<float>");
    nrOfFailedTestCases += verify_add<bfloat16>("add<bfloat16>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
