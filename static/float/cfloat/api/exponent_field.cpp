// exponent_field.cpp: the exponent field read and written a limb at a time
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <string>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

/*
   cfloat::exponent() and setexponent() used to move the exponent one bit at a time whenever the
   field does not fit in the most significant limb. Every add calls them several times through
   isnormal(), isdenormal() and scale(), and that made cfloat<196,19> with uint32_t or uint64_t
   limbs, and cfloat<128,15,uint8_t>, 3x to 5x slower than the widths around them (#1919). Both
   now gather or scatter the field a limb at a time.

   This test holds them to the per-bit definition: on random encodings, exponent() returns the bits
   at() reads at positions fbits .. fbits + es - 1, and setexponent() writes exactly those bits and
   leaves every other bit as it was. The configurations put the field inside one limb, across two
   limbs, and (with uint8_t limbs and es = 19) across three. The block width is the only
   difference within a group, so the per-bit reference is the same for all of them.
*/

namespace sw {
namespace universal {
namespace exponent_field {

struct Rng {  // splitmix64: reproducible on every platform
	std::uint64_t state;
	std::uint64_t next() {
		std::uint64_t z = (state += 0x9E3779B97F4A7C15ull);
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}
};

template<typename Cfloat>
void randomize(Cfloat& c, Rng& rng) {
	std::uint64_t word = 0;
	for (unsigned i = 0; i < Cfloat::nbits; ++i) {
		if ((i % 64u) == 0) word = rng.next();
		c.setbit(i, ((word >> (i % 64u)) & 1ull) != 0);
	}
}

template<typename Cfloat>
std::uint64_t referenceExponent(const Cfloat& c) {
	std::uint64_t e = 0;
	for (unsigned i = 0; i < Cfloat::es; ++i) {
		if (c.at(Cfloat::fbits + i)) e |= (1ull << i);
	}
	return e;
}

template<unsigned nbits, unsigned es, typename bt>
int VerifyExponentField(bool reportTestCases) {
	using Cfloat                 = cfloat<nbits, es, bt, true, false, false>;
	constexpr unsigned fbits     = Cfloat::fbits;
	constexpr std::uint64_t mask = (1ull << es) - 1ull;
	Rng rng{ 0x1919ull * nbits + es * 131ull + sizeof(bt) };
	int fails = 0;
	for (int trial = 0; trial < 2000; ++trial) {
		Cfloat c{};
		randomize(c, rng);

		// read: exponent() agrees with the bits at() reads
		blockbinary<es, bt> e;
		c.exponent(e);
		const std::uint64_t expected = referenceExponent(c);
		if (static_cast<std::uint64_t>(e.to_ull()) != expected) {
			if (reportTestCases)
				std::cerr << "FAIL exponent(): " << to_binary(c) << " gave " << to_binary(e) << " expected 0x" << std::hex
				          << expected << std::dec << '\n';
			++fails;
		}

		// write: setexponent() writes exactly the field, for a normal scale
		const int scale = Cfloat::MIN_EXP_NORMAL + static_cast<int>(rng.next() % static_cast<std::uint64_t>(Cfloat::MAX_EXP - Cfloat::MIN_EXP_NORMAL + 1));
		Cfloat d = c;
		if (!d.setexponent(scale)) {
			if (reportTestCases) std::cerr << "FAIL setexponent(" << scale << ") refused a normal scale\n";
			++fails;
			continue;
		}
		const std::uint64_t field = static_cast<std::uint64_t>(scale + Cfloat::EXP_BIAS) & mask;
		bool ok = (referenceExponent(d) == field);
		for (unsigned i = 0; ok && i < nbits; ++i) {
			if (i >= fbits && i < fbits + es) continue;
			ok = (d.at(i) == c.at(i));
		}
		if (!ok) {
			if (reportTestCases)
				std::cerr << "FAIL setexponent(" << scale << "): " << to_binary(c) << " -> " << to_binary(d) << '\n';
			++fails;
		}
	}
	if constexpr (nbits < 65) {
		// a subnormal scale writes an all-zeros field below 65 bits
		Cfloat c{};
		randomize(c, rng);
		c.setexponent(Cfloat::MIN_EXP_NORMAL - 1);
		if (referenceExponent(c) != 0) {
			if (reportTestCases) std::cerr << "FAIL setexponent(subnormal scale): " << to_binary(c) << '\n';
			++fails;
		}
	}
	return fails;
}

}  // namespace exponent_field
}  // namespace universal
}  // namespace sw

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;
	using namespace sw::universal::exponent_field;

	std::string test_suite          = "cfloat exponent field across limbs";
	std::string test_tag            = "exponent field";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifyExponentField<196, 19, std::uint64_t>(reportTestCases);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#else

#	if REGRESSION_LEVEL_1
	// the field inside the most significant limb, the original fast path
	nrOfFailedTestCases += ReportTestResult(VerifyExponentField<32, 8, std::uint32_t>(reportTestCases), "cfloat<32,8,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentField<256, 19, std::uint64_t>(reportTestCases), "cfloat<256,19,uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentField<16, 5, std::uint8_t>(reportTestCases), "cfloat<16,5,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentField<80, 11, std::uint16_t>(reportTestCases), "cfloat<80,11,uint16_t>", test_tag);
	// the field across two limbs
	nrOfFailedTestCases += ReportTestResult(VerifyExponentField<40, 8, std::uint8_t>(reportTestCases), "cfloat<40,8,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentField<80, 11, std::uint8_t>(reportTestCases), "cfloat<80,11,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentField<128, 15, std::uint8_t>(reportTestCases), "cfloat<128,15,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentField<196, 19, std::uint32_t>(reportTestCases), "cfloat<196,19,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentField<196, 19, std::uint64_t>(reportTestCases), "cfloat<196,19,uint64_t>", test_tag);
	// the field across three limbs
	nrOfFailedTestCases += ReportTestResult(VerifyExponentField<196, 19, std::uint8_t>(reportTestCases), "cfloat<196,19,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentField<256, 19, std::uint8_t>(reportTestCases), "cfloat<256,19,uint8_t>", test_tag);
#	endif

#	if REGRESSION_LEVEL_2
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
} catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
} catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
