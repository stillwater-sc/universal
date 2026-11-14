// cordic_standalone.cpp: the fixpnt CORDIC engine through its own header alone, built without the
// division-by-zero exception (#2072)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Two things the other CORDIC tests cannot reach:
//
// - The header stands alone. universal/number/fixpnt/math/cordic.hpp is the only fixpnt header this file
//   includes, so a header that relies on fixpnt.hpp or mathlib.hpp having been included first fails to
//   compile here. Every engine function is instantiated, for both arithmetic behaviours.
// - The non-throwing branches. With FIXPNT_THROW_DIVIDE_BY_ZERO 0, cordic::div(y, 0) gives maxpos or
//   maxneg by the sign of y (0 counts as positive) under Saturate and 0 under Modulo, as fixpnt's
//   operator/ does; and with
//   FIXPNT_THROW_ARITHMETIC_EXCEPTION 0, cordic::sqrt of a negative sets errno = EDOM and gives 0.
// - The final rounding on exact ties. The accuracy tests bound the error, and a tie is 1/2 ulp away
//   whichever way it rounds, so they cannot tell ties to even from ties up.
//
// The accuracy of each function is measured in cordic.cpp, cordic_linear.cpp and cordic_hyperbolic.cpp.
#include <universal/utility/directives.hpp>
#define FIXPNT_THROW_DIVIDE_BY_ZERO 0
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/fixpnt/math/cordic.hpp>   // the only fixpnt header: it must stand alone
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	template<typename Fixed>
	std::string tag_of() {
		return std::string("fixpnt<") + std::to_string(Fixed::nbits) + ',' + std::to_string(Fixed::rbits) + ','
		       + (Fixed::arithmetic == Saturate ? "sat" : "mod") + '>';
	}

	// every engine function on arguments whose results are exact in the type
	template<typename Fixed>
	int VerifyExactResults(bool reportTestCases) {
		int fails = 0;
		auto expect = [&](const char* what, const Fixed& got, double want) {
			if (double(got) != want) {
				if (reportTestCases) std::cerr << "FAIL: " << tag_of<Fixed>() << ' ' << what << " = " << double(got) << ", expected " << want << '\n';
				++fails;
			}
		};
		const Fixed zero(0), one(1), two(2), three(3), four(4), six(6);
		Fixed s, c, r, t;
		cordic::sincos(zero, s, c);
		expect("sincos(0) sin", s, 0.0);
		expect("sincos(0) cos", c, 1.0);
		expect("sin(0)", cordic::sin(zero), 0.0);
		expect("cos(0)", cordic::cos(zero), 1.0);
		expect("tan(0)", cordic::tan(zero), 0.0);
		expect("atan(0)", cordic::atan(zero), 0.0);
		expect("atan2(0, 1)", cordic::atan2(zero, one), 0.0);
		expect("hypot(3, 4)", cordic::hypot(three, four), 5.0);
		cordic::polar(three, four, r, t);
		expect("polar(3, 4) r", r, 5.0);
		expect("mul(2, 3)", cordic::mul(two, three), 6.0);
		expect("div(6, 3)", cordic::div(six, three), 2.0);
		expect("exp(0)", cordic::exp(zero), 1.0);
		cordic::sinhcosh(zero, s, c);
		expect("sinhcosh(0) sinh", s, 0.0);
		expect("sinhcosh(0) cosh", c, 1.0);
		expect("sinh(0)", cordic::sinh(zero), 0.0);
		expect("cosh(0)", cordic::cosh(zero), 1.0);
		expect("tanh(0)", cordic::tanh(zero), 0.0);
		expect("atanh(0)", cordic::atanh(zero), 0.0);
		expect("log(1)", cordic::log(one), 0.0);
		expect("sqrt(4)", cordic::sqrt(four), 2.0);
		return fails;
	}

	// FIXPNT_THROW_DIVIDE_BY_ZERO 0: y / 0 is maxpos or maxneg by the sign of y under Saturate, and 0 under
	// Modulo; FIXPNT_THROW_ARITHMETIC_EXCEPTION 0: sqrt of a negative is 0 with errno = EDOM
	template<typename Fixed>
	int VerifyNonThrowingBranches(bool reportTestCases) {
		int fails = 0;
		auto expect = [&](const char* what, const Fixed& got, const Fixed& want) {
			if (got != want) {
				if (reportTestCases) std::cerr << "FAIL: " << tag_of<Fixed>() << ' ' << what << " = " << double(got) << ", expected " << double(want) << '\n';
				++fails;
			}
		};
		const Fixed zero(0), one(1), minus_one(-1);
		const Fixed maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg), minpos(SpecificValue::minpos);
		if constexpr (Fixed::arithmetic == Saturate) {
			expect("div(1, 0)", cordic::div(one, zero), maxpos);
			expect("div(-1, 0)", cordic::div(minus_one, zero), maxneg);
			expect("div(minpos, 0)", cordic::div(minpos, zero), maxpos);
			expect("div(maxneg, 0)", cordic::div(maxneg, zero), maxneg);
		}
		else {
			expect("div(1, 0)", cordic::div(one, zero), zero);
			expect("div(-1, 0)", cordic::div(minus_one, zero), zero);
		}
		// and it agrees with fixpnt's operator/ for every dividend, zero included
		for (const Fixed& y : { zero, one, minus_one, minpos, maxpos, maxneg }) {
			Fixed q(y);
			q /= zero;
			expect("div(y, 0) == y / 0", cordic::div(y, zero), q);
		}
		errno = 0;
		expect("sqrt(-1)", cordic::sqrt(minus_one), zero);
		if (errno != EDOM) {
			if (reportTestCases) std::cerr << "FAIL: " << tag_of<Fixed>() << " sqrt(-1) errno = " << errno << ", expected EDOM\n";
			++fails;
		}
		errno = 0;
		return fails;
	}

	// the final rounding of every engine result, detail::fit, on exact ties: round to nearest, ties to even,
	// in two's complement. An error bound cannot see this, since a tie is 1/2 ulp away either way.
	template<typename Fixed>
	int VerifyFinalRounding(bool reportTestCases) {
		using Raw = blockbinary<Fixed::nbits + 4u, typename Fixed::BlockType, BinaryNumberType::Signed>;
		int fails = 0;
		struct Case { long long raw; unsigned shift; long long want; };
		// raw * 2^-shift in units of the target's last bit
		constexpr Case cases[] = {
			{  5, 1,  2 }, {  7, 1,  4 }, {  3, 1,  2 }, {  1, 1,  0 },   // 2.5, 3.5, 1.5, 0.5: ties to even
			{ -5, 1, -2 }, { -7, 1, -4 }, { -3, 1, -2 }, { -1, 1,  0 },   // and their negatives
			{ 10, 2,  2 }, { 11, 2,  3 }, {  9, 2,  2 }, { 14, 2,  4 },   // 2.5, 2.75, 2.25, 3.5
			{ -10, 2, -2 }, { -11, 2, -3 }, { -9, 2, -2 }, { -14, 2, -4 },
		};
		for (const Case& k : cases) {
			const Fixed got = cordic::detail::fit<Fixed>(Raw(k.raw), k.shift);
			Fixed want;
			want.setbits(static_cast<std::uint64_t>(k.want < 0 ? -k.want : k.want));   // the magnitude in raw units
			if (k.want < 0) want = -want;
			if (got != want) {
				if (reportTestCases) std::cerr << "FAIL: " << tag_of<Fixed>() << " fit(" << k.raw << " * 2^-" << k.shift << ") = " << double(got)
				                               << ", expected " << double(want) << '\n';
				++fails;
			}
		}
		return fails;
	}

	template<typename Fixed>
	int VerifyStandalone(bool reportTestCases) {
		return VerifyExactResults<Fixed>(reportTestCases) + VerifyNonThrowingBranches<Fixed>(reportTestCases)
		       + VerifyFinalRounding<Fixed>(reportTestCases);
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

	std::string test_suite  = "fixpnt CORDIC engine through its own header (#2072)";
	std::string test_tag    = "cordic standalone";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult((VerifyStandalone<fixpnt<16, 8, Saturate, std::uint16_t>>(reportTestCases)), "fixpnt<16,8,sat>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult((VerifyStandalone<fixpnt<16, 8, Saturate, std::uint16_t>>(reportTestCases)), "fixpnt<16,8,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyStandalone<fixpnt<16, 8, Modulo, std::uint16_t>>(reportTestCases)), "fixpnt<16,8,mod>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyStandalone<fixpnt<32, 24, Saturate, std::uint8_t>>(reportTestCases)), "fixpnt<32,24,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyStandalone<fixpnt<64, 32, Modulo, std::uint32_t>>(reportTestCases)), "fixpnt<64,32,mod>", test_tag);
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
