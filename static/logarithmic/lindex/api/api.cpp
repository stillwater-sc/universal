// api.cpp: application programming interface of the level-index type lindex<nbits, lbits, bt> (#1694)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The core type: construction, the special encodings, numeric_limits, negation, the
// fields of an encoding and the text layer.  Conversion against the oracle is in
// conversion/conversion.cpp, ordering in logic/logic.cpp.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>
#include <universal/number/lindex/lindex.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

int check(bool ok, const std::string& what, bool reportTestCases) {
	if (!ok && reportTestCases) std::cerr << "FAIL: " << what << '\n';
	return ok ? 0 : 1;
}

// the special encodings, in every block type: they are bit patterns, so they must not depend on bt
template<unsigned nbits, unsigned lbits, typename bt>
int VerifySpecialEncodings(bool reportTestCases) {
	using L = lindex<nbits, lbits, bt>;
	constexpr std::uint64_t mask = (nbits == 64) ? ~0ull : ((1ull << nbits) - 1ull);
	constexpr std::uint64_t sign = 1ull << (nbits - 1);
	int fails = 0;
	fails += check(L(SpecificValue::zero).raw_bits() == 0ull, type_tag(L()) + " zero", reportTestCases);
	fails += check(L(SpecificValue::minpos).raw_bits() == 1ull, type_tag(L()) + " minpos", reportTestCases);
	fails += check(L(SpecificValue::maxpos).raw_bits() == sign - 1ull, type_tag(L()) + " maxpos", reportTestCases);
	fails += check(L(SpecificValue::minneg).raw_bits() == mask, type_tag(L()) + " minneg", reportTestCases);
	fails += check(L(SpecificValue::maxneg).raw_bits() == (sign | 1ull), type_tag(L()) + " maxneg", reportTestCases);
	fails += check(L(SpecificValue::nar).raw_bits() == sign, type_tag(L()) + " nar", reportTestCases);
	fails += check(L(SpecificValue::infpos).isnar() && L(SpecificValue::qnan).isnar(), type_tag(L()) + " no infinity", reportTestCases);
	// negation is the two's complement; zero and NaR are their own negation
	fails += check(-L(SpecificValue::maxpos) == L(SpecificValue::maxneg), type_tag(L()) + " -maxpos", reportTestCases);
	fails += check(-L(SpecificValue::minpos) == L(SpecificValue::minneg), type_tag(L()) + " -minpos", reportTestCases);
	fails += check((-L(SpecificValue::zero)).iszero() && (-L(SpecificValue::nar)).isnar(), type_tag(L()) + " -0, -nar", reportTestCases);
	fails += check(abs(L(SpecificValue::maxneg)) == L(SpecificValue::maxpos), type_tag(L()) + " abs", reportTestCases);
	// 1.0 is psi = 1: level 1, index 0
	L one(1.0);
	fails += check(one.raw_bits() == (1ull << L::fbits) && one.level() == 1 && one.index_bits() == 0, type_tag(L()) + " 1.0", reportTestCases);
	// stepping saturates at the ends, and NaR stays NaR
	L a(SpecificValue::maxpos); ++a;
	fails += check(a == L(SpecificValue::maxpos), type_tag(L()) + " ++maxpos", reportTestCases);
	a = L(SpecificValue::maxneg); --a;
	fails += check(a == L(SpecificValue::maxneg), type_tag(L()) + " --maxneg", reportTestCases);
	a = L(SpecificValue::nar); ++a;
	fails += check(a.isnar(), type_tag(L()) + " ++nar", reportTestCases);
	a = L(SpecificValue::zero); ++a;
	fails += check(a == L(SpecificValue::minpos), type_tag(L()) + " ++0", reportTestCases);
	--a; --a;
	fails += check(a == L(SpecificValue::minneg), type_tag(L()) + " --minpos", reportTestCases);
	return fails;
}

template<unsigned nbits, unsigned lbits, typename bt>
int VerifyNumericLimits(bool reportTestCases) {
	using L = lindex<nbits, lbits, bt>;
	using limits = std::numeric_limits<L>;
	constexpr int fbits = static_cast<int>(nbits - 1 - lbits);
	int fails = 0;
	const std::string tag = type_tag(L());
	fails += check(limits::is_specialized && limits::is_signed && !limits::is_integer && !limits::is_exact, tag + " kind", reportTestCases);
	fails += check(limits::radix == 2 && limits::digits == fbits + 1, tag + " digits", reportTestCases);
	fails += check(limits::min() == L(SpecificValue::minpos) && limits::denorm_min() == limits::min(), tag + " min", reportTestCases);
	fails += check(limits::max() == L(SpecificValue::maxpos) && limits::lowest() == -limits::max(), tag + " max, lowest", reportTestCases);
	// epsilon is 2^-fbits, which is minpos, and also 2^(1 - digits)
	fails += check(limits::epsilon() == limits::min(), tag + " epsilon", reportTestCases);
	if constexpr (fbits <= 52) {
		fails += check(double(limits::epsilon()) == std::ldexp(1.0, 1 - limits::digits), tag + " epsilon value", reportTestCases);
		fails += check(double(limits::min()) == std::ldexp(1.0, limits::min_exponent - 1), tag + " min_exponent", reportTestCases);
	}
	fails += check(limits::max_exponent == std::numeric_limits<int>::max(), tag + " max_exponent saturates", reportTestCases);
	fails += check(!limits::has_infinity && limits::has_quiet_NaN && !limits::has_signaling_NaN, tag + " nan/inf", reportTestCases);
	fails += check(limits::quiet_NaN().isnar() && limits::infinity().isnar(), tag + " NaR", reportTestCases);
	fails += check(limits::has_denorm == std::denorm_absent && !limits::is_iec559 && limits::is_bounded && !limits::is_modulo, tag + " properties", reportTestCases);
	fails += check(limits::round_style == std::round_to_nearest, tag + " round_style", reportTestCases);
	return fails;
}

// a lindex<16,3> value prints as its double, and as phi(psi) past double
int VerifyText(bool reportTestCases) {
	using L = lindex<16, 3>;
	int fails = 0;
	auto str = [](const L& v) { std::stringstream s; s << std::setprecision(15) << v; return s.str(); };
	fails += check(str(L(1.0)) == "1", "print 1.0: " + str(L(1.0)), reportTestCases);
	fails += check(str(L(SpecificValue::nar)) == "nar", "print nar", reportTestCases);
	fails += check(str(L(SpecificValue::maxpos)) == "phi(7.999755859375)", "print maxpos: " + str(L(SpecificValue::maxpos)), reportTestCases);
	fails += check(str(L(SpecificValue::maxneg)) == "-phi(7.999755859375)", "print maxneg: " + str(L(SpecificValue::maxneg)), reportTestCases);
	fails += check(to_binary(L(1.0)) == "0b0.001.000000000000", "to_binary 1.0: " + to_binary(L(1.0)), reportTestCases);
	fails += check(to_binary(L(SpecificValue::nar), true) == "0b1.000.0000'0000'0000", "to_binary nar", reportTestCases);
	fails += check(to_hex(L(SpecificValue::maxpos)) == "0x7FFF", "to_hex maxpos: " + to_hex(L(SpecificValue::maxpos)), reportTestCases);
	fails += check(type_tag(L()) == "lindex< 16, 3, uint8_t>", "type_tag: " + type_tag(L()), reportTestCases);
	std::stringstream in("2.5 junk");
	L r;
	in >> r;
	fails += check(r == L(2.5), "operator>>", reportTestCases);
	r = L(7.0);
	in >> r;   // a failed extraction leaves the target alone
	fails += check(r == L(7.0), "failed operator>>", reportTestCases);
	return fails;
}

// the fields: psi(|v|) = level + index, and scale = floor(log2 |v|)
int VerifyFields(bool reportTestCases) {
	using L = lindex<32, 3>;
	int fails = 0;
	L half(0.5);
	fails += check(half.level() == 0 && half.index() == 0.5 && half.psi() == 0.5 && half.scale() == -1, "fields of 0.5", reportTestCases);
	L m(-1.0);
	fails += check(m.level() == 1 && m.index() == 0.0 && m.isneg() && m.scale() == 0, "fields of -1.0", reportTestCases);
	// 2^100 is past phi(4) = 3.8e6, so level 4; the encoding rounds psi, so its value sits
	// on either side of 2^100
	L big(std::ldexp(1.0, 100));
	fails += check(big.level() == 4 && (big.scale() == 99 || big.scale() == 100), "fields of 2^100, scale " + std::to_string(big.scale()), reportTestCases);
	// minpos is 2^-28
	fails += check(L(SpecificValue::minpos).scale() == -28, "scale of minpos", reportTestCases);
	// level 7 is past 2^(2^63)
	fails += check(L(SpecificValue::maxpos).scale() == std::numeric_limits<int64_t>::max(), "scale of maxpos saturates", reportTestCases);
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

	std::string test_suite  = "lindex API (#1694)";
	std::string test_tag    = "api";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// important behavioral traits
	ReportTrivialityOfType<lindex<8, 3>>();
	ReportTrivialityOfType<lindex<16, 3>>();
	ReportTrivialityOfType<lindex<64, 3, std::uint64_t>>();
	static_assert(is_lindex<lindex<32, 3>> && !is_lindex<double>, "is_lindex");

	{
		using L = lindex<16, 3>;
		std::cout << lindex_range(L()) << '\n';
		for (double v : { 0.0, 0.5, 1.0, 2.718281828459045, 1.0e6, 1.0e300, -3.0 }) {
			L a(v);
			std::cout << std::setw(14) << v << " : " << to_binary(a, true) << " : " << color_print(a, true) << " : " << info_print(a, 10) << '\n';
		}
	}

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySpecialEncodings<8, 3, std::uint8_t>(reportTestCases), "lindex< 8,3>", "special encodings");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialEncodings<16, 3, std::uint8_t>(reportTestCases), "lindex<16,3>", "special encodings");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialEncodings<16, 3, std::uint16_t>(reportTestCases), "lindex<16,3,uint16_t>", "special encodings");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialEncodings<32, 3, std::uint32_t>(reportTestCases), "lindex<32,3,uint32_t>", "special encodings");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialEncodings<64, 3, std::uint8_t>(reportTestCases), "lindex<64,3>", "special encodings");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialEncodings<64, 3, std::uint64_t>(reportTestCases), "lindex<64,3,uint64_t>", "special encodings");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialEncodings<12, 2, std::uint16_t>(reportTestCases), "lindex<12,2,uint16_t>", "special encodings");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialEncodings<5, 1, std::uint8_t>(reportTestCases), "lindex< 5,1>", "special encodings");

	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<8, 3, std::uint8_t>(reportTestCases), "lindex< 8,3>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<16, 3, std::uint8_t>(reportTestCases), "lindex<16,3>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<32, 3, std::uint32_t>(reportTestCases), "lindex<32,3>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<64, 3, std::uint64_t>(reportTestCases), "lindex<64,3>", "numeric_limits");

	nrOfFailedTestCases += ReportTestResult(VerifyText(reportTestCases), "lindex<16,3>", "text");
	nrOfFailedTestCases += ReportTestResult(VerifyFields(reportTestCases), "lindex<32,3>", "fields");
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
