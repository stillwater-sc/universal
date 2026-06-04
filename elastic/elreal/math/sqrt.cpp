// sqrt.cpp: Phase 7.2 (#931) tests for sqrt() on ZBCL.
//
// Perfect squares are checked bit-exactly against the dyadic oracle; irrational
// roots are checked against std::sqrt and via the round-trip sqrt(a)^2 ~= a, to
// a host-scaled tolerance (the result is an approximation refined to ~depth
// blocks, and bfloat16's truncating arithmetic limits its accuracy). 0-overlap
// is verified on every result. Domain: sqrt(0)=0 (empty), sqrt(<0)=empty.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

#include <universal/verification/elreal_oracle.hpp>

namespace {

namespace est = sw::universal::elreal_oracle;

template <typename FpType>
int verify_all(double tol, const std::string& host) {
    using namespace sw::universal;
    int n = 0;

    // (1) Perfect squares -> exact (root and square representable in all hosts).
    struct { double sq, root; } perfect[] = {
        {4.0, 2.0}, {9.0, 3.0}, {16.0, 4.0}, {25.0, 5.0}, {100.0, 10.0}, {144.0, 12.0}
    };
    for (auto& p : perfect) {
        ZBCL<FpType> r = sqrt(from_native<FpType>(p.sq));
        if (!(est::exact_value(r) == est::exact_value(from_native<FpType>(p.root)))) {
            std::cout << host << " sqrt(" << p.sq << ") != " << p.root << '\n'; ++n;
        }
        n += est::check_zero_overlap<FpType>(r, 16, host + " sqrt-exact");
    }

    // (2) Irrational roots: value vs std::sqrt, and round-trip sqrt(a)^2 ~= a.
    for (double v : { 2.0, 3.0, 5.0, 7.0, 0.5 }) {
        ZBCL<FpType> a = from_native<FpType>(v);
        ZBCL<FpType> r = sqrt(a);
        if (std::abs(est::approx(r) - std::sqrt(v)) > tol) {
            std::cout << host << " sqrt(" << v << ") = " << est::approx(r)
                      << " != " << std::sqrt(v) << " (tol " << tol << ")\n"; ++n;
        }
        if (std::abs(est::approx(mul(r, r)) - v) > tol) {
            std::cout << host << " sqrt(" << v << ")^2 = " << est::approx(mul(r, r))
                      << " != " << v << '\n'; ++n;
        }
        n += est::check_zero_overlap<FpType>(r, 16, host + " sqrt-irr");
    }

    // (3) Domain: sqrt(0) = 0 (empty); sqrt(negative) = empty.
    if (!sqrt(ZBCL<FpType>{}).is_empty()) { std::cout << host << " sqrt(0) not empty\n"; ++n; }
    if (!sqrt(from_native<FpType>(-4.0)).is_empty()) { std::cout << host << " sqrt(-4) not empty\n"; ++n; }
    return n;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal Phase 7.2 (#931) sqrt()";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_all<double>(1e-12, "sqrt<double>");
    nrOfFailedTestCases += verify_all<float>(1e-6, "sqrt<float>");
    nrOfFailedTestCases += verify_all<bfloat16>(1e-4, "sqrt<bfloat16>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
