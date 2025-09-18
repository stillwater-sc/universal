// tables.cpp: test suite runner for blocktriple value enumeration
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

// minimum set of include files to reflect source code dependencies
#include <universal/native/integers.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_suite.hpp>

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
		constexpr size_t  fbits = TestType::fbits;  // fbits of a blocktriple represent the number of fraction bits of the representation
		constexpr size_t bfbits = TestType::bfbits; // bfbits represents the number of bits in the blocksignificant that is used for arithmetic
		using bt = typename TestType::BlockType;

		constexpr size_t NR_VALUES = (1 << fbits);
		TestType v;

		// we are going to enumerate the blocktriple's fraction bits.
		// By design, a blocktriple is a normalized floating point number
		// with the leading bit explicitely set to '1'.

		// set blocktriple to a normal encoding
		v.setnormal();
		v.setsign(false);
		if (csvFormat) {
			ostr << "\"Generate Lookup table for a " << typeid(v).name() << " in CSV format\"" << std::endl;
			ostr << "#, Binary, sign, scale, exponent, fraction, value\n";
			for (size_t i = 0; i < NR_VALUES; i++) {
				v.setbits(i + NR_VALUES);
				bool s = v.sign();
				int scale = v.scale();
				blocksignificand<bfbits, bt> f = v.significand();

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

			constexpr size_t index_column = 5;
			constexpr size_t bin_column = 16;
			constexpr size_t sign_column = 8;
			constexpr size_t scale_column = 8;
			constexpr size_t fraction_column = 16;
			constexpr size_t value_column = 30;

			constexpr int scaleRange[] = { -3, -2, -1, 0, 1, 2, 3 };

			ostr << std::setw(index_column) << " # "
				<< std::setw(bin_column) << "Binary"
				<< std::setw(sign_column) << "sign"
				<< std::setw(scale_column) << "scale"
				<< std::setw(fraction_column) << "fraction"
				<< std::setw(value_column) << "value"
				<< std::endl;
			size_t cnt{ 0 };
			for (int sign = 0; sign <= 1; ++sign) {
				v.setsign(sign == 1);
				for (int scale : scaleRange) {
					if (sign) v.setscale(-scale); else v.setscale(scale);  // to have the same progression as posits
					for (size_t i = 0; i < NR_VALUES; i++) {
						if (sign) v.setbits(2 * NR_VALUES - 1 - i); else v.setbits(i + NR_VALUES);  // to have the same progression as posits
						blocksignificand<bfbits, bt> f = v.significand();

						ostr << std::setw(4) << ++cnt << ": "
							<< std::setw(bin_column) << to_binary(v)
							<< std::setw(sign_column) << v.sign()
							<< std::setw(scale_column) << v.scale()
							<< std::setw(fraction_column) << std::right << to_binary(f, true)
							<< std::setw(value_column) << v
							<< std::endl;
					}
				}
			}
		}
	}

}  // namespace sw::universal

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// Usage: tables <txt|csv>
	std::string test_suite  = "blocktriple table generator utility";
	std::string test_tag    = "bt_table";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	bool csv = false;
	if (argc != 2) {
		// this test should be ignored during regressions
		ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
		return EXIT_SUCCESS;
	}

	if (std::string(argv[1]) == std::string("csv")) csv = true;

	std::string tag = "Generate value tables for blocktriple configurations";
	std::cout << tag << '\n';

	GenerateTable < blocktriple<3, BlockTripleOperator::ADD, uint8_t> >(std::cout, csv);
	GenerateTable < blocktriple<4, BlockTripleOperator::ADD, uint8_t> >(std::cout, csv);
	GenerateTable < blocktriple<5, BlockTripleOperator::ADD, uint8_t> >(std::cout, csv);   // a fascimile to a quarter precision IEEE float<8,2>

	GenerateTable < blocktriple<3, BlockTripleOperator::MUL, uint8_t> >(std::cout, csv);
	GenerateTable < blocktriple<4, BlockTripleOperator::MUL, uint8_t> >(std::cout, csv);
	GenerateTable < blocktriple<5, BlockTripleOperator::MUL, uint8_t> >(std::cout, csv);   // a fascimile to a quarter precision IEEE float<8,2>

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
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
