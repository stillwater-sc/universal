// negation.cpp: Phase 6 (#930) tests for negate() (dissertation 4.2.4).
//
// Verifies exact_value(negate(x)) == -exact_value(x), the involution
// -(-x) == x (bit-identical stream), 0-overlap preservation, and the empty
// (zero) co-list special case, parameterised over {double, float, bfloat16}.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <vector>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

#include "arithmetic_oracle.hpp"

namespace {

namespace est = sw::universal::elreal_arith_test;

template <typename FpType>
int verify_one(double v, const std::string& tag) {
    using namespace sw::universal;
    int n = 0;
    ZBCL<FpType> x = from_native<FpType>(v);
    ZBCL<FpType> nx = negate(x);

    // exact_value(negate(x)) == -exact_value(x)
    if (!(est::exact_value(nx) == -est::exact_value(x))) {
        std::cout << tag << " value mismatch\n"; ++n;
    }
    // involution: -(-x) == x, block-for-block
    auto xb = x.take(16);
    auto nnb = negate(nx).take(16);
    if (xb.size() != nnb.size()) { std::cout << tag << " -(-x) size mismatch\n"; ++n; }
    else for (std::size_t i = 0; i < xb.size(); ++i) {
        if (xb[i].v != nnb[i].v || xb[i].exp != nnb[i].exp) {
            std::cout << tag << " -(-x) block " << i << " mismatch\n"; ++n; break;
        }
    }
    // 0-overlap preserved
    n += est::check_zero_overlap<FpType>(nx, 16, tag);
    return n;
}

template <typename FpType>
int verify_all(const std::string& host) {
    using namespace sw::universal;
    int n = 0;
    for (double v : { 1.0, -1.0, 6.0, 0.25, -123.5, 1e6, 7.0 })
        n += verify_one<FpType>(v, host + " negate(" + std::to_string(v) + ")");
    // empty (zero) co-list negates to empty
    if (!negate(ZBCL<FpType>{}).is_empty()) { std::cout << host << " negate(0) not empty\n"; ++n; }
    return n;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal Phase 6 (#930) negate()";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_all<double>("negate<double>");
    nrOfFailedTestCases += verify_all<float>("negate<float>");
    nrOfFailedTestCases += verify_all<bfloat16>("negate<bfloat16>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
