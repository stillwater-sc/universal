// geometric.cpp: Phase 5 (#929) summation tests on geometric series.
//
// Verifies sum() against an INDEPENDENT exact dyadic oracle (not long double):
// for a finite list of terms, sum() must reproduce the exact sum of the
// REPRESENTED terms bit-for-bit:
//     exact_value(sum(series)) == sum_i exact_value(term_i).
// Also checks the 0-overlap invariant on the result and a closed-form sanity
// approximation (sum_{i<N} r^i = (1 - r^N)/(1 - r)).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

#include "summation_oracle.hpp"

namespace {

namespace est = sw::universal::elreal_sum_test;

// Build the finite geometric series [r^0, r^1, ..., r^{N-1}] as singleton ZBCLs.
template <typename FpType>
std::vector<sw::universal::ZBCL<FpType>> geometric_terms(double ratio, std::size_t N) {
    using namespace sw::universal;
    std::vector<ZBCL<FpType>> terms;
    terms.reserve(N);
    double term = 1.0;                       // r^0
    for (std::size_t i = 0; i < N; ++i) {
        terms.push_back(from_native<FpType>(term));
        term *= ratio;
    }
    return terms;
}

template <typename FpType>
int verify_geometric(double ratio, std::size_t N, double closed_form, double tol,
                     const std::string& tag) {
    using namespace sw::universal;
    int nrFailures = 0;

    auto terms  = geometric_terms<FpType>(ratio, N);
    auto result = sum<FpType>(series_from_vector<FpType>(terms));   // finite -> exact

    // (1) Exact-oracle bit-for-bit contract on the represented terms.
    dyadic got = est::exact_value(result);
    dyadic ref = est::exact_series_sum<FpType>(terms);
    if (!(got == ref)) {
        std::cout << tag << " exact mismatch: sum(series) != sum of terms\n";
        ++nrFailures;
    }

    // (2) 0-overlap invariant on the result prefix.
    nrFailures += est::check_zero_overlap<FpType>(result, 16, tag);

    // (3) Closed-form sanity. The result is the exact sum of the REPRESENTED
    // terms, so for a ratio that is not exactly representable (e.g. 1/3) it
    // deviates from the ideal closed form by the host's rounding of the terms
    // (~N * eps). Scale the tolerance by host precision so the check is
    // meaningful on float / cfloat<32,8> too; the exact-oracle check above is
    // the rigorous one.
    double approx   = to_double_approx(result, 64);
    double host_eps = static_cast<double>(std::numeric_limits<FpType>::epsilon());
    double eff_tol  = tol + 8.0 * static_cast<double>(N) * host_eps * std::abs(closed_form);
    if (std::abs(approx - closed_form) > eff_tol) {
        std::cout << tag << " closed-form sanity FAILED: got " << approx
                  << " expected ~" << closed_form << " (tol " << eff_tol << ")\n";
        ++nrFailures;
    }
    return nrFailures;
}

template <typename FpType>
int verify_all(const std::string& host) {
    int n = 0;
    // sum (1/2)^n -> 2 ; exactly representable terms
    n += verify_geometric<FpType>(0.5,      32, 2.0,        1e-9, host + " (1/2)^n");
    // sum (1/4)^n -> 4/3
    n += verify_geometric<FpType>(0.25,     16, 4.0 / 3.0,  1e-9, host + " (1/4)^n");
    // sum (-1/2)^n -> 2/3 ; alternating geometric
    n += verify_geometric<FpType>(-0.5,     32, 2.0 / 3.0,  1e-9, host + " (-1/2)^n");
    // sum (1/3)^n -> 3/2 ; 1/3 is NOT exactly representable, so the oracle
    // compares the represented terms (the exact contract still holds).
    n += verify_geometric<FpType>(1.0/3.0,  24, 1.5,        1e-9, host + " (1/3)^n");
    return n;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal Phase 5 (#929) summation: geometric series";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_all<double>("sum<double>");
    nrOfFailedTestCases += verify_all<float>("sum<float>");
    nrOfFailedTestCases += verify_all<single>("sum<cfloat<32,8>>");   // single = cfloat<32,8>

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
