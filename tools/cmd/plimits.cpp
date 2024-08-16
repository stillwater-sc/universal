// plimits.cpp: cli to show the numeric_limits<> of the standard posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/posit/posit.hpp>

template<size_t nbits, size_t es>
void ReportNumericLimitsOfPosit() {
	using namespace sw::universal;

	std::stringstream ss;
	ss << "numeric_limits< sw::universal::posit<" << nbits << ", " << es << "> >::";
	std::string posit_tag = ss.str();

	std::cout << "Numeric limits for posit< " << nbits << ", " << es << ">\n";
	std::cout << posit_tag << "min()             : " << std::numeric_limits< posit<nbits, es> >::min() << '\n';
	std::cout << posit_tag << "max()             : " << std::numeric_limits< posit<nbits, es> >::max() << '\n';
	std::cout << posit_tag << "lowest()          : " << std::numeric_limits< posit<nbits, es> >::lowest() << '\n';
	std::cout << posit_tag << "epsilon()         : " << std::numeric_limits< posit<nbits, es> >::epsilon() << '\n';

	std::cout << posit_tag << "digits            : " << std::numeric_limits< posit<nbits, es> >::digits << '\n';
	std::cout << posit_tag << "digits10          : " << std::numeric_limits< posit<nbits, es> >::digits10 << '\n';
	std::cout << posit_tag << "max_digits10      : " << std::numeric_limits< posit<nbits, es> >::max_digits10 << '\n';
	std::cout << posit_tag << "is_signed         : " << std::numeric_limits< posit<nbits, es> >::is_signed << '\n';
	std::cout << posit_tag << "is_integer        : " << std::numeric_limits< posit<nbits, es> >::is_integer << '\n';
	std::cout << posit_tag << "is_exact          : " << std::numeric_limits< posit<nbits, es> >::is_exact << '\n';

	std::cout << posit_tag << "min_exponent      : " << std::numeric_limits< posit<nbits, es> >::min_exponent << '\n';
	std::cout << posit_tag << "min_exponent10    : " << std::numeric_limits< posit<nbits, es> >::min_exponent10 << '\n';
	std::cout << posit_tag << "max_exponent      : " << std::numeric_limits< posit<nbits, es> >::max_exponent << '\n';
	std::cout << posit_tag << "max_exponent10    : " << std::numeric_limits< posit<nbits, es> >::max_exponent10 << '\n';
	std::cout << posit_tag << "has_infinity      : " << std::numeric_limits< posit<nbits, es> >::has_infinity << '\n';
	std::cout << posit_tag << "has_quiet_NaN     : " << std::numeric_limits< posit<nbits, es> >::has_quiet_NaN << '\n';
	std::cout << posit_tag << "has_signaling_NaN : " << std::numeric_limits< posit<nbits, es> >::has_signaling_NaN << '\n';
	std::cout << posit_tag << "has_denorm        : " << std::numeric_limits< posit<nbits, es> >::has_denorm << '\n';
	std::cout << posit_tag << "has_denorm_loss   : " << std::numeric_limits< posit<nbits, es> >::has_denorm_loss << '\n';

	std::cout << posit_tag << "is_iec559         : " << std::numeric_limits< posit<nbits, es> >::is_iec559 << '\n';
	std::cout << posit_tag << "is_bounded        : " << std::numeric_limits< posit<nbits, es> >::is_bounded << '\n';
	std::cout << posit_tag << "is_modulo         : " << std::numeric_limits< posit<nbits, es> >::is_modulo << '\n';
	std::cout << posit_tag << "traps             : " << std::numeric_limits< posit<nbits, es> >::traps << '\n';
	std::cout << posit_tag << "tinyness_before   : " << std::numeric_limits< posit<nbits, es> >::tinyness_before << '\n';
	std::cout << posit_tag << "round_style       : " << std::numeric_limits< posit<nbits, es> >::round_style << '\n';
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc == 1) std::cout << argv[0] << ": numeric_limits<> of standard posits\n";
	// numeric_limits of standard posits
	ReportNumericLimitsOfPosit<8, 0>();
	ReportNumericLimitsOfPosit<16, 1>();
	ReportNumericLimitsOfPosit<32, 2>();
	ReportNumericLimitsOfPosit<64, 3>();
	std::cout << ">>>>>>>>>>>>>>>>>> posit<128,4> does not render correctly due to limits of native floating point types\n";
	ReportNumericLimitsOfPosit<128, 4>();
	std::cout << ">>>>>>>>>>>>>>>>>> posit<256,5> does not render correctly due to limits of native floating point types\n";
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
