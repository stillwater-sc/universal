// add_renormalization.cpp: regression for #1034 -- add() must produce a fully
// 0-overlap (DBL_k) stream even when the result needs multiple blocks whose
// components are spaced closer than k.
//
// The bug: addRec_step's "one operand stream empty, workspace non-empty" cases
// drained the workspace without merging the still-pending blocks of the other
// operand, emitting out-of-order / overlapping blocks. Minimal trigger:
//
//   add(2^-90, [2^0, 2^-60])  ->  [2^0, 2^-90, 2^-60]   (WRONG: out of order)
//   correct (priestRenorm)    ->  [2^0, 2^-60]
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

using sw::universal::block;
using sw::universal::ZBCL;
// exact_block / exact_value: the shared exact-dyadic oracle (#1035).
using namespace sw::universal::elreal_oracle;

// 0-overlap check + descending-exponent sanity over the first n blocks.
template <typename FpType>
int check_canonical(const ZBCL<FpType>& z, std::size_t n, const std::string& tag) {
    using namespace sw::universal;
    auto b = z.take(n);
    int fails = 0;
    for (std::size_t i = 0; i + 1 < b.size(); ++i) {
        if (!zero_overlap(b[i], b[i + 1])) {
            std::cout << tag << " 0-overlap FAILED at block " << i
                      << " (E=" << b[i].exponent() << " then E=" << b[i + 1].exponent() << ")\n";
            ++fails;
        }
    }
    return fails;
}

// pow2 singleton ZBCL.
ZBCL<double> p2(int e) {
    using namespace sw::universal;
    return from_native<double>(std::ldexp(1.0, e));
}

// Build a 0-overlap multi-block operand directly from blocks (caller ensures
// the 0-overlap spacing).
ZBCL<double> stream(std::vector<block<double>> bs) {
    ZBCL<double> out{};
    for (std::size_t i = bs.size(); i-- > 0;) out = ZBCL<double>::cons(bs[i], out);
    return out;
}

int verify() {
    using namespace sw::universal;
    int n = 0;

    // (1) Minimal #1034 trigger: add a small block that lands between the two
    // blocks of a 0-overlap operand.
    {
        ZBCL<double> Y = stream({ block<double>{1.0, 0}, block<double>{std::ldexp(1.0, -60), 0} });
        ZBCL<double> r = add(p2(-90), Y);
        n += check_canonical(r, 16, "add(2^-90,[2^0,2^-60])");
        dyadic got = exact_value(r);
        dyadic ref = exact_value(Y);
        { dyadic t = dyadic::from_double(std::ldexp(1.0, -90)); ref = ref + t; }
        if (!(got == ref)) { std::cout << "add(2^-90,Y) value mismatch\n"; ++n; }
    }

    // (2) Fold of 2^(-30 i): components 30 apart must regroup into k-separated
    // blocks (k=53 for double): exps 0, -60, -120 (+ trailing zero).
    {
        ZBCL<double> acc{};
        std::vector<ZBCL<double>> terms;
        for (int i = 0; i < 6; ++i) terms.push_back(p2(-30 * i));
        for (int i = 5; i >= 0; --i) acc = add(terms[i], acc);
        n += check_canonical(acc, 16, "fold 2^(-30 i)");
        // exact value == sum of 2^(-30 i)
        dyadic ref;
        for (int i = 0; i < 6; ++i) { dyadic t = dyadic::from_double(std::ldexp(1.0, -30 * i)); ref = ref + t; }
        if (!(exact_value(acc) == ref)) { std::cout << "fold 2^(-30 i) value mismatch\n"; ++n; }
    }

    // (3) Operand smaller than the workspace tail, several levels.
    {
        ZBCL<double> Y = stream({ block<double>{1.0, 0},
                                  block<double>{std::ldexp(1.0, -60), 0},
                                  block<double>{std::ldexp(1.0, -120), 0} });
        ZBCL<double> r = add(p2(-90), Y);   // -90 lands between -60 and -120
        n += check_canonical(r, 16, "add(2^-90,[0,-60,-120])");
        dyadic got = exact_value(r);
        dyadic ref = exact_value(Y);
        { dyadic t = dyadic::from_double(std::ldexp(1.0, -90)); ref = ref + t; }
        if (!(got == ref)) { std::cout << "add(2^-90,[0,-60,-120]) value mismatch\n"; ++n; }
    }

    return n;
}

// verify_1057: add() must emit the unique 0-overlap CANONICAL form, matching
// priestRenorm. Regression for #1057: addRec_step's gs/fs-empty re-injection used to
// emit the workspace head unconditionally, but a prior twoSum merge can lower that
// head's exponent toward an un-merged operand block (with a carry, the operand can even
// land one bit above), leaving an adjacent pair closer than k -- a value-correct but
// non-canonical (0-overlap-violating) result. The fix folds any overlapping operand
// block into the workspace (accounting for a carry, hence ">=") before emitting.
int verify_1057() {
    using namespace sw::universal;
    int n = 0;

    // Minimal repro: a 5-block canonical operand + one block inserted near the top.
    {
        ZBCL<double> a = zbcl_from_blocks<double>(priestRenorm(std::vector<block<double>>{
            block<double>{-1.6585293272722507751,        200},
            block<double>{ 9.8712660049348273631e-17,    200},
            block<double>{-2.4280874679101948698e-33,    200},
            block<double>{ 1.1460774358321607147e-53,    200},
            block<double>{ 5.2596609909677645217e-70,    200} }));
        ZBCL<double> b = ZBCL<double>::singleton(block<double>{3.2964886804071818101e58, 0});
        ZBCL<double> s = add(a, b);
        n += check_canonical(s, 16, "#1057 minimal-repro add()");
        if (exact_value(s) != exact_value(a) + exact_value(b)) {
            std::cout << "#1057 minimal-repro value mismatch\n"; ++n;
        }
    }

    // Deterministic fuzz: canonical multi-block operand + a single block; add() must be
    // 0-overlap and value-exact against the dyadic oracle. (50k at sanity level.)
    {
        std::mt19937_64 rng(20260605);
        std::uniform_int_distribution<int> nb(1, 5), stepd(53, 70), sd(0, 1);
        std::uniform_real_distribution<double> md(1.0, 2.0);
        const int trials = REGRESSION_LEVEL_1 ? 50000 : 5000;
        for (int t = 0; t < trials; ++t) {
            std::vector<block<double>> ab; int e = 200, na = nb(rng);
            for (int i = 0; i < na; ++i) { ab.push_back(block<double>{ md(rng) * (sd(rng) ? 1.0 : -1.0), e }); e -= stepd(rng); }
            ZBCL<double> a = zbcl_from_blocks<double>(priestRenorm(ab));
            int be = 200 - static_cast<int>(rng() % static_cast<unsigned>(na * 70 + 80));
            ZBCL<double> b = from_native<double>(std::ldexp(md(rng) * (sd(rng) ? 1.0 : -1.0), be));
            ZBCL<double> s = add(a, b);
            if (check_canonical(s, 32, "#1057 fuzz") > 0) { ++n; break; }
            if (exact_value(s) != exact_value(a) + exact_value(b)) { std::cout << "#1057 fuzz value mismatch at t=" << t << "\n"; ++n; break; }
        }
    }
    return n;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal add() renormalization (regression #1034, #1057)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify();
    nrOfFailedTestCases += verify_1057();

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
