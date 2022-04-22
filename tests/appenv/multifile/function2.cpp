
#include <universal/number/cfloat/cfloat.hpp>
#include <iostream>

using FP8 = sw::universal::cfloat<8, 2, uint8_t, false, false, false>;

FP8 Polynomial2(const std::vector<FP8>& coef, const FP8& x) {
	using namespace sw::universal;
	if (coef.size() < 2) {
		std::cerr << "Coefficient set is too small to represent a polynomial\n";
		return FP8(0);
	}

	FP8 v = coef[0];
	for (size_t i = 1; i < coef.size(); ++i) {
		v += pow(x, FP8(i));
	}
	return v;
}
