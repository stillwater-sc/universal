// online_muldiv.cpp: validation of the CANONICAL streaming (online, pull-driven)
// infSum, multiply, and divide against the (deprecated) eager reference and the
// exact dyadic oracle (#1061 phase 1).
//
// LFPERA is online by design; these streaming ops are the faithful realization
// and the eager mul/div/sum they are checked against are deprecated scaffolding
// on the way out (see docs/design/elreal-online-convergence.md). The eager
// versions serve here purely as an independent cross-check oracle.
//
// Scope of what is validated here (the streaming ops, by current completeness):
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
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/elreal_oracle.hpp>
#include <universal/verification/elreal_reference_digits.hpp>   // agreed_decimal_digits
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

// (5) DENSE multi-block divisor (blocks NOT powers of two), shallow. Dense divisors
// route to the Newton-Raphson reciprocal path (a/b = a*(1/b), #1068): the faithful
// long division cost-explodes for them. This checks the shallow prefix matches eager
// div(); verify_div_dense_deep below exercises the full (capped) depth.
int verify_div_dense_shallow() {
    int n = 0;
    // 2-block operands with non-power-of-two low blocks.
    ZBCL<double> a = add(nat(1.357630), nat(std::ldexp(1.400440, -58)));
    ZBCL<double> b = add(nat(1.689380), nat(std::ldexp(1.559740, -57)));
    ZBCL<double> q = div_online(a, b);
    const std::size_t W = 4;
    auto blocks = q.take(W);
    if (blocks.size() < W) {
        std::cout << "dense div: only " << blocks.size() << " blocks (<4)\n"; ++n;
    }
    n += check_canonical(q, W, "div_online dense-shallow");
    ZBCL<double> qe = div(a, b, 8);
    long double rel = std::fabs(zval(q, W) - zval(qe, W));
    long double mag = std::fabs(zval(qe, W)) + 1e-300L;
    if (rel > mag * 1e-13L) {
        std::cout << "dense div != eager (rel=" << static_cast<double>(rel) << ")\n"; ++n;
    }
    return n;
}

// (5b) DENSE divisor, DEEP (Newton-Raphson reciprocal path, #1068). Before the
// Newton routing a dense divisor's long division did not terminate past ~7 blocks.
// With the streaming-multiply host-floor arrest (#1068) the dense quotient now refines
// to the host floor -- ~17 components / ~265 digits for a double host, the same region
// as the single-block path -- 0-overlap canonical, reconstructing q*b == a. (Earlier it
// was capped at ~8 blocks because mul_online emitted subnormal blocks that broke
// 0-overlap; singleMultHelper now drops those at the source.) Regression against
// re-introducing the fan-out (would hang), the subnormal 0-overlap break, or breaking
// the reciprocal (recon would drift).
int verify_div_dense_deep() {
    int n = 0;
    const struct { double a, b; } cases[] = {
        {1.357630, 1.689380}, {2.718281, 3.141592}, {9.876540, 0.333111}
    };
    for (const auto& c : cases) {
        ZBCL<double> a = add(nat(c.a), nat(std::ldexp(1.23, -58)));
        ZBCL<double> b = add(nat(c.b), nat(std::ldexp(1.71, -57)));   // dense (non-power-of-two)
        ZBCL<double> q = div_online(a, b);                            // Newton path
        auto blocks = q.take(24);

        // Refines deep into the host's representable range (not the old ~8-block cap).
        if (blocks.size() < 14) {
            std::cout << "dense-deep div(" << c.a << "/" << c.b << "): only "
                      << blocks.size() << " blocks (<14: host-floor arrest regressed?)\n"; ++n;
        }
        const long lastE = blocks.empty() ? 0 : static_cast<long>(static_cast<int>(blocks.back().exponent()));
        if (lastE > -750) {
            std::cout << "dense-deep div(" << c.a << "/" << c.b << "): lastE=" << lastE
                      << " (> -750: quotient truncated above the host floor)\n"; ++n;
        }
        // Every block normal (the subnormal blocks that broke 0-overlap are gone) and
        // 0-overlap canonical the whole way down.
        for (const auto& bl : blocks) {
            if (!bl.is_normalised() && !bl.is_zero_block()) {
                std::cout << "dense-deep div(" << c.a << "/" << c.b
                          << "): subnormal block at E=" << static_cast<long>(static_cast<int>(bl.exponent())) << "\n"; ++n;
            }
        }
        n += check_canonical(q, blocks.size(), "div_online dense-deep");

        // Reconstruction q*b == a, now exercised deep (mul_online is 0-overlap to the
        // host floor): exact match over the shared prefix via the dyadic oracle.
        ZBCL<double> recon = mul_online(q, b);
        long double resid = std::fabs(zval(a, 16) - zval(recon, 16));
        long double mag   = std::fabs(zval(a, 16)) + 1e-300L;
        if (resid > mag * 1e-13L) {
            std::cout << "dense-deep div(" << c.a << "/" << c.b
                      << "): |a - q*b|/|a| = " << static_cast<double>(resid / mag)
                      << " (reconstruction drifted)\n"; ++n;
        }
    }
    return n;
}

// (6) DEEP reach of the lazy, pull-driven operator (#1061 div host-floor lift).
// The whole point of online div is on-demand precision: pulling deeper must keep
// refining, not stop at an artificial floor. Before the host-floor was gated to
// narrow hosts only, twoDivZBCL's min_exp+2k guard capped a single-block quotient
// at ~17 blocks / ~260 digits on a double host -- ~33 digits short of the eager
// div()'s reach. This asserts the lazy quotient now reaches the host's natural
// ~19-component ceiling, exactly matches eager div() block-for-block over the
// shared prefix, and stays 0-overlap the whole way down.
int verify_div_deep_reach() {
    int n = 0;
    const double cases[][2] = { {1, 3}, {1, 7}, {22, 7}, {355, 113} };
    for (const auto& c : cases) {
        ZBCL<double> q_on = div_online(nat(c[0]), nat(c[1]));
        ZBCL<double> q_eg = div(nat(c[0]), nat(c[1]), 40);   // eager, floor already lifted
        auto on = q_on.take(40);
        auto eg = q_eg.take(40);

        // Reach: a wide host (double, k=53) must refine to its ~19-component
        // ceiling, not stop at the old min_exp+2k (~-915, ~17 blocks) floor.
        if (on.size() < 19) {
            std::cout << "deep div_online(" << c[0] << "/" << c[1] << "): only "
                      << on.size() << " blocks (<19: host-floor not lifted?)\n"; ++n;
        }
        const long lastE = on.empty() ? 0 : static_cast<long>(static_cast<int>(on.back().exponent()));
        if (lastE > -950) {
            std::cout << "deep div_online(" << c[0] << "/" << c[1] << "): lastE=" << lastE
                      << " (> -950: quotient truncated above the host ceiling)\n"; ++n;
        }

        // 0-overlap all the way down (the floor's stated reason for existing).
        n += check_canonical(q_on, 19, "div_online deep");

        // Lazy must equal eager block-for-block over the shared prefix: same
        // exponents AND same significands (exact, via the dyadic oracle).
        const std::size_t W = std::min(on.size(), eg.size());
        ZBCL<double> on_p{}, eg_p{};
        for (std::size_t i = W; i-- > 0;) { on_p = ZBCL<double>::cons(on[i], on_p); eg_p = ZBCL<double>::cons(eg[i], eg_p); }
        if (exact_value(on_p) != exact_value(eg_p)) {
            std::cout << "deep div_online(" << c[0] << "/" << c[1]
                      << "): lazy != eager over " << W << "-block prefix\n"; ++n;
        }
    }
    return n;
}

// A DENSE-divisor quotient must reach the depth the CALLER asks for, on every host.
// It used to stop at a depth derived from FpType's exponent range -- 17 blocks on
// double, 3 on float -- regardless of how deep the caller pulled (#1371). That is the
// premise #1362 removed elsewhere: a block carries its scale in a wide integer<256>
// exponent, so min_exponent bounds nothing about an expansion's depth.
template <typename FpType>
int verify_div_dense_honours_depth(const char* host) {
    using namespace sw::universal;
    int n = 0;
    // sqrt(2) is a genuinely dense multi-block divisor (no power-of-two blocks).
    for (std::size_t d : { std::size_t(8), std::size_t(16), std::size_t(32), std::size_t(48) }) {
        ZBCL<FpType> b = sqrt(from_native<FpType>(2.0), d);
        if (!is_dense_divisor(b)) {           // guard the premise of this test
            std::cout << "dense-depth [" << host << "] d=" << d
                      << ": sqrt(2) is not classified dense -- test no longer exercises the Newton path\n";
            ++n; continue;
        }
        const std::size_t got = div_online(from_native<FpType>(2.0), b, d).take(4 * d + 8).size();
        if (got < d) {
            std::cout << "dense-depth [" << host << "] requested " << d << " blocks, got " << got
                      << " (the #1371 host-derived cap is back: 17 on double, 3 on float)\n";
            ++n;
        }
    }
    return n;
}

// The class facade must propagate precision() into division. Without it, elreal's
// operator/ was pinned to that same host constant no matter what the object asked for,
// which stalled a Newton iteration at ~282 digits on double and ~22 on float.
template <typename FpType>
int verify_facade_division_precision(const char* host) {
    using namespace sw::universal;
    int n = 0;
    for (std::size_t d : { std::size_t(24), std::size_t(48) }) {
        elreal<FpType> num(2.0);
        elreal<FpType> den(ZBCL<FpType>(sqrt(from_native<FpType>(2.0), d)), d);
        num.precision(d);
        const std::size_t got = (num / den).stream().take(4 * d + 8).size();
        if (got < d) {
            std::cout << "facade-precision [" << host << "] precision(" << d << ") but quotient has "
                      << got << " blocks (precision() not reaching div_online)\n";
            ++n;
        }
    }
    return n;
}

// mul_online must be invariant to INTERIOR ZERO BLOCKS in an operand. Zero blocks are
// legitimate ZBCL blocks, so two expansions can carry the EXACT SAME VALUE and differ
// only in how many of them they hold -- the product must not notice. It used to: the
// streaming sum's null-sum branch discarded an unconsumed term when a term summed to
// zero, so an operand with interior zero blocks lost everything past them (#1373).
template <typename FpType>
int verify_mul_ignores_zero_blocks(const char* host) {
    using namespace sw::universal;
    using B = block<FpType>;
    int n = 0;
    // same value, written with and without interior zero blocks
    const int deep = 40 * B::k;                      // far below the leading block
    std::vector<B> with  = { B{ FpType(1.0), typename B::exp_t(0) },
                             B{ FpType(0.0), typename B::exp_t(-2 * B::k) },
                             B{ FpType(0.0), typename B::exp_t(-3 * B::k) },
                             B{ FpType(0.0), typename B::exp_t(-4 * B::k) },
                             B{ FpType(1.0), typename B::exp_t(-deep) } };
    std::vector<B> clean = { B{ FpType(1.0), typename B::exp_t(0) },
                             B{ FpType(1.0), typename B::exp_t(-deep) } };
    ZBCL<FpType> zw = zbcl_from_blocks<FpType>(with), zc = zbcl_from_blocks<FpType>(clean);
    // premise: they really are the same value
    if (agreed_decimal_digits(zw, zc, 2000) < 2000) {
        std::cout << "zero-blocks [" << host << "]: the two operands are not the same value"
                  << " -- test premise broken\n"; return 1;
    }
    ZBCL<FpType> m = sqrt(from_native<FpType>(2.0), 8);
    ZBCL<FpType> pw = zbcl_from_blocks<FpType>(mul_online(m, zw).take(64));
    ZBCL<FpType> pc = zbcl_from_blocks<FpType>(mul_online(m, zc).take(64));
    const int agree = agreed_decimal_digits(pw, pc, 2000);
    if (agree < 2000) {
        std::cout << "zero-blocks [" << host << "]: products differ (" << agree
                  << " digits) -- interior zero blocks truncated the product (#1373)\n";
        ++n;
    }
    return n;
}

// A dense-divisor quotient must keep resolving with depth. Before #1373 the Newton
// reciprocal fixed-pointed, capping the quotient at ~513 decimal digits on a double
// host and ~62 on float NO MATTER what depth was requested -- so a depth whose
// capacity exceeds that cap is what distinguishes fixed from broken. 2/sqrt(2) is
// sqrt(2), so the divisor is its own reference and no decimal constant is needed.
template <typename FpType>
int verify_dense_div_resolves_with_depth(const char* host, std::size_t depth, int oldCap) {
    using namespace sw::universal;
    int n = 0;
    ZBCL<FpType> b = sqrt(from_native<FpType>(2.0), depth);
    ZBCL<FpType> q = zbcl_from_blocks<FpType>(div_online(from_native<FpType>(2.0), b, depth).take(depth + 4));
    const int agree = agreed_decimal_digits(q, b, 4000);
    // the quotient reproduces the divisor to most of the divisor's own precision
    const int want = static_cast<int>(0.90 * double(depth) * double(block<FpType>::k) * 0.30103);
    if (agree < want) {
        std::cout << "dense-div [" << host << "] depth " << depth << ": 2/sqrt(2) reproduces sqrt(2) to only "
                  << agree << " digits (want >= " << want << "; the #1373 cap was ~" << oldCap << ")\n";
        ++n;
    }
    return n;
}

// Dividing a multi-block value by a SINGLE block must stay LINEAR in the requested
// depth. It used to be quadratic: each dividend block was divided independently to a
// full stream and D of them were summed, each carried to the output frontier, so
// D quotient blocks cost D^2/2 block divisions. The schoolbook carry makes it O(1)
// per emitted block.
//
// Guarded by a RATIO across a 4x span rather than an absolute time, so it does not
// depend on machine speed. Linear predicts ~4x; the quadratic form measured 16-30x
// over the same span. The threshold sits well clear of both, so ordinary CI noise
// cannot trip it.
//
// Checked on the double host only. The COUNT of block divisions is linear on both --
// 1.00 per emitted block on double and 1.06-1.09 on float -- but float carries a
// larger running remainder, and that per-step overhead leaves its wall-clock ratio
// around 6-10x rather than 4x. Asserting on float would either be flaky or need a
// threshold too loose to catch anything, so the guard rides on the host where the
// signal is clean; a regression to the per-block-then-sum form would show on both.
template <typename FpType>
int verify_single_block_division_is_linear(const char* host) {
    using namespace sw::universal;
    using clk = std::chrono::steady_clock;
    // The produced block count is checked, not discarded: a divider that terminated
    // early would do less work and sail through a pure timing ratio.
    int shortfall = 0;
    auto cost = [&shortfall](std::size_t D) {
        ZBCL<FpType> big = zbcl_from_blocks<FpType>(
            div_online(from_native<FpType>(1.0), from_native<FpType>(7.0)).take(D));
        const auto t0 = clk::now();
        const std::size_t n =
            zbcl_from_blocks<FpType>(div_online(big, from_native<FpType>(5.0)).take(D)).take(D).size();
        const auto t1 = clk::now();
        if (n < D) {
            std::cout << "single-block division: asked for " << D << " blocks, got " << n << '\n';
            ++shortfall;
        }
        return std::chrono::duration<double, std::milli>(t1 - t0).count();
    };
    (void)cost(40);                                  // warm caches / first-touch
    const double small = cost(40), large = cost(160);
    if (shortfall) return shortfall;
    const double ratio = (small > 0.0) ? large / small : 0.0;
    constexpr double kMaxRatio = 8.0;                // linear ~4, quadratic ~16-30
    if (ratio > kMaxRatio) {
        std::cout << "single-block division [" << host << "] scaled x" << ratio
                  << " over a 4x depth span (want <= " << kMaxRatio
                  << "; the O(D^2) per-block-then-sum form is back)\n";
        return 1;
    }
    return 0;
}

// ... and it must still STREAM. The quotient of 1/3 never terminates, so the
// implementation has to keep producing blocks for as long as the caller pulls; a
// depth-budgeted rewrite would silently cap it.
template <typename FpType>
int verify_single_block_division_streams(const char* host) {
    using namespace sw::universal;
    int n = 0;
    ZBCL<FpType> q = div_online(from_native<FpType>(1.0), from_native<FpType>(3.0));
    const std::vector<block<FpType>> bl = q.take(400);
    if (bl.size() < 400) {
        std::cout << "single-block division [" << host << "] produced only " << bl.size()
                  << " blocks of 1/3, expected the stream to keep going to 400\n";
        ++n;
    }
    // ... and every block it hands out must be canonical. This is what the pending
    // buffer exists for: the raw quotient blocks are 0-overlap on a double host but
    // roughly 2 in 60 land a bit too close on float, so a length check alone would
    // not notice the buffer being removed.
    for (std::size_t i = 1; i < bl.size(); ++i) {
        if (!zero_overlap(bl[i - 1], bl[i])) {
            std::cout << "single-block division [" << host << "] block " << i
                      << " overlaps its predecessor (exponents " << bl[i - 1].exponent()
                      << ", " << bl[i].exponent() << "; k = " << block<FpType>::k
                      << ") -- the quotient is not canonical\n";
            ++n;
            break;
        }
    }
    return n;
}

// A NARROW host needs both division operands prepared with bias_for_eft(), not just
// normalise(). block_two_div_rem forms the residual in host arithmetic, and a host
// whose exponent span is under 2k has no room below a normalised operand for it, so
// the residual denormalises and twoDiv's "e is >= k below s" guarantee fails.
//
// This only bites where block::eft_scale_bias() is non-zero -- 8 on half, 0 on
// bfloat16, float and double -- so double/float coverage cannot see it. Measured on
// half with the bias omitted: 13 of 80 random cases diverge and 12 get materially
// worse, e.g. a quotient good to 139 digits collapsing to 4.
//
// half here is IEEE binary16 with subnormals, the configuration elreal's narrow-host
// work (#1363/#1366) targets.
using elreal_half = sw::universal::cfloat<16, 5, std::uint16_t, true, false, false>;

template <typename FpType>
int verify_narrow_host_division(const char* host) {
    using namespace sw::universal;
    int n = 0;
    // pairs chosen from the divergent set found by sweeping the bias on/off
    const int cases[][3] = { { 54, 51, 9 }, { 30, 29, 33 }, { 32, 31, 12 }, { 7, 3, 16 } };
    for (const auto& c : cases) {
        const std::size_t D = static_cast<std::size_t>(c[2]);
        ZBCL<FpType> big = zbcl_from_blocks<FpType>(
            div_online(from_native<FpType>(1.0), from_native<FpType>(double(c[0]))).take(D));
        if (big.take(1).empty()) continue;
        ZBCL<FpType> q = zbcl_from_blocks<FpType>(
            div_online(big, from_native<FpType>(double(c[1]))).take(D));
        // independent check: q * divisor must reproduce the dividend
        const int agree = agreed_decimal_digits(
            mul(q, from_native<FpType>(double(c[1])), 2 * D + 8), big, 4000);
        const int want = static_cast<int>(0.60 * double(D) * double(block<FpType>::k) * 0.30103);
        if (agree < want) {
            std::cout << "narrow-host division [" << host << "] 1/" << c[0] << " / " << c[1]
                      << " at depth " << D << ": q*divisor reproduces the dividend to only "
                      << agree << " digits (want >= " << want
                      << "; are both operands bias_for_eft'd?)\n";
            ++n;
        }
    }
    return n;
}

} // anonymous

#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal streaming infSum / multiply / divide (#1061)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_infsum();
    nrOfFailedTestCases += verify_mul();
    nrOfFailedTestCases += verify_div_single();
    nrOfFailedTestCases += verify_div_sparse_multiblock();
    nrOfFailedTestCases += verify_div_dense_shallow();
    nrOfFailedTestCases += verify_div_dense_deep();
    nrOfFailedTestCases += verify_div_deep_reach();
    nrOfFailedTestCases += verify_div_dense_honours_depth<double>("double");
    nrOfFailedTestCases += verify_div_dense_honours_depth<float>("float");
    nrOfFailedTestCases += verify_facade_division_precision<double>("double");
    nrOfFailedTestCases += verify_facade_division_precision<float>("float");
    nrOfFailedTestCases += verify_mul_ignores_zero_blocks<double>("double");
    nrOfFailedTestCases += verify_mul_ignores_zero_blocks<float>("float");
    // depth 40 -> capacity ~638 digits, comfortably above the old ~513 cap while
    // keeping the check affordable for the fast CI tier.
    nrOfFailedTestCases += verify_dense_div_resolves_with_depth<double>("double", 40, 513);
    nrOfFailedTestCases += verify_dense_div_resolves_with_depth<float>("float", 24, 62);
    nrOfFailedTestCases += verify_single_block_division_is_linear<double>("double");
    nrOfFailedTestCases += verify_single_block_division_streams<double>("double");
    nrOfFailedTestCases += verify_single_block_division_streams<float>("float");
    nrOfFailedTestCases += verify_narrow_host_division<elreal_half>("half");

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
