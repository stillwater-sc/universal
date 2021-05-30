// tables.cpp: test suite runner for blocktriple value enumeration
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

// minimum set of include files to reflect source code dependencies
#include <universal/native/integers.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>

namespace sw::universal {

	/// <summary>
	/// generate a full binary representation table for a given blocktriple configuration
	/// </summary>
	/// <typeparam name="bt">type of the storage block used to represent the bfloat</typeparam>
	/// <param name="ostr">ostream reference to write to</param>
	/// <param name="uncertainty">if true output certain and uncertain values, otherwise only certain values</param>
	/// <param name="csvFormat">if true present as a comma separated value format, text otherwise</param>
	template<typename TestType>
	void GenerateTable(std::ostream& ostr, bool csvFormat = false) {
		constexpr size_t nbits = TestType::nbits;
		constexpr size_t bfbits = TestType::bfbits;
		using bt = typename TestType::BlockType;
		constexpr size_t NR_VALUES = (1 << nbits);
		TestType v;

		// we are going to enumerate the blocktriple's fraction bits.
		// By design, a blocktriple is a normalized floating point number
		// with the leading bit explicitely set to '1'.

		// set blocktriple to a normal encoding
		v.setnormal();
		v.setsign(false);
		if (csvFormat) {
			ostr << "\"Generate Lookup table for a " << typeid(v).name() << " in CSV format\"" << std::endl;
			ostr << "#, Binary, sign, scale, exponent, fraction, value, hex\n";
			for (size_t i = 0; i < NR_VALUES; i++) {
				v.setbits(i + NR_VALUES);
				bool s = v.sign();
				int scale = v.scale();
				blockfraction<bfbits, bt> f = v.significant();

				ostr << i << ','
					<< to_binary(v) << ','
					<< s << ','
					<< scale << ','
					<< std::right << to_binary(f) << ','
					<< v
					<< '\n';
			}
			ostr << std::endl;
		}
		else {
			ostr << "Generate table for a " << typeid(v).name() << " in TXT format" << std::endl;

			const size_t index_column = 5;
			const size_t bin_column = 16;
			const size_t sign_column = 8;
			const size_t scale_column = 8;
			const size_t fraction_column = 16;
			const size_t value_column = 30;

			ostr << std::setw(index_column) << " # "
				<< std::setw(bin_column) << "Binary"
				<< std::setw(sign_column) << "sign"
				<< std::setw(scale_column) << "scale"
				<< std::setw(fraction_column) << "fraction"
				<< std::setw(value_column) << "value"
				<< std::endl;
			for (size_t i = 0; i < NR_VALUES; i++) {
				v.setbits(i + NR_VALUES);
				bool s = v.sign();
				int scale = v.scale();
				blockfraction<bfbits, bt> f = v.significant();

				ostr << std::setw(4) << i << ": "
					<< std::setw(bin_column) << to_binary(v)
					<< std::setw(sign_column) << s
					<< std::setw(scale_column) << scale
					<< std::setw(fraction_column) << std::right << to_binary(f, true)
					<< std::setw(value_column) << v
					<< std::endl;
			}
		}
	}

}  // namespace sw::universal

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	// Usage: tables_bfloats [-csv]
	bool csv = false;
	if (argc == 2) {
		if (std::string(argv[1]) == std::string("-csv")) csv = true;
	}
	cout << "Generate value tables for blocktriple configurations" << endl;

	//bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "conversion: ";

#if MANUAL_TESTING

	GenerateTable < blocktriple<4, uint8_t> >(cout, csv);

	blocktriple<8, uint8_t> a;
	a = 1.5f;
	cout << "float  : " << to_binary(1.5f, true) << endl;
	cout << "a : " << to_triple(a) << endl;
	a = 1.5;
	cout << "double : " << to_binary(1.5, true) << endl;
	cout << "a : " << to_triple(a) << endl;

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING

	cout << "blocktriple conversion validation" << endl;


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
