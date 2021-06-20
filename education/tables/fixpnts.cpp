// fixpnts.cpp: generates encoding tables of fixed-point configurations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)  // unreferenced function is removed
#pragma warning(disable : 4710)  // function is not inlined
#pragma warning(disable : 5045)  // compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif 
// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include <universal/number/fixpnt/fixpnt_impl.hpp>
#include <universal/number/fixpnt/manipulators.hpp>
#include <universal/number/fixpnt/math_functions.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/fixpnt_test_suite.hpp>

// generate a full binary representation table for a given posit configuration
template<size_t nbits, size_t rbits>
void GenerateFixedPointTable(std::ostream& ostr, bool csvFormat = false) {
	const size_t size = (1 << nbits);
	sw::universal::fixpnt<nbits, rbits>	p;
	if (csvFormat) {
		ostr << "\"Generate Fixed-Point Lookup table for a FIXPNT<" << nbits << "," << rbits << "> in CSV format\"" << std::endl;
		ostr << "#, Binary, sign, scale, value\n";
		for (size_t i = 0; i < size; i++) {
			p.setbits(i);
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
			p.setbits(i);
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

	// Usage: fixpnts [-csv]
	bool csv = false;
	if (argc == 2) {
		if (std::string(argv[1]) == std::string("-csv")) csv = true;
	}
	cout << "Generate value tables for fixpnt configurations" << endl;


	GenerateFixedPointTable<4, 0>(cout, csv);
	GenerateFixedPointTable<4, 1>(cout, csv);
	GenerateFixedPointTable<4, 2>(cout, csv);
	GenerateFixedPointTable<4, 3>(cout, csv);
	GenerateFixedPointTable<4, 4>(cout, csv);

	GenerateFixedPointTable<5, 0>(cout, csv);
	GenerateFixedPointTable<5, 1>(cout, csv);
	GenerateFixedPointTable<5, 2>(cout, csv);
	GenerateFixedPointTable<5, 3>(cout, csv);
	GenerateFixedPointTable<5, 4>(cout, csv);
	GenerateFixedPointTable<5, 5>(cout, csv);

	GenerateFixedPointTable<6, 0>(cout, csv);
	GenerateFixedPointTable<6, 1>(cout, csv);
	GenerateFixedPointTable<6, 2>(cout, csv);
	GenerateFixedPointTable<6, 3>(cout, csv);
	GenerateFixedPointTable<6, 4>(cout, csv);
	GenerateFixedPointTable<6, 5>(cout, csv);
	GenerateFixedPointTable<6, 6>(cout, csv);

	GenerateFixedPointTable<8, 0>(cout, csv);
	GenerateFixedPointTable<8, 1>(cout, csv);
	GenerateFixedPointTable<8, 2>(cout, csv);
	GenerateFixedPointTable<8, 3>(cout, csv);
	GenerateFixedPointTable<8, 4>(cout, csv);
	GenerateFixedPointTable<8, 5>(cout, csv);
	GenerateFixedPointTable<8, 6>(cout, csv);	
	GenerateFixedPointTable<8, 7>(cout, csv);
	GenerateFixedPointTable<8, 8>(cout, csv);

	return EXIT_SUCCESS;
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
