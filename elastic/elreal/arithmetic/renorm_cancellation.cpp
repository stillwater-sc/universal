// renorm_cancellation.cpp: regression for #1044 -- priestRenorm must produce a
// 0-overlap (DBL_k) list even for a pool with catastrophic cancellation between
// nearly-equal high-precision values (a single Priest pass can leave the
// collapsed leading term closer than k to a following limb; priestRenorm now
// iterates the pass to a 0-overlap fixpoint).
//
// The canonical trigger is trig argument reduction x - n*(pi/2): for x near a
// multiple of pi/2 the leading limbs cancel.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/elreal/math/constants.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using sw::universal::block;

template <typename FpType>
int check_renorm(std::vector<block<FpType>> pool, double ref, double tol, const std::string& tag) {
    using namespace sw::universal;
    int n = 0;
    std::vector<block<FpType>> r = priestRenorm(pool);
    for (std::size_t i = 0; i + 1 < r.size(); ++i) {
        if (!zero_overlap(r[i], r[i + 1])) {
            std::cout << tag << " 0-overlap FAILED at block " << i
                      << " (E=" << r[i].exponent() << " then E=" << r[i + 1].exponent() << ")\n";
            ++n;
        }
    }
    double v = 0.0;
    for (const auto& b : r) v += b.template value_as<double>();
    if (std::abs(v - ref) > tol * std::max(1.0, std::abs(ref))) {
        std::cout << tag << " value " << v << " != " << ref << '\n'; ++n;
    }
    return n;
}

int verify() {
    using namespace sw::universal;
    int n = 0;

    // x - n*(pi/2) for x near a multiple of pi/2 (the trig-reduction trigger).
    auto half_pi = mul_scalar(block<double>{0.5, 0}, pi_zbcl<double>(4), 4);
    const double pi_2 = std::acos(-1.0) / 2.0;
    for (double x : { 1.5708, 3.14159, 4.71239, -1.5708 }) {     // ~ pi/2, pi, 3pi/2, -pi/2
        const double k = std::round(x / pi_2);
        std::vector<block<double>> pool;
        for (const auto& b : from_native<double>(x).take(20)) pool.push_back(b);
        ZBCL<double> nk = mul_scalar(block<double>{ k, 0 }, half_pi, 8);
        for (const auto& b : nk.take(20)) pool.push_back(block<double>{ -b.v, b.exp });
        n += check_renorm<double>(pool, x - k * pi_2, 1e-9, "x-n*pi/2 (x=" + std::to_string(x) + ")");
    }

    // direct catastrophic-cancellation pools
    n += check_renorm<double>({ block<double>{1.0, 0}, block<double>{-1.0, 0}, block<double>{std::ldexp(1.0, -35), 0} },
                              std::ldexp(1.0, -35), 1e-30, "[1,-1,2^-35]");
    return n;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal priestRenorm cancellation (regression #1044)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify();

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
