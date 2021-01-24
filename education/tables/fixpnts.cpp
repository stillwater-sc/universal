// fixpnts.cpp: generates encoding tables of fixed-point configurations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include <universal/fixpnt/fixpnt.hpp>
#include <universal/fixpnt/manipulators.hpp>
#include <universal/fixpnt/math_functions.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/fixpnt_test_suite.hpp>

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

		// generate a full binary representation table for a given posit configuration
template<size_t nbits, size_t rbits>
void GenerateFixedPointTable(std::ostream& ostr, bool csvFormat = false) {
	const size_t size = (1 << nbits);
	sw::universal::fixpnt<nbits, rbits>	p;
	if (csvFormat) {
		ostr << "\"Generate Fixed-Point Lookup table for a FIXPNT<" << nbits << "," << rbits << "> in CSV format\"" << std::endl;
		ostr << "#, Binary, sign, scale, value\n";
		for (size_t i = 0; i < size; i++) {
			p.set_raw_bits(i);
			ostr << i << ","
				<< to_binary(p) << ","
				<< p.sign() << ","
				<< scale(p) << ","
				<< p
				<< '\n';
		}
		ostr << std::endl;
	}
	else {
		ostr << "Generate Fixed-Point Lookup table for a FIXPNT<" << nbits << "," << rbits << "> in TXT format" << std::endl;

		const size_t index_column = 5;
		const size_t bin_column = 16;
		const size_t sign_column = 8;
		const size_t scale_column = 8;
		const size_t value_column = 30;
		const size_t format_column = 16;

		ostr << std::setw(index_column) << " # "
			<< std::setw(bin_column) << "Binary"
			<< std::setw(sign_column) << "sign"
			<< std::setw(scale_column) << "scale"
			<< std::setw(value_column) << "value"
			<< std::setw(format_column) << "format"
			<< std::endl;
		for (size_t i = 0; i < size; i++) {
			p.set_raw_bits(i);
			ostr << std::setw(4) << i << ": "
				<< std::setw(bin_column) << to_binary(p)
				<< std::setw(sign_column) << p.sign()
				<< std::setw(scale_column) << scale(p)
				<< std::setw(value_column) << p << " "
				<< std::setw(format_column) << std::right << p
				<< std::endl;
		}
	}
}
int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::string tag = "Generate fixed-point value tables";

#if MANUAL_TESTING

	fixpnt<8, 4> f;
	f = 3.5f;
	bitset<8> bs(f.byte(0));
	cout << bs << endl;
	cout << f << endl;



#if STRESS_TESTING
	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, 0, Modular, uint8_t>("Manual Testing", true), "fixpnt<4,0,Modular,uint8_t>", "addition");
#endif

#else

	cout << "Generate fixed-point value tables:\n";

	GenerateFixedPointTable<4, 0>(cout, false);
	GenerateFixedPointTable<4, 3>(cout, false);
	GenerateFixedPointTable<4, 4>(cout, false);

	GenerateFixedPointTable<8, 7>(cout, false);

#if STRESS_TESTING


#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
