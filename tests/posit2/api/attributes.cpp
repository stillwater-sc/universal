// attributes.cpp: attribute tests for arbitrary configuration posit types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
// first: enable general or specialized configurations
#define POSIT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// third: enable support for native literals in logic and arithmetic operations
#define POSIT_ENABLE_LITERALS 1
// fourth: enable/disable error-free serialization I/O
#define POSIT_ERROR_FREE_IO_FORMAT 0
// minimum set of include files to reflect source code dependencies
#include <universal/number/posit2/posit.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "generalized posit attribute functions";
	std::string test_tag    = "attributes";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// posit attribute functions

	// useed, minpos and maxpos scale
	// what is special with these functions is that they are independent of a posit
	// but associate through their template parameterizations
	{
		std::cout << "useed values for full articulated standard posits\n";
		std::cout << "es\tuseed scale\tuseed value\n";
		std::cout << ES_IS_0 << '\t' << std::setw(5) << useed_scale<ES_IS_0>() << '\t' << std::setw(15) << useed<ES_IS_0>() << '\n';
		std::cout << ES_IS_1 << '\t' << std::setw(5) << useed_scale<ES_IS_1>() << '\t' << std::setw(15) << useed<ES_IS_1>() << '\n';
		std::cout << ES_IS_2 << '\t' << std::setw(5) << useed_scale<ES_IS_2>() << '\t' << std::setw(15) << useed<ES_IS_2>() << '\n';
		std::cout << ES_IS_3 << '\t' << std::setw(5) << useed_scale<ES_IS_3>() << '\t' << std::setw(15) << useed<ES_IS_3>() << '\n';
		std::cout << ES_IS_4 << '\t' << std::setw(5) << useed_scale<ES_IS_4>() << '\t' << std::setw(15) << useed<ES_IS_4>() << '\n';
		std::cout << ES_IS_5 << '\t' << std::setw(5) << useed_scale<ES_IS_5>() << '\t' << std::setw(15) << useed<ES_IS_5>() << '\n';
		std::cout << '\n';
	}

	{
		constexpr size_t nbits = 16;
		constexpr size_t es = 2;
		using bt = std::uint16_t;
		constexpr posit<nbits, es, bt> maxpos(SpecificValue::maxpos);
		constexpr posit<nbits, es, bt> minpos(SpecificValue::minpos);
		constexpr posit<nbits, es, bt> zero(SpecificValue::zero);
		constexpr posit<nbits, es, bt> minneg(SpecificValue::minneg);
		constexpr posit<nbits, es, bt> maxneg(SpecificValue::maxneg);
		std::cout << "minpos patterns for full articulated standard posits\n";
		std::cout << "minpos : " << to_binary(minpos) << '\t' << minpos_scale<nbits, es>() << '\n';
		std::cout << "minneg : " << to_binary(minneg) << '\n';

		std::cout << "maxpos patterns for full articulated standard posits\n";
		std::cout << "maxpos : " << to_binary(maxpos) << '\t' << maxpos_scale<nbits, es>() << '\n';
		std::cout << "maxneg : " << to_binary(maxneg) << '\n';
		std::cout << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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
