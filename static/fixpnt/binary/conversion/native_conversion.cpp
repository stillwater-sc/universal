//  native_conversion.cpp : test suite for conversion of native integers and floats to arbitrary configuration fixed-point numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_suite.hpp>

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

namespace {

	// an integer as a sign and a 64-bit magnitude, so every native integer type fits one form
	struct Candidate {
		bool               negative;
		unsigned long long magnitude;
	};

	template<typename Int>
	bool Fits(const Candidate& c) {
		if (c.negative) {
			if constexpr (std::is_unsigned_v<Int>) return false;
			else return c.magnitude <= 0ull - static_cast<unsigned long long>(std::numeric_limits<Int>::min());
		}
		return c.magnitude <= static_cast<unsigned long long>(std::numeric_limits<Int>::max());
	}

	template<typename Int>
	Int Make(const Candidate& c) {
		// modular conversion of the two's complement pattern: exact because Fits<Int>(c)
		unsigned long long pattern = c.negative ? 0ull - c.magnitude : c.magnitude;
		return static_cast<Int>(pattern);
	}

	// the reference: the integer part as a 64-bit two's complement pattern plus the value of every
	// bit above it, independent of fixpnt's conversion code
	struct Expected {
		bool               saturatePos;
		bool               saturateNeg;
		unsigned long long pattern;   // bit k of the integer part, k < 64
		bool               extension; // bit k of the integer part, k >= 64
	};

	template<unsigned nbits, unsigned rbits, bool saturating>
	Expected Reference(const Candidate& c) {
		constexpr unsigned ib = nbits - rbits;
		Expected e{ false, false, 0, false };
		if constexpr (saturating) {
			if constexpr (ib == 0) {
				if (c.magnitude != 0) { e.saturatePos = !c.negative; e.saturateNeg = c.negative; return e; }
			}
			else if constexpr (ib - 1 < 64) {
				const unsigned long long bound = 1ull << (ib - 1); // the integer part holds [-bound, bound)
				if (!c.negative && c.magnitude >= bound) { e.saturatePos = true; return e; }
				if (c.negative && c.magnitude > bound)   { e.saturateNeg = true; return e; }
			}
		}
		const bool negative = c.negative && c.magnitude != 0;
		unsigned long long pattern = negative ? 0ull - c.magnitude : c.magnitude;
		bool extension = negative;
		if constexpr (ib < 64) {
			// keep the low ib bits and sign-extend from bit ib-1: modulo 2^ib, or exact when in range
			if constexpr (ib == 0) {
				pattern = 0;
				extension = false;
			}
			else {
				const unsigned long long mask = (1ull << ib) - 1ull;
				pattern &= mask;
				extension = ((pattern >> (ib - 1)) & 1ull) != 0;
				if (extension) pattern |= ~mask;
			}
		}
		e.pattern = pattern;
		e.extension = extension;
		return e;
	}

	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	bool Matches(const sw::universal::fixpnt<nbits, rbits, arithmetic, bt>& f, const Expected& e) {
		using Fixpnt = sw::universal::fixpnt<nbits, rbits, arithmetic, bt>;
		if (e.saturatePos) return f == Fixpnt(sw::universal::SpecificValue::maxpos);
		if (e.saturateNeg) return f == Fixpnt(sw::universal::SpecificValue::maxneg);
		for (unsigned i = 0; i < rbits; ++i) {
			if (f.at(i)) return false; // an integer has no fraction bits
		}
		for (unsigned i = rbits; i < nbits; ++i) {
			const unsigned k = i - rbits;
			const bool want = (k < 64) ? (((e.pattern >> k) & 1ull) != 0) : e.extension;
			if (f.at(i) != want) return false;
		}
		return true;
	}

	template<typename Fixpnt, typename Int>
	int VerifyType(const std::vector<Candidate>& candidates, const char* intName, bool reportTestCases) {
		constexpr unsigned nbits = Fixpnt::nbits;
		constexpr unsigned rbits = Fixpnt::rbits;
		constexpr bool saturating = (Fixpnt::arithmetic == sw::universal::Saturate);
		int fails = 0;
		for (const Candidate& c : candidates) {
			if (!Fits<Int>(c)) continue;
			const Int v = Make<Int>(c);
			const Expected e = Reference<nbits, rbits, saturating>(c);
			const Fixpnt constructed(v);
			Fixpnt assigned;
			assigned = v;
			const bool ok = Matches(constructed, e) && Matches(assigned, e);
			if (!ok) {
				++fails;
				if (reportTestCases) {
					std::cerr << "FAIL: " << sw::universal::type_tag(constructed) << " from " << intName << ' '
					          << (c.negative ? "-" : "") << c.magnitude << " -> " << sw::universal::to_binary(constructed) << '\n';
				}
			}
		}
		return fails;
	}

	// every native integer type through constructor and assignment, against the bit-level reference
	template<typename Fixpnt>
	int VerifyIntegerConversion(bool reportTestCases) {
		constexpr unsigned ib = Fixpnt::nbits - Fixpnt::rbits;
		std::vector<Candidate> candidates;
		for (unsigned long long m : { 0ull, 1ull, 2ull, 3ull, 6ull, 100ull, 127ull, 128ull, 129ull, 200ull, 255ull, 256ull,
		                              32767ull, 32768ull, 32769ull, 65535ull, 65536ull,
		                              2147483647ull, 2147483648ull, 2147483649ull, 4294967295ull, 4294967296ull,
		                              1000000000000ull, 9223372036854775807ull, 9223372036854775808ull,
		                              18446744073709551615ull }) {
			candidates.push_back({ false, m });
			candidates.push_back({ true, m });
		}
		// the edges of this configuration's integer range: -2^(ib-1) - 1, -2^(ib-1), 2^(ib-1) - 1, 2^(ib-1)
		if constexpr (ib >= 1 && ib - 1 < 64) {
			const unsigned long long bound = 1ull << (ib - 1);
			for (unsigned long long m : { bound - 1ull, bound, bound + 1ull }) {
				candidates.push_back({ false, m });
				candidates.push_back({ true, m });
			}
		}
		int fails = 0;
		fails += VerifyType<Fixpnt, signed char>(candidates, "signed char", reportTestCases);
		fails += VerifyType<Fixpnt, short>(candidates, "short", reportTestCases);
		fails += VerifyType<Fixpnt, int>(candidates, "int", reportTestCases);
		fails += VerifyType<Fixpnt, long>(candidates, "long", reportTestCases);
		fails += VerifyType<Fixpnt, long long>(candidates, "long long", reportTestCases);
		fails += VerifyType<Fixpnt, char>(candidates, "char", reportTestCases);
		fails += VerifyType<Fixpnt, unsigned short>(candidates, "unsigned short", reportTestCases);
		fails += VerifyType<Fixpnt, unsigned int>(candidates, "unsigned int", reportTestCases);
		fails += VerifyType<Fixpnt, unsigned long>(candidates, "unsigned long", reportTestCases);
		fails += VerifyType<Fixpnt, unsigned long long>(candidates, "unsigned long long", reportTestCases);
		return fails;
	}

	// values m * 2^k that are exact in double and in the fixpnt: converting -v must give -fixpnt(v),
	// and both must read back exactly
	template<typename Fixpnt>
	int VerifyFloatConversion(bool reportTestCases) {
		constexpr int ib = static_cast<int>(Fixpnt::nbits - Fixpnt::rbits);
		constexpr int rbits = static_cast<int>(Fixpnt::rbits);
		int fails = 0;
		for (int m : { 1, 3, 5, 15, 255, 1048575 }) {
			const int mbits = static_cast<int>(std::floor(std::log2(m))) + 1;
			// m * 2^k needs k >= -rbits for its lsb and mbits + k <= ib - 1 to stay below 2^(ib-1)
			for (int k = -rbits; k + mbits <= ib - 1; ++k) {
				const double v = std::ldexp(static_cast<double>(m), k);
				const Fixpnt pos(v), neg(-v);
				const bool ok = (neg == -pos) && (double(pos) == v) && (double(neg) == -v);
				if (!ok) {
					++fails;
					if (reportTestCases) {
						std::cerr << "FAIL: " << sw::universal::type_tag(pos) << " from double +/-" << v << " -> "
						          << sw::universal::to_binary(pos) << " / " << sw::universal::to_binary(neg) << '\n';
					}
				}
			}
		}
		return fails;
	}

	template<unsigned nbits, unsigned rbits, typename bt>
	void VerifyBothModes(bool reportTestCases, int& nrOfFailedTestCases) {
		using namespace sw::universal;
		using Sat = fixpnt<nbits, rbits, Saturate, bt>;
		using Mod = fixpnt<nbits, rbits, Modulo, bt>;
		nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<Sat>(reportTestCases), type_tag(Sat()), "integer conversion");
		nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<Mod>(reportTestCases), type_tag(Mod()), "integer conversion");
		if constexpr (nbits > rbits) { // the float check needs integer bits for its sign
			nrOfFailedTestCases += ReportTestResult(VerifyFloatConversion<Sat>(reportTestCases), type_tag(Sat()), "double conversion");
			nrOfFailedTestCases += ReportTestResult(VerifyFloatConversion<Mod>(reportTestCases), type_tag(Mod()), "double conversion");
		}
	}

}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fixpnt conversion from native types ";
	std::string test_tag    = "native conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using Fixpnt = fixpnt<65, 32, Saturate, std::uint32_t>;
	Fixpnt a(1), b(2);
	std::cout << to_binary(a) << " / " << to_binary(b) << " = " << to_binary(a / b) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	// Saturate used to compare v against Arith(maxpos), which wraps as soon as the integer part is
	// wider than Arith: fixpnt<65,32,Saturate>(1) was maxpos, and every unsigned source compared
	// against the raw encoding, so fixpnt<16,8,Saturate>(1u) was maxneg. A negative double lost
	// its sign above bit 63: fixpnt<65,32>(-2.0) was 2^32 - 2 (#1906)
	VerifyBothModes<  8,  0, std::uint8_t >(reportTestCases, nrOfFailedTestCases);
	VerifyBothModes<  8,  4, std::uint8_t >(reportTestCases, nrOfFailedTestCases);
	VerifyBothModes<  8,  8, std::uint8_t >(reportTestCases, nrOfFailedTestCases);
	VerifyBothModes< 16,  8, std::uint8_t >(reportTestCases, nrOfFailedTestCases);
	VerifyBothModes< 32, 16, std::uint16_t>(reportTestCases, nrOfFailedTestCases);
	VerifyBothModes< 48,  8, std::uint32_t>(reportTestCases, nrOfFailedTestCases);
	VerifyBothModes< 64, 32, std::uint32_t>(reportTestCases, nrOfFailedTestCases);
	VerifyBothModes< 64,  0, std::uint64_t>(reportTestCases, nrOfFailedTestCases);
	VerifyBothModes< 65, 32, std::uint32_t>(reportTestCases, nrOfFailedTestCases);
	VerifyBothModes< 80, 40, std::uint8_t >(reportTestCases, nrOfFailedTestCases);
	VerifyBothModes< 96,  8, std::uint32_t>(reportTestCases, nrOfFailedTestCases);
	VerifyBothModes<128, 32, std::uint64_t>(reportTestCases, nrOfFailedTestCases);
	VerifyBothModes<128, 64, std::uint64_t>(reportTestCases, nrOfFailedTestCases);
	VerifyBothModes<200,100, std::uint32_t>(reportTestCases, nrOfFailedTestCases);
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
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
