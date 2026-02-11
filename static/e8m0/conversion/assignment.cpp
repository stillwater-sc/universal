// assignment.cpp: exhaustive 256-value table verification for e8m0
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define E8M0_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/e8m0/e8m0.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "e8m0 exhaustive conversion tests";
	int nrOfFailedTestCases = 0;

	// Verify all 256 encodings
	std::cout << "+---------    e8m0 exhaustive table verification (256 encodings)   --------+\n";
	{
		for (unsigned encoding = 0; encoding < 256; ++encoding) {
			e8m0 a;
			a.setbits(encoding);

			if (encoding == 0xFF) {
				// NaN encoding
				if (!a.isnan()) {
					std::cerr << "FAIL: encoding 0xFF should be NaN\n";
					++nrOfFailedTestCases;
				}
				continue;
			}

			// Verify the value is 2^(encoding - 127)
			float expected = std::ldexp(1.0f, static_cast<int>(encoding) - 127);
			float actual = float(a);

			if (actual != expected) {
				std::cerr << "FAIL: encoding " << encoding
					<< " expected " << expected
					<< " got " << actual << '\n';
				++nrOfFailedTestCases;
			}

			// Verify round-trip: from_float(to_float(encoding)) == encoding
			e8m0 b(actual);
			if (b.bits() != static_cast<uint8_t>(encoding)) {
				std::cerr << "FAIL: round-trip for encoding " << encoding
					<< " : to_float=" << actual
					<< " back to encoding=" << unsigned(b.bits()) << '\n';
				++nrOfFailedTestCases;
			}
		}
	}

	// Verify specific known values
	std::cout << "+---------    e8m0 specific value verification   --------+\n";
	{
		struct TestCase {
			unsigned encoding;
			float expected_value;
			const char* description;
		};

		TestCase tests[] = {
			{ 0,   std::ldexp(1.0f, -127), "2^-127 (smallest)" },
			{ 1,   std::ldexp(1.0f, -126), "2^-126" },
			{ 127, 1.0f,                   "2^0 = 1.0" },
			{ 128, 2.0f,                   "2^1 = 2.0" },
			{ 129, 4.0f,                   "2^2 = 4.0" },
			{ 126, 0.5f,                   "2^-1 = 0.5" },
			{ 254, std::ldexp(1.0f, 127),  "2^127 (largest)" },
		};

		for (const auto& tc : tests) {
			e8m0 a;
			a.setbits(tc.encoding);
			float actual = float(a);
			if (actual != tc.expected_value) {
				std::cerr << "FAIL: " << tc.description
					<< " encoding=" << tc.encoding
					<< " expected=" << tc.expected_value
					<< " got=" << actual << '\n';
				++nrOfFailedTestCases;
			}
			else {
				std::cout << to_binary(a) << " : " << actual << " (" << tc.description << ") PASS\n";
			}
		}
	}

	// Verify that e8m0 cannot represent zero
	std::cout << "+---------    e8m0 cannot represent zero   --------+\n";
	{
		e8m0 a(0.0f);
		std::cout << "e8m0(0.0f) : " << to_binary(a) << " : " << a << " (clamped to minpos)\n";
		if (a.iszero()) {
			std::cerr << "FAIL: e8m0 should never return iszero() == true\n";
			++nrOfFailedTestCases;
		}
	}

	// Verify negative values are clamped
	std::cout << "+---------    e8m0 negative value clamping   --------+\n";
	{
		e8m0 a(-1.0f);
		std::cout << "e8m0(-1.0f) : " << to_binary(a) << " : " << a << " (clamped to minpos)\n";
		// Should be clamped to encoding 0 (smallest value)
		if (a.bits() != 0) {
			std::cerr << "FAIL: e8m0(-1.0) should clamp to encoding 0\n";
			++nrOfFailedTestCases;
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
