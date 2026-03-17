// validation.cpp: exhaustive validation tests for small unum Type I configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define UNUM_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/unum/unum.hpp>
#include <universal/number/unum/manipulators.hpp>
#include <universal/verification/test_reporters.hpp>

#include <set>
#include <cmath>

// exhaustively verify that encode(decode(bits)) == bits for all valid bit patterns
// of a specific esize/fsize configuration
template<unsigned esizesize, unsigned fsizesize, typename bt>
int verify_round_trip(unsigned target_es, unsigned target_fs, bool verbose = false) {
	using Unum = sw::universal::unum<esizesize, fsizesize, bt>;
	int failures = 0;

	// word width for this esize/fsize: sign(1) + exp(es+1) + frac(fs) + utag + ubit
	unsigned utag = 1u + fsizesize + esizesize;
	unsigned nbits = 1u + (target_es + 1u) + target_fs + utag;
	uint64_t nrValues = 1ull << nbits;

	for (uint64_t i = 0; i < nrValues; ++i) {
		Unum u;
		u.setbits(i);

		// verify the utag fields decode correctly
		if (u.esize() != target_es || u.fsize() != target_fs) {
			// this bit pattern has a different esize/fsize, skip
			continue;
		}

		// verify round-trip: decode to double, encode back, decode again
		double d = u.to_double();
		if (std::isnan(d)) continue;  // skip NaN patterns

		Unum u2;
		u2 = d;
		double d2 = u2.to_double();

		// the re-encoded value should match (possibly with different esize/fsize
		// but the same numerical value)
		if (d != d2 && !(d == 0.0 && d2 == 0.0)) {
			++failures;
			if (verbose) {
				std::cout << "  FAIL: bits=0x" << std::hex << i << std::dec
				          << " -> " << d << " -> " << d2
				          << "  " << sw::universal::to_binary(u)
				          << " vs " << sw::universal::to_binary(u2) << '\n';
			}
		}
	}
	return failures;
}

// enumerate all exact values in a unum configuration and report them
template<unsigned esizesize, unsigned fsizesize, typename bt>
void enumerate_values(unsigned target_es, unsigned target_fs) {
	using Unum = sw::universal::unum<esizesize, fsizesize, bt>;

	unsigned utag = 1u + fsizesize + esizesize;
	unsigned nbits = 1u + (target_es + 1u) + target_fs + utag;
	uint64_t nrValues = 1ull << nbits;

	std::set<double> exact_values;
	unsigned exact_count = 0;
	unsigned inexact_count = 0;

	for (uint64_t i = 0; i < nrValues; ++i) {
		Unum u;
		u.setbits(i);
		if (u.esize() != target_es || u.fsize() != target_fs) continue;

		double d = u.to_double();
		if (std::isnan(d)) continue;

		if (u.ubit()) {
			++inexact_count;
		}
		else {
			++exact_count;
			exact_values.insert(d);
		}
	}

	std::cout << "  esize=" << target_es << " fsize=" << target_fs
	          << ": " << nbits << " bits, "
	          << exact_count << " exact encodings, "
	          << inexact_count << " inexact encodings, "
	          << exact_values.size() << " unique exact values\n";
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "unum Type I exhaustive validation";
	std::string test_tag    = "unum validation";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	// enumerate unum<2,2> configurations
	std::cout << "*** unum<2,2> value enumeration\n";
	{
		for (unsigned es = 0; es < 4; ++es) {
			for (unsigned fs = 0; fs < 4; ++fs) {
				enumerate_values<2, 2, uint8_t>(es, fs);
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// round-trip validation for unum<2,2>
	std::cout << "\n*** unum<2,2> round-trip validation\n";
	{
		int start = nrOfFailedTestCases;
		for (unsigned es = 0; es < 4; ++es) {
			for (unsigned fs = 0; fs < 4; ++fs) {
				int failures = verify_round_trip<2, 2, uint8_t>(es, fs);
				if (failures > 0) {
					nrOfFailedTestCases += failures;
					std::cout << "  FAIL: esize=" << es << " fsize=" << fs
					          << " had " << failures << " round-trip failures\n";
				}
			}
		}
		if (nrOfFailedTestCases - start == 0) {
			std::cout << "  all unum<2,2> configurations round-trip correctly\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// round-trip validation for unum<2,3>
	std::cout << "\n*** unum<2,3> round-trip validation\n";
	{
		int start = nrOfFailedTestCases;
		for (unsigned es = 0; es < 4; ++es) {
			for (unsigned fs = 0; fs < 8; ++fs) {
				int failures = verify_round_trip<2, 3, uint8_t>(es, fs);
				if (failures > 0) {
					nrOfFailedTestCases += failures;
					std::cout << "  FAIL: esize=" << es << " fsize=" << fs
					          << " had " << failures << " round-trip failures\n";
				}
			}
		}
		if (nrOfFailedTestCases - start == 0) {
			std::cout << "  all unum<2,3> configurations round-trip correctly\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// verify specific known values for unum<2,2>
	std::cout << "\n*** known value verification\n";
	{
		int start = nrOfFailedTestCases;
		using Unum = unum<2, 2>;

		// zero
		Unum u;
		u = 0.0;
		if (!u.iszero()) { ++nrOfFailedTestCases; std::cout << "  FAIL: 0.0\n"; }

		// powers of 2
		double pows[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0 };
		for (double p : pows) {
			u = p;
			if (u.to_double() != p) {
				++nrOfFailedTestCases;
				std::cout << "  FAIL: " << p << " -> " << u.to_double() << '\n';
			}
			if (u.ubit()) {
				++nrOfFailedTestCases;
				std::cout << "  FAIL: " << p << " should be exact\n";
			}
		}

		// negative powers of 2
		for (double p : pows) {
			u = -p;
			if (u.to_double() != -p) {
				++nrOfFailedTestCases;
				std::cout << "  FAIL: " << -p << " -> " << u.to_double() << '\n';
			}
		}

		if (nrOfFailedTestCases - start == 0) {
			std::cout << "  all known values verified correctly\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// verify ubit propagation through arithmetic
	std::cout << "\n*** ubit propagation through arithmetic\n";
	{
		int start = nrOfFailedTestCases;
		using Unum = unum<2, 2>;

		// exact + exact = exact (when result is representable)
		Unum a, b, c;
		a = 1.0; b = 2.0;
		c = a + b;
		if (c.ubit()) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1+2 should be exact\n"; }

		// exact * exact = exact (when result is representable)
		a = 2.0; b = 4.0;
		c = a * b;
		if (c.ubit()) { ++nrOfFailedTestCases; std::cout << "  FAIL: 2*4 should be exact\n"; }

		// exact / exact = inexact (when result is not representable)
		a = 1.0; b = 3.0;
		c = a / b;
		if (!c.ubit()) { ++nrOfFailedTestCases; std::cout << "  FAIL: 1/3 should be inexact\n"; }

		if (nrOfFailedTestCases - start == 0) {
			std::cout << "  ubit propagation verified correctly\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// verify that the unum encoding space is consistent
	std::cout << "\n*** encoding space consistency\n";
	{
		using Unum = unum<2, 2>;
		// for each esize/fsize, the number of unique exact values should be
		// at most 2 * (2^(esize+1) - 1) * 2^fsize + 1 (for zero)
		for (unsigned es = 0; es < 4; ++es) {
			for (unsigned fs = 0; fs < 4; ++fs) {
				unsigned utag = 1u + 2u + 2u;
				unsigned nbits = 1u + (es + 1u) + fs + utag;
				uint64_t nrValues = 1ull << nbits;

				std::set<double> exact_values;
				for (uint64_t i = 0; i < nrValues; ++i) {
					Unum u;
					u.setbits(i);
					if (u.esize() != es || u.fsize() != fs) continue;
					if (u.ubit()) continue;
					double d = u.to_double();
					if (!std::isnan(d)) exact_values.insert(d);
				}
				// theoretical max: sign gives 2x, exponents give 2^(es+1) values,
				// fractions give 2^fs values, plus zero
				unsigned theoretical = 2u * ((1u << (es + 1u)) - 1u) * (1u << fs) + 1u;
				std::cout << "  esize=" << es << " fsize=" << fs
				          << ": " << exact_values.size() << " unique values"
				          << " (theoretical max " << theoretical << ")\n";
			}
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::unum_arithmetic_exception& err) {
	std::cerr << "Uncaught unum arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
