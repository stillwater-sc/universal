// rounding.cpp: functional tests for blocksignificant rounding
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <iostream>
#include <iomanip>

#include <universal/native/ieee754.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocksignificant/blocksignificant.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_reporters.hpp> // ReportBinaryArithmeticError

// enumerate all rounding cases for an blocksignificant<nbits,BlockType> configuration
template<typename blocksignificantConfiguration>
int VerifyRounding(bool bReportIndividualTestCases) {
	constexpr size_t nbits = blocksignificantConfiguration::nbits;
	using BlockType = typename blocksignificantConfiguration::BlockType;
	constexpr sw::universal::BitEncoding encoding = blocksignificantConfiguration::encoding;

	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	using namespace sw::universal;
	
//	std::cout << endl;
//	std::cout << "blocksignificant<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

	// two's complement blocksignificants will have the form: 0ii.fffff
	// 
	int nrOfFailedTests = 0;

	blocksignificant<nbits, BlockType, encoding> a;
	constexpr size_t nrBlocks = blockbinary<nbits, BlockType>::nrBlocks;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		a.setradix(5); 
		// the LSB that we need to round can be anywhere in the fraction
		// let's pick one that has explicit bits to use for the rounding
		size_t targetLsb = 4;
		bool roundUp = a.roundingDirection(targetLsb);
		std::cout << to_binary(a) << " : round " << (roundUp ? "up " : "dn ") << '\n';
	}
//	std::cout << endl;
	return nrOfFailedTests;
}

int main()
try {
	using namespace sw::universal;
	
	int nrOfFailedTestCases = 0;
	std::string tag = "blocksignificant rounding";
	constexpr BitEncoding twos = BitEncoding::Twos;

	std::cout << tag << '\n';

	// Map out the full rounding truth table
			//  ... lsb | guard  round sticky   round
			//       x     0       x     x       down
			//       0     1       0     0       down  round to even
			//       1     1       0     0        up   round to even
			//       x     1       0     1        up
	{
		blocksignificant<10, uint32_t, twos> a;
		// test rounding of 0b00'0lgr'ssss
		//                        |          position of the lsb
		// lsb is 6
		/*
		*         lgr'ssss
			0b00'0000'0000 round down
			0b00'0000'0001 round down
			0b00'0001'0000 round down
			0b00'0001'0001 round down
			0b00'0010'0000 round down   <-- rounding to even on tie
			0b00'0010'0001 round up
			0b00'0011'0000 round up
			0b00'0011'0001 round up
			0b00'0100'0000 round down
			0b00'0100'0001 round down
			0b00'0101'0000 round down
			0b00'0101'0001 round down
			0b00'0110'0000 round up     <-- rounding to even on tie
			0b00'0110'0001 round up
			0b00'0111'0000 round up
			0b00'0111'0001 round up
		*/
		for (size_t i = 0; i < 8; ++i) {
			size_t bits = (i << 4);
			a.setbits(bits);
			std::cout <<  to_binary(a, true) << " round " << (a.roundingDirection(6) ? "up" : "down") << '\n';
			bits |= 0x1;
			a.setbits(bits);
			std::cout << to_binary(a, true) << " round " << (a.roundingDirection(6) ? "up" : "down") << '\n';
		}
	}

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
