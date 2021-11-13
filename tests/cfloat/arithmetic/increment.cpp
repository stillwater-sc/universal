// increment.cpp: test suite runner for increment operator on classic floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

#define MANUAL_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat<> increment operator validation";
	std::string test_tag    = test_tag;
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

/*
	{
		using Cfloat = cfloat<4, 1, uint8_t>;
		std::vector<Cfloat> set;
		GenerateOrderedCfloatSet<Cfloat>(set);
		for (auto v : set) {
			std::cout << to_binary(v) << " : " << v << '\n';
		}
	}
		{
		using Cfloat = cfloat<17,2, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		Cfloat c;
		c.setbits(0x0FEFD);
		for (int i = 0; i < 10; ++i) {
			std::cout << to_binary(c) << " : " << c++ << '\n';
		}
	}

*/

	{	
		constexpr bool hasSubnormals = true;
		constexpr bool hasSupernormals = true;
		constexpr bool isSaturating = true;
		using Cfloat = cfloat<17, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyCfloatIncrement< Cfloat >(true), "cfloat<17,3,uint8_t,subnormals,supernormals,!saturating>", test_tag);
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

	// normal encoding only
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<8, 2, uint8_t, false, false, false> >(reportTestCases), type_tag(cfloat<8, 2, uint8_t, false, false, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<9, 2, uint8_t, false, false, false> >(reportTestCases), type_tag(cfloat<9, 2, uint8_t, false, false, false>()), test_tag);


	// subnormal + normal
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<8, 2, uint8_t, true, false, false> >(reportTestCases), type_tag(cfloat<8, 2, uint8_t, true, false, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<9, 2, uint8_t, true, false, false> >(reportTestCases), type_tag(cfloat<9, 2, uint8_t, true, false, false>()), test_tag);


	// normal + supernormal
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<8, 2, uint8_t, false, true, false> >(reportTestCases), type_tag(cfloat<8, 2, uint8_t, false, true, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<9, 2, uint8_t, false, true, false> >(reportTestCases), type_tag(cfloat<9, 2, uint8_t, false, true, false>()), test_tag);


	// subnormal + normal + supernormal

	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement< cfloat<4, 1, uint8_t, true, true, false> >(reportTestCases), type_tag(cfloat<4, 1, uint8_t, true, true, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<8, 2, uint8_t, true, true, false> >(reportTestCases), type_tag(cfloat<8, 2, uint8_t, true, true, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<9, 2, uint8_t, true, true, false> >(reportTestCases), type_tag(cfloat<9, 2, uint8_t, true, true, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<10, 3, uint8_t, true, true, false> >(reportTestCases), type_tag(cfloat<10, 3, uint8_t, true, true, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<17, 3, uint8_t, true, true, false> >(reportTestCases), type_tag(cfloat<17, 4, uint8_t, true, true, false>()), test_tag);

	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< cfloat<32, 8, uint32_t, true, true, false> >(reportTestCases), type_tag(cfloat<32, 8, uint32_t, true, true, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< cfloat<64, 11, uint32_t, true, true, false> >(reportTestCases), type_tag(cfloat<64, 11, uint32_t, true, true, false>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< cfloat<128, 15, uint32_t, true, true, false> >(reportTestCases), type_tag(cfloat<128, 15, uint32_t, true, true, false>()), test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
