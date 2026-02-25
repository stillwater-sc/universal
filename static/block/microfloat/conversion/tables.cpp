// tables.cpp: exhaustive value table verification for microfloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define MICROFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/microfloat/microfloat.hpp>
#include <universal/verification/test_suite.hpp>

// Verify that to_float() -> from_float() is idempotent for all encodings
template<typename MicrofloatType>
int VerifyExhaustiveTable() {
	constexpr unsigned nbits = MicrofloatType::nbits;
	constexpr unsigned total_encodings = (1u << nbits);
	int nrOfFailedTestCases = 0;

	for (unsigned encoding = 0; encoding < total_encodings; ++encoding) {
		MicrofloatType a;
		a.setbits(encoding);

		// Skip NaN encodings for round-trip (NaN != NaN)
		if (a.isnan()) continue;

		float fv = float(a);
		MicrofloatType b(fv);

		if (a.iszero() && b.iszero()) continue; // +0 and -0 are both ok

		if (a.bits() != b.bits()) {
			// Check if the values are at least equal (handles +0/-0)
			if (float(a) != float(b)) {
				std::cerr << "FAIL: encoding 0x" << std::hex << encoding << std::dec
					<< " : " << sw::universal::to_binary(a) << " -> " << fv
					<< " -> " << sw::universal::to_binary(b)
					<< " (" << float(b) << ")\n";
				++nrOfFailedTestCases;
			}
		}
	}
	return nrOfFailedTestCases;
}

// Print the complete value table for a microfloat type
template<typename MicrofloatType>
void PrintValueTable() {
	constexpr unsigned nbits = MicrofloatType::nbits;
	constexpr unsigned total_encodings = (1u << nbits);

	for (unsigned encoding = 0; encoding < total_encodings; ++encoding) {
		MicrofloatType a;
		a.setbits(encoding);
		std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << encoding
			<< std::dec << " : " << sw::universal::to_binary(a)
			<< " : " << std::setw(12) << float(a) << '\n';
	}
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "microfloat exhaustive table verification";
	int nrOfFailedTestCases = 0;

	std::cout << "+---------    e2m1 value table (4-bit, 16 values)   --------+\n";
	PrintValueTable<e2m1>();
	nrOfFailedTestCases += VerifyExhaustiveTable<e2m1>();

	std::cout << "+---------    e2m3 value table (6-bit, 64 values)   --------+\n";
	PrintValueTable<e2m3>();
	nrOfFailedTestCases += VerifyExhaustiveTable<e2m3>();

	std::cout << "+---------    e3m2 value table (6-bit, 64 values)   --------+\n";
	PrintValueTable<e3m2>();
	nrOfFailedTestCases += VerifyExhaustiveTable<e3m2>();

	std::cout << "+---------    e4m3 value table (8-bit, 256 values)   --------+\n";
	// Only verify, don't print all 256 entries in test
	nrOfFailedTestCases += VerifyExhaustiveTable<e4m3>();
	std::cout << "e4m3 exhaustive table: " << (nrOfFailedTestCases == 0 ? "PASS" : "FAIL") << '\n';

	std::cout << "+---------    e5m2 value table (8-bit, 256 values)   --------+\n";
	nrOfFailedTestCases += VerifyExhaustiveTable<e5m2>();
	std::cout << "e5m2 exhaustive table: " << (nrOfFailedTestCases == 0 ? "PASS" : "FAIL") << '\n';

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
