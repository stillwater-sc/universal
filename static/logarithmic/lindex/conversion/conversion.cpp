// conversion.cpp: lindex conversion to and from native types against the mpmath oracle (#1694)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The expected encodings and values come from tools/generators/lindex_conversion_gen.py,
// which evaluates psi and phi at 600 bits and rounds once.  Every comparison is for
// equality: an encoding against the correctly rounded encoding, a double or float against
// the correctly rounded double or float.  There are no round trips, which would pass a
// conversion that is wrong in both directions.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <universal/number/lindex/lindex.hpp>
#include <universal/verification/lindex_conversion_vectors.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
namespace lv = sw::universal::lindex_verification;

// the column of lindex<nbits, lbits> in the enc[] arrays
template<unsigned nbits, unsigned lbits>
constexpr unsigned column() {
	for (unsigned c = 0; c < lv::nr_configs; ++c) {
		if (lv::lindex_configs[c].nbits == nbits && lv::lindex_configs[c].lbits == lbits) return c;
	}
	return lv::nr_configs;
}

template<typename Real>
bool same_bits(Real a, Real b) { return std::memcmp(&a, &b, sizeof(Real)) == 0; }

template<typename L>
void report_encoding(bool reportTestCases, const char* what, double x, const L& got, std::uint64_t expected) {
	if (!reportTestCases) return;
	std::cerr << "FAIL: " << type_tag(got) << ' ' << what << ' ' << std::hexfloat << x << std::defaultfloat
	          << " -> " << to_hex(got) << ", expected 0x" << std::hex << expected << std::dec << '\n';
}

template<unsigned nbits, unsigned lbits, typename bt = std::uint8_t>
int VerifyFromNative(bool reportTestCases) {
	using L = lindex<nbits, lbits, bt>;
	constexpr unsigned col = column<nbits, lbits>();
	static_assert(col < lv::nr_configs, "configuration is not in the vectors");
	int fails = 0;
	for (const auto& v : lv::from_double) {
		L a(v.x);
		if (a.raw_bits() != v.enc[col]) { ++fails; report_encoding(reportTestCases, "from double", v.x, a, v.enc[col]); }
		// a value that is a float converts the same from float
		const float f = static_cast<float>(v.x);
		if (static_cast<double>(f) == v.x) {
			L b(f);
			if (b.raw_bits() != v.enc[col]) { ++fails; report_encoding(reportTestCases, "from float", v.x, b, v.enc[col]); }
		}
	}
	for (const auto& v : lv::from_signed) {
		L a(static_cast<long long>(v.x));
		if (a.raw_bits() != v.enc[col]) { ++fails; report_encoding(reportTestCases, "from long long", double(v.x), a, v.enc[col]); }
		// the narrower signed types take the same route
		if (v.x >= std::numeric_limits<int>::min() && v.x <= std::numeric_limits<int>::max()) {
			L b(static_cast<int>(v.x));
			if (b.raw_bits() != v.enc[col]) { ++fails; report_encoding(reportTestCases, "from int", double(v.x), b, v.enc[col]); }
		}
		if (v.x >= -128 && v.x <= 127) {
			L c(static_cast<signed char>(v.x));
			if (c.raw_bits() != v.enc[col]) { ++fails; report_encoding(reportTestCases, "from signed char", double(v.x), c, v.enc[col]); }
		}
	}
	for (const auto& v : lv::from_unsigned) {
		L a(static_cast<unsigned long long>(v.x));
		if (a.raw_bits() != v.enc[col]) { ++fails; report_encoding(reportTestCases, "from unsigned long long", double(v.x), a, v.enc[col]); }
	}
	return fails;
}

template<unsigned nbits, unsigned lbits, std::size_t N, typename bt = std::uint8_t>
int VerifyToNative(const lv::ToNativeVector (&vectors)[N], bool reportTestCases) {
	using L = lindex<nbits, lbits, bt>;
	int fails = 0;
	for (const auto& v : vectors) {
		L a;
		a.setbits(v.enc);
		const double d = double(a);
		const float f = float(a);
		if (!same_bits(d, v.d) || !same_bits(f, v.f)) {
			++fails;
			if (reportTestCases) {
				std::cerr << "FAIL: " << type_tag(a) << ' ' << to_hex(a) << " -> " << std::hexfloat << d << ", " << f
				          << ", expected " << v.d << ", " << v.f << std::defaultfloat << '\n';
			}
		}
	}
	return fails;
}

// NaN, infinities, signed zero, and the integer and NaR edges of conversion to native
template<unsigned nbits, unsigned lbits>
int VerifySpecialCases(bool reportTestCases) {
	using L = lindex<nbits, lbits>;
	int fails = 0;
	auto check = [&](bool ok, const std::string& what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << type_tag(L()) << ' ' << what << '\n'; }
	};
	check(L(std::numeric_limits<double>::quiet_NaN()).isnar(), "NaN -> NaR");
	check(L(std::numeric_limits<double>::infinity()).isnar(), "+inf -> NaR");
	check(L(-std::numeric_limits<double>::infinity()).isnar(), "-inf -> NaR");
	check(L(std::numeric_limits<float>::infinity()).isnar(), "float inf -> NaR");
	check(L(-0.0).raw_bits() == 0ull, "-0.0 -> 0");
	check(std::isnan(double(L(SpecificValue::nar))) && std::isnan(float(L(SpecificValue::nar))), "NaR -> NaN");
	check(double(L(SpecificValue::zero)) == 0.0 && !std::signbit(double(L(SpecificValue::zero))), "0 -> +0.0");
	if constexpr ((1u << lbits) > 5u) {
		// the type has no infinity, but maxpos is past double: +-inf is the honest IEEE answer
		check(double(L(SpecificValue::maxpos)) == std::numeric_limits<double>::infinity(), "maxpos -> +inf");
		check(double(L(SpecificValue::maxneg)) == -std::numeric_limits<double>::infinity(), "maxneg -> -inf");
		// integers saturate
		check(int(L(SpecificValue::maxpos)) == std::numeric_limits<int>::max(), "int(maxpos)");
		check(int(L(SpecificValue::maxneg)) == std::numeric_limits<int>::min(), "int(maxneg)");
		check((long long)(L(SpecificValue::maxpos)) == std::numeric_limits<long long>::max(), "long long(maxpos)");
		check((unsigned long long)(L(SpecificValue::maxpos)) == std::numeric_limits<unsigned long long>::max(), "unsigned long long(maxpos)");
	}
	else {
		// lbits = 2 tops out at level 3: maxpos of lindex<16,2> is phi(4 - 2^-13) = 3795151.4
		check(int(L(SpecificValue::maxpos)) == 3795151, "int(maxpos)");
	}
	// integers truncate toward zero, and NaR gives the most negative integer
	check(int(L(SpecificValue::nar)) == std::numeric_limits<int>::min(), "int(nar)");
	check((unsigned int)(L(-5.0)) == 0u, "unsigned(-5)");
	check(int(L(0.75)) == 0 && int(L(-0.75)) == 0, "int(+-0.75) truncates");
	return fails;
}

#if LONG_DOUBLE_SUPPORT
// long double: a value a double holds converts the same as that double, and a value past
// DBL_MAX lands between psi(DBL_MAX) and the next level rather than on NaR
template<unsigned nbits, unsigned lbits>
int VerifyLongDouble(bool reportTestCases) {
	using L = lindex<nbits, lbits>;
	int fails = 0;
	for (const auto& v : lv::from_double) {
		L a(static_cast<long double>(v.x)), b(v.x);
		if (a != b) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << type_tag(a) << " from long double " << v.x << '\n'; }
	}
	if constexpr (std::numeric_limits<long double>::max_exponent > std::numeric_limits<double>::max_exponent) {
		const long double huge = static_cast<long double>(std::numeric_limits<double>::max()) * 1.0e10l;
		L c(huge), d(std::numeric_limits<double>::max());
		if (!(c > d) || c.level() != 4) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << type_tag(c) << " from long double past DBL_MAX\n"; }
		// and back: the value is finite in long double
		const long double back = static_cast<long double>(c);
		if (!(back > static_cast<long double>(std::numeric_limits<double>::max())) || std::isinf(back)) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL: " << type_tag(c) << " to long double past DBL_MAX\n";
		}
	}
	return fails;
}
#endif

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

	std::string test_suite  = "lindex conversion against the mpmath oracle (#1694)";
	std::string test_tag    = "conversion";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyFromNative<16, 3>(reportTestCases), "lindex<16,3>", "from native");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyFromNative< 8, 3>(reportTestCases), "lindex< 8,3>", "from native");
	nrOfFailedTestCases += ReportTestResult(VerifyFromNative<16, 3>(reportTestCases), "lindex<16,3>", "from native");
	nrOfFailedTestCases += ReportTestResult(VerifyFromNative<32, 3>(reportTestCases), "lindex<32,3>", "from native");
	nrOfFailedTestCases += ReportTestResult(VerifyFromNative<64, 3>(reportTestCases), "lindex<64,3>", "from native");
	nrOfFailedTestCases += ReportTestResult(VerifyFromNative<16, 2>(reportTestCases), "lindex<16,2>", "from native");
	nrOfFailedTestCases += ReportTestResult(VerifyFromNative<16, 4>(reportTestCases), "lindex<16,4>", "from native");
	// the encoding does not depend on the block type
	nrOfFailedTestCases += ReportTestResult((VerifyFromNative<32, 3, std::uint32_t>(reportTestCases)), "lindex<32,3,uint32_t>", "from native");
	nrOfFailedTestCases += ReportTestResult((VerifyFromNative<64, 3, std::uint64_t>(reportTestCases)), "lindex<64,3,uint64_t>", "from native");

	nrOfFailedTestCases += ReportTestResult(VerifyToNative< 8, 3>(lv::to_native_8_3, reportTestCases), "lindex< 8,3>", "to native");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<16, 3>(lv::to_native_16_3, reportTestCases), "lindex<16,3>", "to native");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<32, 3>(lv::to_native_32_3, reportTestCases), "lindex<32,3>", "to native");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<64, 3>(lv::to_native_64_3, reportTestCases), "lindex<64,3>", "to native");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<16, 2>(lv::to_native_16_2, reportTestCases), "lindex<16,2>", "to native");
	nrOfFailedTestCases += ReportTestResult(VerifyToNative<16, 4>(lv::to_native_16_4, reportTestCases), "lindex<16,4>", "to native");
	nrOfFailedTestCases += ReportTestResult((VerifyToNative<64, 3, std::size(lv::to_native_64_3), std::uint64_t>(lv::to_native_64_3, reportTestCases)), "lindex<64,3,uint64_t>", "to native");

	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<16, 3>(reportTestCases), "lindex<16,3>", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<64, 3>(reportTestCases), "lindex<64,3>", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<16, 2>(reportTestCases), "lindex<16,2>", "special cases");
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyLongDouble<32, 3>(reportTestCases), "lindex<32,3>", "long double");
	nrOfFailedTestCases += ReportTestResult(VerifyLongDouble<64, 3>(reportTestCases), "lindex<64,3>", "long double");
#endif
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
