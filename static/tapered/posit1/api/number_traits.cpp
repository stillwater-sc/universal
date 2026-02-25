// numeric_traits.cpp: test suite runner of the numeric_limits specialization for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/native/ieee754.hpp>
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

int main()
try {
	using namespace sw::universal;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::streamsize precision = std::cout.precision();

	numberTraits<short>(std::cout);
	numberTraits<unsigned>(std::cout);
	numberTraits<float>(std::cout);
	numberTraits<posit<32, 2> >(std::cout);

	std::cout << minmax_range<float>() << '\n';
	std::cout << minmax_range< posit<32, 2> >() << '\n';

	std::cout << dynamic_range<float>() << '\n';
	std::cout << dynamic_range< posit<32, 2> >() << '\n';

	std::cout << symmetry_range<float>() << '\n';
	std::cout << symmetry_range< posit<32, 2> >() << '\n';

	using Float = float;
	using Posit = sw::universal::posit<32, 2>;
	compareNumberTraits<Float, Posit>(std::cout);

	std::cout << std::endl;
	std::cout << "std::numeric_limits<T>::min():\n"
		<< "\tfloat: " << std::numeric_limits<float>::min()
		<< " or " << std::hexfloat << std::numeric_limits<float>::min() << '\n'
		<< "\tdouble: " << std::defaultfloat << std::numeric_limits<double>::min()
		<< " or " << std::hexfloat << std::numeric_limits<double>::min() << '\n';
	std::cout << "std::numeric_limits<T>::lowest():\n"
		<< "\tfloat: " << std::defaultfloat << std::numeric_limits<float>::lowest()
		<< " or " << std::hexfloat << std::numeric_limits<float>::lowest() << '\n'
		<< "\tdouble: " << std::defaultfloat << std::numeric_limits<double>::lowest()
		<< " or " << std::hexfloat << std::numeric_limits<double>::lowest() << '\n';
	std::cout << "std::numeric_limits<T>::max():\n"
		<< "\tfloat: " << std::defaultfloat << std::numeric_limits<float>::max()
		<< " or " << std::hexfloat << std::numeric_limits<float>::max() << '\n'
		<< "\tdouble: " << std::defaultfloat << std::numeric_limits<double>::max()
		<< " or " << std::hexfloat << std::numeric_limits<double>::max() << '\n';

	std::cout << std::setprecision(precision);

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
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
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
