// test_decimal_to_binary_oracle.cpp: exact validation of decimal string parsing at multi-component precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// test_decimal_to_binary.cpp already checks this machinery against std::stod. That caps verification
// at 53 bits: it can say a double came out right, and nothing about the 106, 159 and 212-bit results
// the multi-component types ask for. Everything above double precision is currently unverified,
// which is a poor place to start optimizing from (universal#1319).
//
// This suite closes that. The reference is the decimal string's own exact value, computed in exact
// integer arithmetic, so it is correct at any precision and shares no code with the converter.
//
// The comparison is exact, with no rounding anywhere in the oracle:
//
//   A parsed multi-component value is a sum of doubles, so it is a dyadic rational D = n * 2^s.
//   A decimal string is M * 10^E for an integer M and an integer E. That is dyadic only when
//   E >= 0, because 10^E = 2^E * 5^E and a negative E divides by 5^|E|. Rather than approximate,
//   both sides are scaled by 5^|E| when E is negative:
//
//       |D - M*10^E| <= tol * |M*10^E|
//   <=> |D*5^|E| - M*2^E| <= tol * |M*2^E|          (5^|E| > 0, so the inequality is preserved)
//
//   and every term in that form is a dyadic rational the oracle can build exactly.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <string>
#include <vector>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/dyadic_exact.hpp>

namespace {

	using BigInt = sw::universal::dyadic::bigint;

	// Scan a decimal literal into an exact (mantissa, base-10 exponent) pair.
	//
	// Deliberately hand-written rather than calling the library's scanner: an oracle that shares its
	// front end with the code under test cannot detect a fault in that front end.
	bool scan_exact(const std::string& text, BigInt& mantissa, int& exponent10, bool& negative) {
		std::size_t i = 0;
		negative = false;
		if (i < text.size() && (text[i] == '+' || text[i] == '-')) {
			negative = (text[i] == '-');
			++i;
		}
		BigInt m(0);
		const BigInt ten(10);
		int fractionDigits = 0;
		bool sawDigit = false;
		for (; i < text.size() && text[i] >= '0' && text[i] <= '9'; ++i) {
			m = m * ten + BigInt(text[i] - '0');
			sawDigit = true;
		}
		if (i < text.size() && text[i] == '.') {
			++i;
			for (; i < text.size() && text[i] >= '0' && text[i] <= '9'; ++i) {
				m = m * ten + BigInt(text[i] - '0');
				++fractionDigits;
				sawDigit = true;
			}
		}
		if (!sawDigit) return false;
		int exp = 0;
		if (i < text.size() && (text[i] == 'e' || text[i] == 'E')) {
			++i;
			bool expNegative = false;
			if (i < text.size() && (text[i] == '+' || text[i] == '-')) {
				expNegative = (text[i] == '-');
				++i;
			}
			int value = 0;
			bool sawExpDigit = false;
			for (; i < text.size() && text[i] >= '0' && text[i] <= '9'; ++i) {
				value = value * 10 + (text[i] - '0');
				sawExpDigit = true;
			}
			if (!sawExpDigit) return false;
			exp = expNegative ? -value : value;
		}
		if (i != text.size()) return false;
		mantissa = m;
		exponent10 = exp - fractionDigits;
		return true;
	}

	BigInt power_of_five(int e) {
		BigInt result(1);
		const BigInt five(5);
		for (int i = 0; i < e; ++i) result = result * five;
		return result;
	}

	// exact dyadic value of a multi-component number: the sum of its components
	template<typename Scalar, unsigned NR_LIMBS>
	sw::universal::dyadic exact_value(const Scalar& v) {
		using namespace sw::universal;
		dyadic d;
		for (unsigned i = 0; i < NR_LIMBS; ++i) d = d + dyadic::from_double(v[static_cast<int>(i)]);
		return d;
	}

	bool magnitude_leq(const sw::universal::dyadic& a, const sw::universal::dyadic& b) {
		using namespace sw::universal;
		dyadic::bigint na, nb;
		int common{ 0 };
		dyadic_align(a, b, na, nb, common);
		return abs(na) <= abs(nb);
	}

	// |parsed - exact| <= 2^-toleranceBits * |exact|
	bool within_tolerance(const sw::universal::dyadic& parsed, const sw::universal::dyadic& exact, int toleranceBits) {
		using namespace sw::universal;
		dyadic residual = exact - parsed;
		if (residual.iszero()) return true;
		dyadic scaled(residual.numerator, residual.scale + toleranceBits);
		return magnitude_leq(scaled, exact);
	}

	// The test vectors: shapes that exercise different paths through the converter rather than a
	// uniform random sweep, because the interesting failures live at the boundaries.
	std::vector<std::string> decimal_test_vectors() {
		std::vector<std::string> v = {
			// short literals - the case that pays the fixed cost for nothing
			"1.5", "0.5", "2.0", "3.14159", "-1.5", "+2.25", "0.1", "0.2", "0.3",
			// exactly representable, and one ulp either side of a tie
			"1.0", "0.25", "1024.0", "1.0000000000000002", "0.9999999999999999",
			// digit counts crossing the double, dd, td and qd significands
			"1.2345678901234567",
			"1.23456789012345678901234567890123",
			"1.234567890123456789012345678901234567890123456789",
			"1.2345678901234567890123456789012345678901234567890123456789012345",
			// large and small exponents, including where the intermediate needs the most bits
			"1.0e10", "1.0e-10", "1.0e100", "1.0e-100", "1.0e300", "1.0e-300",
			"1.7976931348623157e308", "2.2250738585072014e-308",
			"9.999999999999999e299", "1.234567890123456789e-299",
			// no fractional part, and no integer part
			"12345678901234567890", ".5", ".0009765625",
			// long runs that stress the digit loop
			"3.141592653589793238462643383279502884197169399375105820974944592307816406286",
			"2.718281828459045235360287471352662497757247093699959574966967627724076630354",
		};
		return v;
	}

	// How many significand bits this format can actually hold at this magnitude.
	//
	// A multi-component value stacks doubles, each about 2^-53 below the last, so the nominal
	// significand is 53*N bits. Near the bottom of the exponent range that is not available: a
	// trailing component whose exponent would fall below 2^-1022 becomes subnormal and carries
	// fewer bits, and below 2^-1074 it cannot exist at all. Parsing 1e-300 into a double-double
	// therefore yields about 78 bits, not 106, no matter how good the converter is - the second
	// component would need exponent 2^-1050, which is deep in the subnormal range.
	//
	// The oracle has to know this, or it reports a format limit as a parser bug. It did, until
	// three large-negative-exponent vectors flagged and an independent check showed the converter
	// was right and the budget was wrong.
	unsigned representable_bits(double leading, unsigned nominalBits) {
		if (leading == 0.0) return nominalBits;
		int e{ 0 };
		std::frexp(leading, &e);                       // |leading| in [2^(e-1), 2^e)
		constexpr int smallestSubnormalExponent = -1074;
		int available = e - smallestSubnormalExponent;
		if (available < 1) return 1;
		return (static_cast<unsigned>(available) < nominalBits) ? static_cast<unsigned>(available) : nominalBits;
	}

	// Verify one type at one precision. toleranceBits is the budget in bits of the target
	// significand: a correctly rounded parse lands within half an ulp, i.e. precision bits.
	template<typename Scalar, unsigned NR_LIMBS>
	int VerifyParseAgainstExact(bool reportTestCases, int toleranceBits, const std::string& tag) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;
		for (const auto& text : decimal_test_vectors()) {
			BigInt mantissa;
			int exponent10{ 0 };
			bool negative{ false };
			if (!scan_exact(text, mantissa, exponent10, negative)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "  oracle could not scan \"" << text << "\"\n";
				continue;
			}

			Scalar parsed(0.0);   // assign() leaves the value untouched if the text does not parse
			parsed.assign(text);
			dyadic computed = exact_value<Scalar, NR_LIMBS>(parsed);

			// build both sides so that every term is dyadic (see the header comment)
			dyadic lhs, rhs;
			if (exponent10 >= 0) {
				BigInt scaledMantissa = mantissa * power_of_five(exponent10);
				if (negative) scaledMantissa = -scaledMantissa;
				lhs = computed;
				rhs = dyadic(scaledMantissa, exponent10);
			}
			else {
				BigInt fiveToTheE = power_of_five(-exponent10);
				BigInt signedMantissa = negative ? -mantissa : mantissa;
				lhs = computed * dyadic(fiveToTheE, 0);
				rhs = dyadic(signedMantissa, exponent10);
			}

			int budget = static_cast<int>(representable_bits(parsed[0], static_cast<unsigned>(toleranceBits + 1))) - 1;
			if (!within_tolerance(lhs, rhs, budget)) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cerr << "  FAIL " << tag << " parsing \"" << text
					          << "\" : more than 2^-" << budget << " relative from exact";
					if (budget != toleranceBits) std::cerr << " (budget reduced from 2^-" << toleranceBits
					                                       << ": the trailing components are subnormal at this magnitude)";
					std::cerr << '\n';
				}
			}
		}
		return nrOfFailedTests;
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

	std::string test_suite  = "decimal string parsing against exact integer arithmetic";
	std::string test_tag    = "parse oracle";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// A correctly rounded parse lands within half an ulp of the target significand, i.e. within
	// 2^-precision relative. These budgets sit one bit looser, which still fails an implementation
	// that drops a component or mis-scales, while tolerating a legitimately different tie break.
	constexpr int DD_TOLERANCE = 105;   // 106-bit significand
	constexpr int TD_TOLERANCE = 158;   // 159-bit
	constexpr int QD_TOLERANCE = 211;   // 212-bit

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifyParseAgainstExact<dd_cascade, 2>(true, DD_TOLERANCE, "dd_cascade");
	nrOfFailedTestCases += VerifyParseAgainstExact<qd_cascade, 4>(true, QD_TOLERANCE, "qd_cascade");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyParseAgainstExact<dd, 2>(reportTestCases, DD_TOLERANCE, "dd"), test_tag, "dd");
	nrOfFailedTestCases += ReportTestResult(VerifyParseAgainstExact<dd_cascade, 2>(reportTestCases, DD_TOLERANCE, "dd_cascade"), test_tag, "dd_cascade");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyParseAgainstExact<td_cascade, 3>(reportTestCases, TD_TOLERANCE, "td_cascade"), test_tag, "td_cascade");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyParseAgainstExact<qd, 4>(reportTestCases, QD_TOLERANCE, "qd"), test_tag, "qd");
	nrOfFailedTestCases += ReportTestResult(VerifyParseAgainstExact<qd_cascade, 4>(reportTestCases, QD_TOLERANCE, "qd_cascade"), test_tag, "qd_cascade");
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
