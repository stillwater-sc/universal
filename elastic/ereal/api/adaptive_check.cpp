// adaptive_threshold.cpp: debug adaptive threshold with ereal arithmetic
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_mathlib_adaptive.hpp>

int main() {
	using namespace sw::universal;
	using Real = ereal<16>;

	std::cout << "Debug: ereal arithmetic in check_relative_error\n";
	std::cout << "================================================\n\n";

	Real x(1.0);
	Real y(1.0 + 1e-20);

	std::cout << "x = " << x << "\n";
	std::cout << "y = " << y << "\n";

	Real diff = y - x;
	std::cout << "y - x = " << diff << "\n";

	Real abs_diff = abs(diff);
	std::cout << "abs(y - x) = " << abs_diff << "\n";

	Real rel_error = abs_diff / abs(y);
	std::cout << "rel_error = abs(y-x) / abs(y) = " << rel_error << "\n";
	std::cout << "rel_error as double = " << double(rel_error) << "\n";

	double threshold = get_adaptive_threshold<Real>();
	std::cout << "threshold = " << threshold << "\n";

	bool passes = double(rel_error) < threshold;
	std::cout << "Passes check? " << (passes ? "YES" : "NO") << "\n";

	std::cout << "\n--- Testing distant values ---\n";
	Real z(100.0);
	std::cout << "x = " << x << "\n";
	std::cout << "z = " << z << "\n";

	Real diff2 = x - z;
	std::cout << "x - z = " << diff2 << "\n";

	Real abs_diff2 = abs(diff2);
	std::cout << "abs(x - z) = " << abs_diff2 << "\n";

	Real rel_error2 = abs_diff2 / abs(z);
	std::cout << "rel_error2 = abs(x-z) / abs(z) = " << rel_error2 << "\n";
	std::cout << "rel_error2 as double = " << double(rel_error2) << "\n";

	bool passes2 = double(rel_error2) < threshold;
	std::cout << "Passes check? " << (passes2 ? "YES (WRONG!)" : "NO (correct)") << "\n";

	return 0;
}
