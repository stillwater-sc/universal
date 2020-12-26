// plimits.cpp: cli to show the numeric_limits<> of the standard posits
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/posit/posit>

template<size_t nbits, size_t es>
void ReportNumericLimitsOfPosit() {
	using namespace std;
	using namespace sw::universal;

	stringstream ss;
	ss << "numeric_limits< sw::universal::posit<" << nbits << ", " << es << "> >::";
	string posit_tag = ss.str();

	cout << "Numeric limits for posit< " << nbits << ", " << es << ">\n";
	cout << posit_tag << "min()             : " << numeric_limits< posit<nbits, es> >::min() << '\n';
	cout << posit_tag << "max()             : " << numeric_limits< posit<nbits, es> >::max() << '\n';
	cout << posit_tag << "lowest()          : " << numeric_limits< posit<nbits, es> >::lowest() << '\n';
	cout << posit_tag << "epsilon()         : " << numeric_limits< posit<nbits, es> >::epsilon() << '\n';

	cout << posit_tag << "digits            : " << numeric_limits< posit<nbits, es> >::digits << '\n';
	cout << posit_tag << "digits10          : " << numeric_limits< posit<nbits, es> >::digits10 << '\n';
	cout << posit_tag << "max_digits10      : " << numeric_limits< posit<nbits, es> >::max_digits10 << '\n';
	cout << posit_tag << "is_signed         : " << numeric_limits< posit<nbits, es> >::is_signed << '\n';
	cout << posit_tag << "is_integer        : " << numeric_limits< posit<nbits, es> >::is_integer << '\n';
	cout << posit_tag << "is_exact          : " << numeric_limits< posit<nbits, es> >::is_exact << '\n';

	cout << posit_tag << "min_exponent      : " << numeric_limits< posit<nbits, es> >::min_exponent << '\n';
	cout << posit_tag << "min_exponent10    : " << numeric_limits< posit<nbits, es> >::min_exponent10 << '\n';
	cout << posit_tag << "max_exponent      : " << numeric_limits< posit<nbits, es> >::max_exponent << '\n';
	cout << posit_tag << "max_exponent10    : " << numeric_limits< posit<nbits, es> >::max_exponent10 << '\n';
	cout << posit_tag << "has_infinity      : " << numeric_limits< posit<nbits, es> >::has_infinity << '\n';
	cout << posit_tag << "has_quiet_NaN     : " << numeric_limits< posit<nbits, es> >::has_quiet_NaN << '\n';
	cout << posit_tag << "has_signaling_NaN : " << numeric_limits< posit<nbits, es> >::has_signaling_NaN << '\n';
	cout << posit_tag << "has_denorm        : " << numeric_limits< posit<nbits, es> >::has_denorm << '\n';
	cout << posit_tag << "has_denorm_loss   : " << numeric_limits< posit<nbits, es> >::has_denorm_loss << '\n';

	cout << posit_tag << "is_iec559         : " << numeric_limits< posit<nbits, es> >::is_iec559 << '\n';
	cout << posit_tag << "is_bounded        : " << numeric_limits< posit<nbits, es> >::is_bounded << '\n';
	cout << posit_tag << "is_modulo         : " << numeric_limits< posit<nbits, es> >::is_modulo << '\n';
	cout << posit_tag << "traps             : " << numeric_limits< posit<nbits, es> >::traps << '\n';
	cout << posit_tag << "tinyness_before   : " << numeric_limits< posit<nbits, es> >::tinyness_before << '\n';
	cout << posit_tag << "round_style       : " << numeric_limits< posit<nbits, es> >::round_style << '\n';
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc == 1) cout << argv[0] << ": numeric_limits<> of standard posits\n";
	// numeric_limits of standard posits
	ReportNumericLimitsOfPosit<8, 0>();
	ReportNumericLimitsOfPosit<16, 1>();
	ReportNumericLimitsOfPosit<32, 2>();
	ReportNumericLimitsOfPosit<64, 3>();
	cout << ">>>>>>>>>>>>>>>>>> posit<128,4> does not render correctly due to limits of native floating point types" << endl;
	ReportNumericLimitsOfPosit<128, 4>();
	cout << ">>>>>>>>>>>>>>>>>> posit<256,5> does not render correctly due to limits of native floating point types" << endl;
	ReportNumericLimitsOfPosit<256, 5>();

	return EXIT_SUCCESS;
}
catch (const char* const msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
