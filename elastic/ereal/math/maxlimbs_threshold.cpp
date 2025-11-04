// test_maxlimbs_threshold.cpp: determine maximum useful maxlimbs for double-based thresholds
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite_mathlib_adaptive.hpp>
#include <limits>
#include <iostream>
#include <iomanip>

int main() {
	using namespace sw::universal;

	std::cout << "Maximum Useful maxlimbs for Double-Based Thresholds\n";
	std::cout << "====================================================\n\n";

	std::cout << "DBL_MIN (smallest non-subnormal) = " << std::scientific
	          << std::numeric_limits<double>::min() << "\n";
	std::cout << "                                  ≈ 2^-1022\n";
	std::cout << "                                  ≈ 10^-308\n\n";

	std::cout << std::setw(10) << "maxlimbs"
	          << std::setw(12) << "digits10"
	          << std::setw(15) << "threshold"
	          << std::setw(20) << "representable?"
	          << "\n";
	std::cout << std::string(57, '-') << "\n";

	for (unsigned maxlimbs = 1; maxlimbs <= 25; ++maxlimbs) {
		// Manually calculate what digits10 would be
		int digits = maxlimbs * std::numeric_limits<double>::digits;  // maxlimbs * 53
		int digits10 = static_cast<int>(digits * 0.30103);

		// Calculate threshold
		int exponent = -(digits10 - 2);
		double threshold = std::pow(10.0, exponent);

		bool representable = (threshold > 0.0) && (threshold >= std::numeric_limits<double>::min());

		std::cout << std::setw(10) << maxlimbs
		          << std::setw(12) << digits10
		          << std::setw(15) << threshold
		          << std::setw(20) << (representable ? "YES" : "NO (underflow)")
		          << "\n";
	}

	std::cout << "\n";
	std::cout << "Conclusion: maxlimbs <= 19 for threshold to be representable\n";
	std::cout << "             maxlimbs >= 20 causes threshold underflow in double\n";

	return 0;
}
