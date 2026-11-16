// overflow.cpp: a sum that outgrows the quire's accumulator resolves to +-maxpos or +-inf, it does not wrap
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The quire dropped the carry out of the top bit of its accumulator and kept no overflow state, so a sum
// past the accumulator wrapped: quire<cfloat<4,2>, 0> of 3*3 + 3*3 resolved to 2, and quire<posit<8,0>, 0>
// of four maxpos^2 to 0 (#2094). As decided, an overflowed quire resolves to +-maxpos in a type without
// infinities (posit, bposit, lns, dbns, fixpnt) and to +-inf in one with them (cfloat, float, double).
// The overflow is sticky. Capacity 0 makes the accumulator hold just the product range, so a few
// products reach its top.
#include <universal/utility/directives.hpp>
#define QUIRE_THROW_ARITHMETIC_EXCEPTION 0
#include <cmath>
#include <limits>
#include <string>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/bposit/bposit.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/dbns/fdp.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/native/fdp.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// Products of a power of two v = 2^k with itself, into a capacity-0 quire, until the sum passes the
// accumulator's top: k is the largest scale whose square the accumulator holds as one operand (an operand
// past it is skipped, the #1226 policy, not accumulated), and 2^(h - 2k + 1) copies reach 2^(h + 1).
template<typename Scalar>
sw::universal::quire<Scalar, 0> accumulate_past_top(bool negative, int& copies) {
	using namespace sw::universal;
	quire<Scalar, 0> q;
	const int h = q.max_scale();
	int k = h / 2;
	while (k > 0 && (double(Scalar(std::ldexp(1.0, k))) != std::ldexp(1.0, k))) --k;   // 2^k must be exact
	const Scalar v(std::ldexp(1.0, k));
	copies = 1 << (h - 2 * k + 1);
	for (int i = 0; i < copies; ++i) q += quire_mul(negative ? -v : v, v);
	return q;
}

// a type without infinities: an overflowed sum resolves to +-maxpos
template<typename Scalar>
int VerifySaturates(const std::string& tag, bool reportTestCases) {
	using namespace sw::universal;
	int fails = 0;
	for (bool negative : { false, true }) {
		int copies = 0;
		const auto q = accumulate_past_top<Scalar>(negative, copies);
		const Scalar expected(negative ? SpecificValue::maxneg : SpecificValue::maxpos);
		const Scalar got = quire_resolve(q);
		if (!q.isoverflow() || !(got == expected)) {
			if (reportTestCases) std::cerr << "FAIL: " << tag << (negative ? " -" : " +") << copies << " squares: overflow " << q.isoverflow() << ", resolved " << got << '\n';
			++fails;
		}
	}
	return fails;
}

// dbns's quire is sized generously (its top bit weighs 2^127 against a maxpos near 2^24), so no
// count of products reaches it: the value 2^max_scale is assigned, and the quire added to itself
template<typename Scalar>
int VerifySaturatesByQuireSum(const std::string& tag, bool reportTestCases) {
	using namespace sw::universal;
	int fails = 0;
	for (bool negative : { false, true }) {
		quire<Scalar, 0> q;
		q = std::ldexp(negative ? -1.0 : 1.0, q.max_scale());
		const quire<Scalar, 0> same(q);
		q += same;
		const Scalar expected(negative ? SpecificValue::maxneg : SpecificValue::maxpos);
		const Scalar got = quire_resolve(q);
		if (!q.isoverflow() || !(got == expected)) {
			if (reportTestCases) std::cerr << "FAIL: " << tag << (negative ? " -" : " +") << "2^top + 2^top: overflow " << q.isoverflow() << ", resolved " << got << '\n';
			++fails;
		}
	}
	return fails;
}

// cfloat: an overflowed sum resolves to +-inf, and a finite term of the other sign does not bring it back
int VerifyCfloat(bool reportTestCases) {
	using namespace sw::universal;
	using C = cfloat<4, 2, uint8_t, true, false, false>;
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) {
			if (reportTestCases) std::cerr << "FAIL: cfloat<4,2> " << what << '\n';
			++fails;
		}
	};
	quire<C, 0> q;
	q += quire_mul(C(3), C(3));
	check(!q.isoverflow() && quire_resolve(q).isinf(), "one 3*3 is finite in the quire, inf in the cfloat");
	q += quire_mul(C(3), C(3));
	check(q.isoverflow() && quire_resolve(q).isinf() && !quire_resolve(q).sign(), "3*3 + 3*3 overflows to +inf (was 2)");
	q += quire_mul(C(-3), C(3));
	check(q.isoverflow() && quire_resolve(q).isinf(), "the overflow is sticky");
	quire<C, 0> n;
	n += quire_mul(C(-3), C(3));
	n += quire_mul(C(-3), C(3));
	check(n.isoverflow() && quire_resolve(n).isinf() && quire_resolve(n).sign(), "-(3*3) - 3*3 overflows to -inf");
	// quire + quire carries too, and an overflowed quire orders past every finite one
	quire<C, 0> a, b;
	a += quire_mul(C(3), C(3));
	b += quire_mul(C(3), C(3));
	a += b;
	check(a.isoverflow(), "quire + quire overflows");
	quire<C, 0> small;
	small += quire_mul(C(1), C(1));
	check(small < a && a > small && !(a == small) && n < small, "an overflowed quire orders past the finite ones");
	check(std::isinf(a.convert_to<double>()) && a.convert_to<int>() == std::numeric_limits<int>::max(), "convert_to saturates or is inf");
	// opposite overflows cannot be ordered: their sum is NaN
	quire<C, 0> pn(a);
	pn += n;
	check(pn.isnan(), "+overflow + -overflow is NaN");
	// a fresh assignment clears the state
	a = quire_mul(C(1), C(1));
	check(!a.isoverflow() && quire_resolve(a) == C(1), "assignment clears the overflow");
	return fails;
}

// double: the native quire resolves an overflow to +-inf
int VerifyDouble(bool reportTestCases) {
	using namespace sw::universal;
	int fails = 0;
	quire<double, 0> q;
	const double m = std::numeric_limits<double>::max();
	for (int i = 0; i < 4; ++i) q += quire_mul(m, m);
	if (!q.isoverflow() || !std::isinf(quire_resolve(q)) || quire_resolve(q) < 0) {
		if (reportTestCases) std::cerr << "FAIL: quire<double, 0> 4 max^2: overflow " << q.isoverflow() << ", resolved " << quire_resolve(q) << '\n';
		++fails;
	}
	return fails;
}

}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "quire overflow";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using C = cfloat<4, 2, uint8_t, true, false, false>;
	quire<C, 0> q;
	q += quire_mul(C(3), C(3));
	q += quire_mul(C(3), C(3));
	std::cout << "3*3 + 3*3 in quire<cfloat<4,2>, 0>: " << quire_resolve(q) << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyCfloat(reportTestCases), "cfloat<4,2>", "overflow to inf");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble(reportTestCases), "double",      "overflow to inf");
	nrOfFailedTestCases += ReportTestResult(VerifySaturates<posit<8, 0>>("posit<8,0>", reportTestCases),   "posit<8,0>",   "overflow to maxpos");
	nrOfFailedTestCases += ReportTestResult(VerifySaturates<posit<16, 1>>("posit<16,1>", reportTestCases), "posit<16,1>",  "overflow to maxpos");
	nrOfFailedTestCases += ReportTestResult(VerifySaturates<bposit<16, 5, 2>>("bposit<16,5,2>", reportTestCases), "bposit<16,5,2>", "overflow to maxpos");
	nrOfFailedTestCases += ReportTestResult(VerifySaturates<lns<8, 4>>("lns<8,4>", reportTestCases),        "lns<8,4>",     "overflow to maxpos");
	nrOfFailedTestCases += ReportTestResult(VerifySaturatesByQuireSum<dbns<8, 3>>("dbns<8,3>", reportTestCases), "dbns<8,3>", "overflow to maxpos");
	nrOfFailedTestCases += ReportTestResult(VerifySaturatesByQuireSum<posit<8, 0>>("posit<8,0>", reportTestCases), "posit<8,0>", "quire + quire");
	nrOfFailedTestCases += ReportTestResult(VerifySaturates<fixpnt<8, 4, Saturate, uint8_t>>("fixpnt<8,4>", reportTestCases), "fixpnt<8,4>", "overflow to maxpos");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
