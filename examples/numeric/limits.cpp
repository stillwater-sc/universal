// limits.cpp example program comparing numeric_limits of different number systems
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

// select the number systems we would like to compare
#include <universal/integer/integer>
#include <universal/fixpnt/fixpnt>
#include <universal/areal/areal>
#include <universal/posit/posit>
#include <universal/lns/lns>

constexpr long double pi     = 3.14159265358979323846;
constexpr long double e      = 2.71828182845904523536;
constexpr long double log_2e = 1.44269504088896340736;

template<typename Real>
void ReportNumberTraits(std::ostream& ostr) {
	using namespace std;
	using namespace sw::unum;
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
int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	cout << "numeric_limits for different number systems " << endl;

	using int32    = integer<32>;
	using fixpnt32 = fixpnt<32,16>;
	using posit32  = posit<32,2>;
	using areal32  = areal<32,8>;
	using lns32    = lns<32>;

	// report on precision and dynamic range of the number system
	ReportNumberTraits<float>(cout);
	ReportNumberTraits<int32>(cout);
	ReportNumberTraits<fixpnt32>(cout);
	ReportNumberTraits<posit32>(cout);
	ReportNumberTraits<areal32>(cout);
	ReportNumberTraits<lns32>(cout);

	cout << endl;

	return EXIT_SUCCESS;
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
