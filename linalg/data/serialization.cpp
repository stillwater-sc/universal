// serialization.cpp: test suite for serialization functions for data exchange
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/math/math_constants.hpp>

void ReportNumberSystemFormats() {
	using namespace sw::universal;

	{
		constexpr unsigned nbits = 40;
		integer<nbits, uint32_t, IntegerNumberType::NaturalNumber> a{123456789};
		integer<nbits, uint32_t, IntegerNumberType::WholeNumber> b{123456789};
		integer<nbits, uint32_t, IntegerNumberType::IntegerNumber> c{-123456789};
		ReportFormats(a);
		ReportFormats(b);
		ReportFormats(c);
	}
	{
		fixpnt<40, 32, Modulo, uint32_t> a(m_pi);
		ReportFormats(a);
		ReportFormats(-a);
		fixpnt<40, 32, Saturate, uint32_t> b(m_pi);
		ReportFormats(b);

	}
	{
		cfloat<64, 11, uint32_t, true, false, false> a(m_pi);
		ReportFormats(a);
	}
	{
		posit<64, 5> a(m_pi);
		ReportFormats(a);
	}
	{
		lns<64, 32> a(m_pi);
		ReportFormats(a);
	}
	{
		dbns<8,3,uint8_t> a(m_pi);
		ReportFormats(a);
		dbns<10, 4, uint8_t> b(m_pi);
		ReportFormats(b);
		dbns<12, 5, uint8_t> c(m_pi);
		ReportFormats(c);
	}
}

template<typename Scalar>
void save(std::ostream& ostr, const sw::universal::blas::vector<Scalar>& v) {
	ostr << sw::universal::type_tag(Scalar()) << '\n';
	ostr << sw::universal::type_field(Scalar()) << '\n';
	ostr << "shape(" << v.size() << ", 1)\n";
	unsigned i = 0;
	for (auto e : v) {
		ostr << sw::universal::to_hex(e) << ' ';
		if ((++i % 16) == 0) ostr << '\n';
	}
	ostr << std::endl;
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "serialization";
	std::string test_tag = "save/restore";
	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// manual test cases
	//nrOfFailedTestCases += ReportTestResult(VerifyCompress<quarter>(reportTestCases), "compress to quarter precision", "quarter precision");

	ReportNumberSystemFormats();
	return 0;

	unsigned N = 32;
	sw::universal::blas::vector<double> x(N), y(N);
	double zeroMean = 0.0;
	double variance = 0.1;
	gaussian_random(x, zeroMean, variance);

	{
		sw::universal::blas::vector<lns<8, 2, uint8_t>> v(N);
		v = x;
		save(std::cout, v);
	}
	{
		sw::universal::blas::vector<lns<12, 4, uint8_t>> v(N);
		v = x;
		save(std::cout, v);
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	//nrOfFailedTestCases += ReportTestResult(VerifyCompress<lns<8,4>>(reportTestCases), "compress to lns<8,4>", "lns<8,4>");
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught universal internal exception: " << err.what() << std::endl;
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
