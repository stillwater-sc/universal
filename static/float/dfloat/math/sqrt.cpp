// sqrt.cpp: dfloat sqrt is correctly rounded in decimal (#1725)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// dfloat sqrt was dfloat(std::sqrt(double(a))): decimal to binary, a binary root, binary back
// to decimal.  A double holds about 15.9 decimal digits, so decimal64 and decimal128 lost their
// trailing digits and decimal32 was rounded twice.
//
// The oracle shares nothing with dfloat arithmetic: every value is unpacked into an integer
// significand times a power of ten, and r is the correctly rounded root of a exactly when a
// lies between the squares of the midpoints to r's neighbours, compared in integer<1024>.  A
// tie cannot occur: a midpoint has ndigits + 1 digits ending in 5, so its square ends in 25
// and has more significant digits than any argument.
#include <universal/utility/directives.hpp>
#include <cerrno>
#include <cstdint>
#include <random>
#include <string>

#include <universal/number/dfloat/dfloat.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
using Big = integer<1024, std::uint32_t>;

struct Decimal { Big N; int e; };           // N 10^e

template<typename Dfloat>
Decimal decompose(const Dfloat& v) {
	bool s{ false };
	int e{ 0 };
	typename Dfloat::significand_t sig;
	v.unpack(s, e, sig);
	Big n(0);
	for (unsigned i = Dfloat::sig_bits; i-- > 0;) {
		n <<= 1;
		if (sig.at(i)) n += Big(1);
	}
	return { n, e };
}

inline Big pow10(int k) {
	Big p(1);
	for (int i = 0; i < k; ++i) p *= Big(10);
	return p;
}

// sign of (2 m)^2 - 4 a, for m the midpoint of u and v, all exact
inline int compare_midpoint_square(const Decimal& u, const Decimal& v, const Decimal& a) {
	const int e0 = (u.e < v.e) ? u.e : v.e;
	const Big twice_m = u.N * pow10(u.e - e0) + v.N * pow10(v.e - e0);   // 2m 10^-e0
	Big lhs = twice_m * twice_m, rhs = Big(4) * a.N;                      // (2m)^2 10^-2e0, 4a 10^-qa
	const int k = 2 * e0 - a.e;
	if (k >= 0) lhs *= pow10(k); else rhs *= pow10(-k);
	return (lhs > rhs) ? 1 : ((lhs < rhs) ? -1 : 0);
}

template<typename Dfloat>
bool is_correctly_rounded(const Dfloat& a, const Dfloat& r) {
	if (r.isnan() || r.isinf() || r.iszero() || r.isneg()) return false;
	Dfloat down(r), up(r);
	--down;
	++up;
	const Decimal A = decompose(a), R = decompose(r);
	if (!down.iszero() && compare_midpoint_square(decompose(down), R, A) > 0) return false;
	if (down.iszero() && compare_midpoint_square(Decimal{ Big(0), R.e }, R, A) > 0) return false;
	if (up.isinf() || up.isnan()) return true;
	return compare_midpoint_square(R, decompose(up), A) >= 0;
}

template<typename Dfloat>
int VerifySqrtExhaustive(bool reportTestCases) {
	static_assert(Dfloat::nbits <= 20, "exhaustive only for narrow formats");
	int fails = 0, checked = 0;
	for (unsigned long long bits = 0; bits < (1ull << Dfloat::nbits); ++bits) {
		Dfloat a;
		a.setbits(bits);
		if (a.isnan() || a.isinf() || a.iszero() || a.isneg()) continue;
		if (decompose(a).N.iszero()) continue;                 // a zero-valued (non-canonical) encoding
		++checked;
		const Dfloat r = sqrt(a);
		if (!is_correctly_rounded(a, r)) {
			if (reportTestCases && fails < 4) std::cerr << "FAIL: " << type_tag(a) << " sqrt(" << a << ") = " << r << '\n';
			++fails;
		}
	}
	return checked > 0 ? fails : 1;
}

// random ndigits-digit significands over the whole exponent range, through the decimal parser
template<typename Dfloat>
int VerifySqrtSampled(bool reportTestCases, unsigned samples) {
	constexpr unsigned ndigits = Dfloat::ndigits;
	std::mt19937_64 rng(1725);
	std::uniform_int_distribution<int> digit(0, 9), lead(1, 9);
	std::uniform_int_distribution<int> exponent(Dfloat::emin + static_cast<int>(ndigits), Dfloat::emax);
	int fails = 0;
	for (unsigned i = 0; i < samples; ++i) {
		std::string s(1, static_cast<char>('0' + lead(rng)));
		for (unsigned d = 1; d < ndigits; ++d) s += static_cast<char>('0' + digit(rng));
		s += "e" + std::to_string(exponent(rng));
		const Dfloat a(s);
		if (a.isnan() || a.isinf() || a.iszero()) continue;
		const Dfloat r = sqrt(a);
		if (!is_correctly_rounded(a, r)) {
			if (reportTestCases && fails < 4) std::cerr << "FAIL: " << type_tag(a) << " sqrt(" << s << ") = " << r << '\n';
			++fails;
		}
	}
	return fails;
}

template<typename Dfloat>
int VerifySpecialValues(bool reportTestCases) {
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << type_tag(Dfloat()) << ' ' << what << '\n'; }
	};
	check(sqrt(Dfloat(0)).iszero(), "sqrt(0) is not 0");
	check(sqrt(Dfloat(SpecificValue::infpos)).isinf(), "sqrt(inf) is not inf");
	check(sqrt(Dfloat(SpecificValue::qnan)).isnan(), "sqrt(NaN) is not NaN");
#if DFLOAT_THROW_ARITHMETIC_EXCEPTION
	bool thrown = false;
	try { (void)sqrt(Dfloat(-4)); } catch (const dfloat_negative_sqrt_arg&) { thrown = true; }
	check(thrown, "sqrt(-4) did not throw dfloat_negative_sqrt_arg");
#else
	errno = 0;
	check(sqrt(Dfloat(-4)).isnan() && errno == EDOM, "sqrt(-4) is not NaN with EDOM");
#endif
	// an exact root has the preferred exponent floor(q/2): the encoding of 12 itself
	check(to_binary(sqrt(Dfloat(144))) == to_binary(Dfloat(12)), "sqrt(144) is not 12 at exponent 0");
	check(to_binary(sqrt(Dfloat("1.44"))) == to_binary(Dfloat("1.2")), "sqrt(1.44) is not 1.2 at exponent -1");
	return fails;
}

}  // anonymous namespace

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

	std::string test_suite  = "dfloat sqrt correct rounding (#1725)";
	std::string test_tag    = "sqrt";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifySqrtSampled<decimal64>(reportTestCases, 1000), "decimal64", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult((VerifySqrtExhaustive<dfloat<3, 2, DecimalEncoding::BID, std::uint16_t>>(reportTestCases)), "dfloat<3,2> BID", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrtExhaustive<dfloat<3, 2, DecimalEncoding::DPD, std::uint16_t>>(reportTestCases)), "dfloat<3,2> DPD", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtSampled<decimal32>(reportTestCases, 5000), "decimal32", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtSampled<decimal64>(reportTestCases, 3000), "decimal64", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtSampled<decimal64_dpd>(reportTestCases, 1000), "decimal64 DPD", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtSampled<decimal128>(reportTestCases, 500), "decimal128", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<decimal32>(reportTestCases), "decimal32 special values", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<decimal64>(reportTestCases), "decimal64 special values", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySqrtSampled<decimal64>(reportTestCases, 30000), "decimal64", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtSampled<decimal128>(reportTestCases, 5000), "decimal128", test_tag);
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
