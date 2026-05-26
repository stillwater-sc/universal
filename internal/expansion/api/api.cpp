// api.cpp: API demonstration of expansion (multi-component) arithmetic
//
// This file is the curated, pedagogical entry point for the expansion API. It
// demonstrates the three error-free transformations (EFTs) that all expansion
// arithmetic is built on, making the "error-free" property explicit by
// reconstructing the exact result from the (head, tail) pair each one returns:
//
//     two_sum   :  a + b  ==  s + r      (sum  s, exact roundoff r)
//     two_prod  :  a * b  ==  p + e      (prod p, exact roundoff e)
//     two_div   :  a      ==  q*b + r    (quot q = fl(a/b), exact residual r)
//
// and then shows how these compose into multi-component expansions that carry
// far more than 53 bits of precision.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/internal/expansion/expansion_ops.hpp>
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_case.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cmath>

namespace {

	void print_expansion(const std::string& label, const std::vector<double>& e) {
		std::cout << "    " << std::left << std::setw(20) << label
		          << "[" << e.size() << " limb(s)] {";
		std::cout << std::setprecision(17) << std::scientific;
		for (size_t i = 0; i < e.size(); ++i) {
			if (i) std::cout << ", ";
			std::cout << e[i];
		}
		std::cout << "}\n";
	}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::expansion_ops;

	std::cout << "Expansion arithmetic API -- error-free transformations\n";
	std::cout << "======================================================\n\n";

	// ---------------------------------------------------------------------
	// TWO-SUM:  a + b == s + r exactly.  s is fl(a+b); r is the part that
	// rounding dropped.  Pick a + b that is NOT representable as one double.
	// ---------------------------------------------------------------------
	std::cout << "two_sum:  a + b == s + r   (s = fl(a+b), r = exact roundoff)\n";
	std::cout << "-----------------------------------------------------------\n";
	{
		double a = 9007199254740992.0;  // 2^53
		double b = 1.0;                 // 2^53 + 1 is NOT a double
		double s, r;
		two_sum(a, b, s, r);
		ReportValue(a, "a =", 25u, 17u);
		ReportValue(b, "b =", 25u, 17u);
		ReportValue(s, "s = fl(a+b) =", 25u, 17u);          // rounds back to 2^53; b appears lost
		ReportValue(r, "r = roundoff =", 25u, 17u);        // == 1.0: exactly what s dropped
		ReportValue(a+b, "naive (a+b) in double =", 25u, 17u);  // == s: the +1 is gone
		std::cout << "    => the single double a+b loses b, but the pair (s,r) holds 2^53+1 exactly.\n";
		einteger s_r = einteger(s) + einteger(r);
		einteger a_b = einteger(a) + einteger(b);
		ReportValue(s_r, "s + r =", 25, 17u);
		ReportValue(a_b, "a + b =", 25, 17u);
		bool     exact = (s_r == a_b);
		std::cout << "    => identity: s + r == a + b, reconstructed exactly (dyadic check: "
		          << (exact ? "holds" : "FAILS") << ").\n\n";
	}

	// A fractional case: 1 + 2^-53 (also not a double); r carries the tail.
	{
		double a = 1.0, b = std::ldexp(1.0, -53);
		double s, r;
		two_sum(a, b, s, r);
		ReportValue(a, "a =", 25, 17);
		ReportValue(b, "b = 2^-53 =", 25, 17);
		ReportValue(s, "s =", 25, 17);  // 1.0
		ReportValue(r, "r =", 25, 17);  // 2^-53, the dropped tail
		std::cout << "    => (s,r) = (1, 2^-53) represents 1 + 2^-53 exactly.\n\n";
	}

	// ---------------------------------------------------------------------
	// FAST-TWO-SUM:  same identity, 3 ops, valid only when |a| >= |b|.
	// ---------------------------------------------------------------------
	std::cout << "fast_two_sum:  a + b == s + r   (requires |a| >= |b|)\n";
	std::cout << "-----------------------------------------------------\n";
	{
		double a = 1.0e16, b = 7.0;   // |a| >= |b|
		double s, r;
		fast_two_sum(a, b, s, r);
		ReportValue(a, "a =", 25, 17);
		ReportValue(b, "b =", 25, 17);
		ReportValue(s, "s =", 25, 17);
		ReportValue(r, "r =", 25, 17);
		std::cout << "    => 3-op version; reuse only with magnitude-ordered operands.\n\n";
	}

	// ---------------------------------------------------------------------
	// TWO-PROD:  a * b == p + e exactly.  Squaring a rounded sqrt(2) shows the
	// product is not 2, and e captures the exact low-order bits.
	// ---------------------------------------------------------------------
	std::cout << "two_prod:  a * b == p + e   (p = fl(a*b), e = exact roundoff)\n";
	std::cout << "------------------------------------------------------------\n";
	{
		double a = std::sqrt(2.0);    // the rounded square root
		double b = a;
		double p, e;
		two_prod(a, b, p, e);
		ReportValue(a, "a = fl(sqrt 2) =", 25, 17);
		ReportValue(p, "p = fl(a*a) =", 25, 17);  // ~2, but not exactly 2
		ReportValue(e, "e = roundoff =", 25, 17);  // the bits below p
		ReportValue(p - 2.0, "p - 2.0 =", 25, 17);  // shows p != 2
		std::cout << "    => a*a is not 2; the exact product a*a == p + e (e holds the tail).\n\n";
	}
	{
		double a = 0.1, b = 0.3;      // neither is exact in binary
		double p, e;
		two_prod(a, b, p, e);
		ReportValue(a, "a =", 25, 17);
		ReportValue(b, "b =", 25, 17);
		ReportValue(p, "p = fl(a*b) =", 25, 17);
		ReportValue(e, "e = roundoff =", 25, 17);
		std::cout << "    => p + e is the exact product of the two stored doubles.\n\n";
	}

	// ---------------------------------------------------------------------
	// TWO-DIV (residual form):  a == q*b + r,  q = fl(a/b),  r = fma(-q,b,a).
	// fma computes a - q*b with a single rounding, so r is the EXACT residual
	// -- division's rounding error is recoverable even though a/b is inexact.
	// ---------------------------------------------------------------------
	std::cout << "two_div:  a == q*b + r   (q = fl(a/b), r = fma(-q,b,a) exact residual)\n";
	std::cout << "--------------------------------------------------------------------\n";
	{
		double a = 1.0, b = 3.0;
		double q = a / b;                 // rounded quotient ~0.3333...
		double r = std::fma(-q, b, a);    // exact residual a - q*b
		ReportValue(a, "a =", 25, 17);
		ReportValue(b, "b =", 25, 17);
		ReportValue(q, "q = fl(a/b) =", 25, 17);
		ReportValue(r, "r = a - q*b (exact) =", 25, 17);
		ReportValue(a - q * b, "naive a - q*b in double =", 25, 17);  // lossy; often 0
		double improved = q + r / b;      // one Newton-like correction step
		ReportValue(improved, "q + r/b (refined) =", 25, 17);
		std::cout << "    => r is the exact rounding error of the division (|r| < ulp).\n";
		std::cout << "    => fma recovers r exactly; the naive a - q*b cannot.\n\n";
	}

	// ---------------------------------------------------------------------
	// Composition: EFTs build multi-component expansions that carry the bits
	// a single double would drop.
	// ---------------------------------------------------------------------
	std::cout << "Composition into expansions\n";
	std::cout << "---------------------------\n";
	{
		// grow_expansion: fold a scalar into an expansion, keeping the roundoff.
		std::vector<double> e = { 3.0, 5.0e-16 };
		print_expansion("e =", e);
		std::vector<double> h = grow_expansion(e, 1.0);
		print_expansion("grow(e, 1.0) =", renormalize_expansion(h));
		std::cout << '\n';

		// linear_expansion_sum: add two expansions exactly, then renormalize.
		std::vector<double> x = { 1.0e16, 1.0 };
		std::vector<double> y = { 1.0e16, 3.0 };
		print_expansion("x =", x);
		print_expansion("y =", y);
		std::vector<double> z = renormalize_expansion(linear_expansion_sum(x, y));
		print_expansion("x + y =", z);
		std::cout << std::setprecision(17) << std::scientific;
		std::cout << "    estimate(x+y) = " << estimate(z)
		          << "   (carries the +4 a single double would lose)\n\n";

		// scale_expansion: expansion * scalar, exact via two_prod per limb.
		std::vector<double> s = scale_expansion(x, 3.0);
		print_expansion("x * 3.0 =", renormalize_expansion(s));
		std::cout << '\n';

		// invariants the expansion algorithms maintain: components are ordered by
		// decreasing magnitude and are non-overlapping. Non-overlap means the
		// binary exponent gap between adjacent limbs exceeds the 52-bit mantissa
		// width, so the limbs share no significand bits. (The canonical
		// structural check lives in the verification layer as check_priest_normal.)
		std::vector<double> ok  = { 2.0, std::ldexp(1.0, -60), std::ldexp(1.0, -120) };
		std::vector<double> bad = { 10.0, 0.1, 1.0 };  // not decreasing in magnitude
		print_expansion("well-formed:", ok);
		std::cout << "      is_decreasing_magnitude: " << (is_decreasing_magnitude(ok) ? "yes" : "no") << '\n';
		std::cout << "      adjacent exponent gaps (need > 52): ";
		for (size_t i = 1; i < ok.size(); ++i) std::cout << (std::ilogb(ok[i-1]) - std::ilogb(ok[i])) << ' ';
		std::cout << " -> non-overlapping\n";
		print_expansion("ill-formed:", bad);
		std::cout << "      is_decreasing_magnitude: " << (is_decreasing_magnitude(bad) ? "yes" : "no") << '\n';
	}

	std::cout << "\nAll API examples completed.\n";
	return EXIT_SUCCESS;
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
