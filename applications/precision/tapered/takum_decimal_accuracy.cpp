// takum_decimal_accuracy.cpp: compare decimal accuracy of takum, posit, cfloat, and lns
//
// The core result of the takum paper (Hunhold, 2024, arXiv:2404.18603) is that
// takums provide more precision than posits at the extremes of the number line
// (far from 1.0) while remaining competitive near 1.0.  This demo visualizes
// that by scanning across magnitudes and reporting how many correct decimal
// digits each 16-bit format preserves for a set of test values.
//
// For each base magnitude, we sample several values in [base, 2*base), convert
// through each number format, and measure the average number of correct decimal
// digits (the "decimal accuracy").  The result is a table showing how precision
// tapers differently for each format.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/takum/takum.hpp>

namespace sw { namespace universal {

// Compute the number of correct decimal digits when 'approx' represents 'exact'.
// Returns 0 when the value cannot be represented (maps to 0, inf, or NaN).
inline double decimalAccuracy(double exact, double approx) {
	if (exact == 0.0) return (approx == 0.0) ? 16.0 : 0.0;
	if (std::isinf(approx) || std::isnan(approx) || approx == 0.0) return 0.0;
	double relErr = std::abs((approx - exact) / exact);
	if (relErr == 0.0) return 16.0; // perfect
	double digits = -std::log10(relErr);
	return (digits > 0.0) ? digits : 0.0;
}

// Measure average decimal accuracy at a given magnitude for a type T.
// Samples N values in [base, 2*base) and returns the average correct digits.
template<typename T>
double averageDecimalAccuracy(double base, int N = 32) {
	if (N <= 0) return 0.0;
	double step = base / static_cast<double>(N);
	double totalDigits = 0.0;
	int count = 0;
	for (int i = 0; i < N; ++i) {
		double exact = base + i * step;
		T approx(exact);
		double back = double(approx);
		totalDigits += decimalAccuracy(exact, back);
		++count;
	}
	return totalDigits / count;
}

// Scan decimal accuracy across magnitudes for four 16-bit formats.
template<typename Takum16, typename Posit16, typename Cfloat16, typename Lns16>
void ScanDecimalAccuracy() {
	constexpr int LABEL_W = 14;
	constexpr int COL_W   = 12;
	constexpr int SAMPLES = 64;

	std::cout << std::setw(LABEL_W) << "magnitude"
	          << std::setw(COL_W)   << "takum<16>"
	          << std::setw(COL_W)   << "posit<16>"
	          << std::setw(COL_W)   << "cfloat<16>"
	          << std::setw(COL_W)   << "lns<16>"
	          << '\n';
	std::cout << std::string(LABEL_W + 4 * COL_W, '-') << '\n';

	// Scan from very small to very large magnitudes
	for (int exp = -30; exp <= 30; exp += 2) {
		double base = std::pow(10.0, exp);

		double takum_acc  = averageDecimalAccuracy<Takum16>(base, SAMPLES);
		double posit_acc  = averageDecimalAccuracy<Posit16>(base, SAMPLES);
		double cfloat_acc = averageDecimalAccuracy<Cfloat16>(base, SAMPLES);
		double lns_acc    = averageDecimalAccuracy<Lns16>(base, SAMPLES);

		std::cout << "  10^" << std::setw(4) << exp << "    "
		          << std::fixed << std::setprecision(2)
		          << std::setw(COL_W) << takum_acc
		          << std::setw(COL_W) << posit_acc
		          << std::setw(COL_W) << cfloat_acc
		          << std::setw(COL_W) << lns_acc
		          << '\n';
	}
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "Takum Decimal Accuracy Comparison\n";
	std::cout << "=================================\n\n";
	std::cout << "Average correct decimal digits at each magnitude\n";
	std::cout << "for 16-bit number formats.\n\n";
	std::cout << "The takum paper (Hunhold 2024) shows that takums provide\n";
	std::cout << "more precision than posits at the extremes (far from 1.0)\n";
	std::cout << "while staying competitive near 1.0.\n\n";

	// 16-bit configurations:
	//   takum<16>          : 16-bit linear takum, 3-bit regime (spec default)
	//   posit<16,2>        : 16-bit posit, 2 exponent bits
	//   cfloat<16,5,...>   : IEEE 754 half-precision (1+5+10)
	//   lns<16,8>          : 16-bit logarithmic number system
	using Takum16  = takum<16>;
	using Posit16  = posit<16, 2>;
	using Cfloat16 = cfloat<16, 5, std::uint16_t, true, false, false>;
	using Lns16    = lns<16, 8>;

	std::cout << "Configurations:\n";
	std::cout << "  " << type_tag(Takum16())  << '\n';
	std::cout << "  " << type_tag(Posit16())  << '\n';
	std::cout << "  " << type_tag(Cfloat16()) << '\n';
	std::cout << "  " << type_tag(Lns16())    << '\n';
	std::cout << '\n';

	ScanDecimalAccuracy<Takum16, Posit16, Cfloat16, Lns16>();

	std::cout << "\n";
	std::cout << "Key observations:\n";
	std::cout << "  - Near 1.0 (10^0): all tapered formats provide ~3 digits\n";
	std::cout << "  - At extremes (10^-20, 10^+20): takum retains more digits\n";
	std::cout << "    than posit because the bounded 3-bit regime wastes fewer\n";
	std::cout << "    bits on exponent encoding\n";
	std::cout << "  - cfloat (IEEE half) has uniform precision but limited range\n";
	std::cout << "    (0 digits beyond ~10^4 due to overflow/underflow)\n";

	return EXIT_SUCCESS;
}
catch (const std::exception& err) {
	std::cerr << "Exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
