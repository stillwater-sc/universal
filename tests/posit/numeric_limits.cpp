// numeric_limits.cpp: tests of the numeric_limits specialization for posits
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/posit/posit>
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "../utils/posit_test_helpers.hpp"

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	streamsize precision = cout.precision();

	numeric_limits_table<int16_t>(cout);
	numeric_limits_table<float>(cout);
	numeric_limits_table<posit<32, 2>>(cout);

	cout << minmax_range<float>() << endl;
	cout << minmax_range<posit<32, 2>>() << endl;

	cout << dynamic_range<float>() << endl;
	cout << dynamic_range<posit<32, 2>>() << endl;

	cout << symmetry<float>() << endl;
	cout << symmetry<posit<32, 2>>() << endl;

	using Float = float;
	using Posit = sw::unum::posit<32, 2>;
	compare_numeric_limits<Float, Posit>(cout);

	cout << endl;
	cout << "std::numeric_limits<T>::min():\n"
		<< "\tfloat: " << std::numeric_limits<float>::min()
		<< " or " << std::hexfloat << std::numeric_limits<float>::min() << '\n'
		<< "\tdouble: " << std::defaultfloat << std::numeric_limits<double>::min()
		<< " or " << std::hexfloat << std::numeric_limits<double>::min() << '\n';
	cout << "std::numeric_limits<T>::lowest():\n"
		<< "\tfloat: " << std::defaultfloat << std::numeric_limits<float>::lowest()
		<< " or " << std::hexfloat << std::numeric_limits<float>::lowest() << '\n'
		<< "\tdouble: " << std::defaultfloat << std::numeric_limits<double>::lowest()
		<< " or " << std::hexfloat << std::numeric_limits<double>::lowest() << '\n';
	cout << "std::numeric_limits<T>::max():\n"
		<< "\tfloat: " << std::defaultfloat << std::numeric_limits<float>::max()
		<< " or " << std::hexfloat << std::numeric_limits<float>::max() << '\n'
		<< "\tdouble: " << std::defaultfloat << std::numeric_limits<double>::max()
		<< " or " << std::hexfloat << std::numeric_limits<double>::max() << '\n';

	cout << setprecision(precision);

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
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
