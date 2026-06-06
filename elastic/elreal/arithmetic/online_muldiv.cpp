// online_muldiv.cpp: validation of the streaming (online, pull-driven) infSum,
// multiply, and divide against the eager reference and the exact dyadic oracle
// (#1061 phase 1).
//
// Scope of what is validated here (the landed, working subset):
//   * infsum(series)        == eager sum() exactly (finite series).
//   * mul_online(a, b)      == exact product a*b (finite operands).
//   * div_online, single-block divisor: matches eager div() to host precision,
//     0-overlap canonical.
//   * div_online, SPARSE (power-of-two) multi-block divisor: full-depth quotient,
//     0-overlap canonical, exact reconstruction q*b == numerator.
//
// NOT exercised: GENERAL dense multi-block divisors. Those are not yet supported
// (a 0-overlap correctness bug plus a cost explosion -- the running divisor's
// block count grows per level). See the online_divide.hpp banner. Calling
// div_online on a dense multi-block divisor does not terminate in reasonable
// time, so this test deliberately avoids it.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/elreal_oracle.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
using namespace sw::universal::elreal_oracle;

// sum a ZBCL prefix as long double (host-precision spot check).
long double zval(const ZBCL<double>& z, std::size_t W = 24) {
    auto b = z.take(W);
    long double s = 0;
    for (const auto& x : b) s += x.value_as<long double>();
    return s;
}

// 0-overlap over the first n blocks.
int check_canonical(const ZBCL<double>& z, std::size_t n, const std::string& tag) {
    auto b = z.take(n);
    int fails = 0;
    for (std::size_t i = 0; i + 1 < b.size(); ++i) {
        if (!zero_overlap(b[i], b[i + 1])) {
            std::cout << tag << " 0-overlap FAILED at block " << i
                      << " (E=" << static_cast<long long>(static_cast<int>(b[i].exponent()))
                      << " then E=" << static_cast<long long>(static_cast<int>(b[i + 1].exponent())) << ")\n";
            ++fails;
        }
    }
    return fails;
}

ZBCL<double> nat(double v) { return from_native<double>(v); }

// (1) infsum == eager sum (exact, finite series).
int verify_infsum() {
    int n = 0;
    std::mt19937_64 rng(20260606);
    std::uniform_real_distribution<double> md(-2.0, 2.0);
    std::uniform_int_distribution<int>     ed(-40, 40);
    const int trials = REGRESSION_LEVEL_1 ? 2000 : 200;
    for (int t = 0; t < trials; ++t) {
        std::vector<ZBCL<double>> terms;
        int e = 60;
        const int nt = 3 + static_cast<int>(rng() % 5);
        for (int i = 0; i < nt; ++i) { terms.push_back(nat(std::ldexp(md(rng), e))); e -= 55 + (ed(rng) & 7); }
        series<double> s = series_from_vector<double>(terms);
        ZBCL<double> online = infsum(s);
        ZBCL<double> eager  = sum(series_from_vector<double>(terms), terms.size() + 1);
        if (exact_value(online) != exact_value(eager)) {
            std::cout << "infsum != sum at t=" << t << "\n"; ++n; break;
        }
    }
    return n;
}

// (2) mul_online == exact product (finite operands).
int verify_mul() {
    int n = 0;
    std::mt19937_64 rng(20260607);
    std::uniform_real_distribution<double> md(1.0, 2.0);
    std::uniform_int_distribution<int>     sd(0, 1);
    const int trials = REGRESSION_LEVEL_1 ? 5000 : 500;
    for (int t = 0; t < trials; ++t) {
        // 2-block operands (a 0-overlap pair) so the exact product is multi-block.
        ZBCL<double> a = add(nat(md(rng) * (sd(rng) ? 1 : -1)), nat(std::ldexp(md(rng), -57)));
        ZBCL<double> b = add(nat(md(rng) * (sd(rng) ? 1 : -1)), nat(std::ldexp(md(rng), -58)));
        ZBCL<double> p = mul_online(a, b);
        if (exact_value(p) != exact_value(a) * exact_value(b)) {
            std::cout << "mul_online != exact product at t=" << t << "\n"; ++n; break;
        }
        if (check_canonical(p, 8, "mul_online") > 0) { ++n; break; }
    }
    return n;
}

// (3) single-block divisor: div_online matches eager div() to host precision.
int verify_div_single() {
    int n = 0;
    const struct { double a, b; } cases[] = {
        {1, 7}, {1, 3}, {22, 7}, {2, 3}, {355, 113}, {-5, 9}, {1, 1024}, {7, 2}
    };
    for (const auto& c : cases) {
        ZBCL<double> q_on = div_online(nat(c.a), nat(c.b));
        ZBCL<double> q_eg = div(nat(c.a), nat(c.b), 24);
        long double rel = std::fabs(zval(q_on) - zval(q_eg));
        long double mag = std::fabs(zval(q_eg)) + 1e-300L;
        if (rel > mag * 1e-14L) {
            std::cout << "div_online(" << c.a << "/" << c.b << ") != eager (rel="
                      << static_cast<double>(rel) << ")\n"; ++n;
        }
        if (check_canonical(q_on, 16, "div_online single") > 0) ++n;
    }
    // exact ratio terminates and reconstructs exactly: 6/3 == 2.
    {
        ZBCL<double> q = div_online(nat(6.0), nat(3.0));
        if (exact_value(q) != dyadic::from_double(2.0)) { std::cout << "6/3 != 2\n"; ++n; }
    }
    return n;
}

// (4) sparse (power-of-two) multi-block divisor: full-depth, 0-overlap, exact
// reconstruction. This is the direct payoff of the wide block exponent (#1066):
// before, int32 overflow capped this at ~11 blocks.
int verify_div_sparse_multiblock() {
    int n = 0;
    // b = 1 + 2^-55 (two power-of-two blocks). a = 1.
    ZBCL<double> b = add(nat(1.0), nat(std::ldexp(1.0, -55)));
    ZBCL<double> a = nat(1.0);
    ZBCL<double> q = div_online(a, b);
    auto blocks = q.take(20);
    if (blocks.size() < 12) {
        std::cout << "sparse multi-block div: only " << blocks.size()
                  << " blocks (<12: wide-exponent regression?)\n"; ++n;
    }
    n += check_canonical(q, 20, "div_online sparse-multiblock");
    // reconstruction: q * b == a (== 1) to the depth of the quotient prefix.
    ZBCL<double> qz{};
    { auto bs = q.take(18); for (std::size_t i = bs.size(); i-- > 0;) qz = ZBCL<double>::cons(bs[i], qz); }
    ZBCL<double> prod = mul_online(qz, b);
    long double resid = std::fabs(1.0L - zval(prod, 30));
    if (resid > 1e-15L) {
        std::cout << "sparse multi-block div reconstruction |1 - q*b| = "
                  << static_cast<double>(resid) << " (too large)\n"; ++n;
    }
    return n;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal streaming infSum / multiply / divide (#1061)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_infsum();
    nrOfFailedTestCases += verify_mul();
    nrOfFailedTestCases += verify_div_single();
    nrOfFailedTestCases += verify_div_sparse_multiblock();

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
