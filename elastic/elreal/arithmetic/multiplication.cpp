// multiplication.cpp: Phase 6 (#930) tests for mul() (dissertation 4.2.5).
//
// Verifies the exact-product contract exact_value(mul(x,y)) == exact(x)*exact(y)
// (finite operands), the identity x*1 = x, commutativity x*y = y*x,
// distributivity x*(y+z) = x*y + x*z, zero and signed operands, and 0-overlap,
// parameterised over {double, float, bfloat16}.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

#include "arithmetic_oracle.hpp"

namespace {

namespace est = sw::universal::elreal_arith_test;

// exact_value(mul(x,y)) == exact(x) * exact(y), plus 0-overlap.
template <typename FpType>
int verify_exact(double a, double b, const std::string& tag) {
    using namespace sw::universal;
    int n = 0;
    ZBCL<FpType> x = from_native<FpType>(a), y = from_native<FpType>(b);
    ZBCL<FpType> p = mul(x, y);
    if (!(est::exact_value(p) == est::exact_value(x) * est::exact_value(y))) {
        std::cout << tag << " exact product mismatch\n"; ++n;
    }
    n += est::check_zero_overlap<FpType>(p, 16, tag);
    return n;
}

template <typename FpType>
int verify_all(const std::string& host) {
    using namespace sw::universal;
    int n = 0;

    // exact product contract over a spread of operands (representable in all hosts)
    const double vals[] = { 1.0, 2.0, 3.0, 0.5, -4.0, 7.0, -0.25 };
    for (double a : vals)
        for (double b : vals)
            n += verify_exact<FpType>(a, b, host + " mul");

    // identity x * 1 = x (exact)
    {
        ZBCL<FpType> x = from_native<FpType>(6.0), one = from_native<FpType>(1.0);
        if (!(est::exact_value(mul(x, one)) == est::exact_value(x))) {
            std::cout << host << " x*1 != x\n"; ++n;
        }
    }
    // commutativity x*y = y*x
    {
        ZBCL<FpType> x = from_native<FpType>(3.0), y = from_native<FpType>(-5.0);
        if (!(est::exact_value(mul(x, y)) == est::exact_value(mul(y, x)))) {
            std::cout << host << " x*y != y*x\n"; ++n;
        }
    }
    // distributivity x*(y+z) = x*y + x*z
    {
        ZBCL<FpType> x = from_native<FpType>(2.0), y = from_native<FpType>(3.0), z = from_native<FpType>(4.0);
        dyadic lhs = est::exact_value(mul(x, add(y, z)));
        dyadic rhs = est::exact_value(add(mul(x, y), mul(x, z)));
        if (!(lhs == rhs)) { std::cout << host << " distributivity failed\n"; ++n; }
    }
    // zero operand: x * 0 = 0 (empty)
    {
        ZBCL<FpType> x = from_native<FpType>(9.0);
        if (!mul(x, ZBCL<FpType>{}).is_empty()) { std::cout << host << " x*0 not empty\n"; ++n; }
    }
    // mul_scalar(s, y): single block s times ZBCL y == exact(s) * exact(y)
    {
        block<FpType> s{ static_cast<FpType>(3.0), 0 };
        ZBCL<FpType> y = from_native<FpType>(5.0);
        dyadic got = est::exact_value(mul_scalar<FpType>(s, y));
        dyadic ref = est::exact_block(s) * est::exact_value(y);
        if (!(got == ref)) { std::cout << host << " mul_scalar mismatch\n"; ++n; }
        if (!mul_scalar<FpType>(block<FpType>{ static_cast<FpType>(0.0), 0 }, y).is_empty()) {
            std::cout << host << " mul_scalar(0,y) not empty\n"; ++n;
        }
    }
    return n;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal Phase 6 (#930) mul()";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_all<double>("mul<double>");
    nrOfFailedTestCases += verify_all<float>("mul<float>");
    nrOfFailedTestCases += verify_all<bfloat16>("mul<bfloat16>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
