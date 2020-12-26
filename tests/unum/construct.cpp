// construct.cpp: functional tests to construct arbitrary configuration unums
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <universal/unum/unum>

template<typename Real>
void ReportNumberTraits(std::ostream& ostr) {
	using namespace std;
	using namespace sw::universal;
	ostr << "Real type          : " << typeid(Real).name() << '\n';
	ostr << "minimum exponent   : " << numeric_limits<Real>::min_exponent << '\n';
	ostr << "maximum exponent   : " << numeric_limits<Real>::max_exponent << '\n';
	ostr << "radix              : " << numeric_limits<Real>::radix << '\n';
	ostr << "radix digits       : " << numeric_limits<Real>::digits << '\n';
	ostr << "minimum value      : " << numeric_limits<Real>::min() << '\n';
	ostr << "maximum value      : " << numeric_limits<Real>::max() << '\n';
	ostr << "epsilon value      : " << numeric_limits<Real>::epsilon() << '\n';
	ostr << "max rounding error : " << numeric_limits<Real>::round_error() << '\n';
	ostr << "infinite           : " << numeric_limits<Real>::infinity() << '\n';
	ostr << "quiet NaN          : " << numeric_limits<Real>::quiet_NaN() << '\n';
	ostr << "signalling NaN     : " << numeric_limits<Real>::signaling_NaN() << "\n\n";
}

int main(int argc, char* argv[]) 
try {
	using namespace std;
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	cout << "Constructing flexible configuration unums" << endl;

	unum<2, 2> u2_2;
	cout << typeid(unum<2,2>).name() << " : " << u2_2 << endl;

	ReportNumberTraits<unum<2, 2>>(cout);
	ReportNumberTraits<unum<2, 3>>(cout);
	ReportNumberTraits<unum<2, 4>>(cout);
	ReportNumberTraits<unum<3, 2>>(cout);
	ReportNumberTraits<unum<3, 3>>(cout);
	ReportNumberTraits<unum<3, 4>>(cout);

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
