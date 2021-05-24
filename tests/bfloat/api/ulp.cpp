// ulp.cpp: application programming interface utilities tests for bfloat number system
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the bfloat template environment
// first: enable general or specialized configurations
#define BFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define BFLOAT_THROW_ARITHMETIC_EXCEPTION 0

#include <universal/number/bfloat/bfloat.hpp>
#include <universal/number/bfloat/manipulators.hpp>  // hex_print and the like
#include <universal/verification/test_suite_arithmetic.hpp>

namespace sw::universal {
	template<size_t nbits, size_t es, typename bt>
	void GenerateUlpsInRange(const bfloat<nbits, es, bt>& begin, const bfloat<nbits, es, bt>& end) {
		/*
		bfloat<nbits, es, bt> current(begin);
		while (current < end) {
			bfloat<nbits, es, bt> bulp = ulp(current);
			std::cout << to_binary(bulp, true) << " : " << bulp << '\n';
		}
		*/
		bfloat<nbits, es, bt> current(begin);
		while (current != end) { // != is simpler than <
			bfloat<nbits, es, bt> prev(current++);
			bfloat<nbits, es, bt> bulp = current - prev;
			std::cout << to_binary(prev, true) << " : " << to_binary(bulp, true) << " : " << bulp << '\n';
		}
	}
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;

	std::cout << "bfloat<> Unit in Last Position tests" << std::endl;

#if MANUAL_TESTING

	bfloat<8,2,uint8_t> begin(0), end;
	end.setbits(0x7Fu);
	GenerateUlpsInRange(begin, end);

#else // !MANUAL_TESTING



#endif // MANUAL_TESTING

	std::cout << "\nBFLOAT Unit in Last Position test suite           : " << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");
	//bool bReportIndividualTestCases = false;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
