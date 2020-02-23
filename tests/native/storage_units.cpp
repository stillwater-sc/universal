// storage_units.cpp: functional tests for storage units
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>

// minimum set of include files to reflect source code dependencies
#include "universal/native/byteArray.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"

// generate specific test case that you can trace with the trace conditions in fixpnt.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename StorageUnit = uint8_t>
void GenerateTestCase(int64_t _a, int64_t _b) {
	constexpr size_t bitsInStorageUnit = sizeof(StorageUnit) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInStorageUnit);
	StorageUnit a[nrUnits], b[nrUnits], result[nrUnits];
	using namespace sw::unum;
	setRawBits<nbits, StorageUnit>(a, uint64_t(_a));
	setRawBits<nbits, StorageUnit>(b, uint64_t(_b));
	copy<nbits, StorageUnit>(result, a);
	addBlockArray<nbits, StorageUnit>(result, b);
	int64_t ref = _a + _b;
	std::streamsize oldPrecision = std::cout.precision();
	//	std::cout << std::setprecision(nbits - 2);
	//	std::cout << std::setw(nbits) << _a << " + " << std::setw(nbits) << _b << " = " << std::setw(nbits) << ref << std::endl;
	//	std::cout << a << " + " << b << " = " << result << " (reference: " << ref << ")   " ;
	//	std::cout << (ref == result ? "PASS" : "FAIL") << std::endl << std::endl;
	//	std::cout << std::dec << std::setprecision(oldPrecision);
}

template<size_t nbits, typename StorageUnit = uint8_t>
void GenerateSequence(int upperbound, int stride = 1) {
	using namespace std;
	using namespace sw::unum;
	constexpr size_t bitsInStorageUnit = sizeof(StorageUnit) * 8;
	constexpr size_t nrUnits = 1 + ((nbits - 1) / bitsInStorageUnit);
	StorageUnit storage[nrUnits];
	for (int i = -upperbound; i < upperbound; i += stride) {
		setRawBits<nbits, StorageUnit>(storage, i);
		cout << to_hex<nbits, StorageUnit>(storage) << endl;
	}
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "storage units: ";

#if MANUAL_TESTING

	GenerateSequence<12, uint8_t>(32, 4);
	GenerateSequence<12, uint16_t>(32, 4);
	GenerateSequence<12, uint32_t>(32, 4);
	GenerateSequence<12, uint64_t>(32, 4);

	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, uint8_t>(12345, 54321);


#if STRESS_TESTING

#endif

#else

	cout << "storage unit tests" << endl;



#if STRESS_TESTING



#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
