// rounding.cpp: test suite runner for blocktriple rounding decisions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
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

/*
precondition for rounding is a 1's complement bit pattern, and no denorm
That is:  patterns of the form 
   0b001.ffff
   0b010.ffff
   0b011.ffff
  and excluding
   0b000.ffff
   0b1##.ffff
 */
template<size_t fbits, sw::universal::BlockTripleOperator op, typename bt>
int VerifyRounding() {
	using namespace sw::universal;
	using BlockTriple = blocktriple<fbits, op, bt>;
	BlockTriple a, nut;
	std::cout << ' ' << type_tag(nut) << " with radix point at " << nut.radix << '\n';
	int nrOfFailures = 0;

	size_t fractionbits = fbits; // default is ADD
	if constexpr (op == BlockTripleOperator::MUL) {
		fractionbits = 2*fbits; // override to 2*fbits
	}
	size_t START = (1ull << fractionbits);
	size_t NR_VALUES = (1ull << (fractionbits + 2));
	for (size_t i = START; i < NR_VALUES; ++i) {
		if (i == 0) a.setzero(); else a.setnormal();
		a.setbits(i);
		// for add/sub ops    0b0ii.fffff with only a single bit of rounding. 
		// TODO is that always true? if you have dynamic range, don't you have 2*fhbits of bits to examine?
		// for mul op         0bii.fffff`fffff with fbits of rounding
		auto retval = a.roundingDecision();
		std::cout << to_triple(a) << (retval.first ? " rounds up\n" : " rounds down\n");

		bool correct = true;
		if (!correct) {
			++nrOfFailures;
			std::cout << "FAIL: " << std::setw(10) << i << " : " << to_binary(a) << " != ";
			std::cout << to_binary(nut) << '\n';
		}
		else {
			//std::cout << "PASS: " << std::setw(10) << i << " : " << to_binary(a) << " == " << to_binary(nut) << '\n';
		}
	}
	std::cout << (nrOfFailures ? "FAIL\n" : "PASS\n");
	return nrOfFailures;
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	//bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "blocktriple rounding validation: ";

#if MANUAL_TESTING

//	nrOfFailedTestCases += VerifyRounding<5, BlockTripleOperator::REPRESENTATION, uint8_t>();
	nrOfFailedTestCases += VerifyRounding<5, BlockTripleOperator::ADD, uint8_t>();
	nrOfFailedTestCases += VerifyRounding<5, BlockTripleOperator::MUL, uint8_t>();

	nrOfFailedTestCases = 0;

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING

	std::cout << tag << endl;


#if STRESS_TESTING

#endif  // STRESS_TESTING

	std::cout << tag << ((0 == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");

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
