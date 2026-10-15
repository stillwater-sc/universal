#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

#include <universal/number/valid/valid.hpp>

namespace {

int failures = 0;

void check(bool ok, const char* message) {
	if (!ok) {
		++failures;
		std::cerr << "FAIL: " << message << '\n';
	}
}

template<typename Valid>
bool contains(const Valid& v, long double x) {
	return v.contains(x);
}

template<typename V>
V rump() {
	const V a(77617), b(33096);
	const V b2 = b * b, b4 = b2 * b2, b6 = b4 * b2, b8 = b4 * b4, a2 = a * a;
	return V(333.75) * b6 + a2 * (V(11) * a2 * b2 - b6 - V(121) * b4 - V(2)) + V(5.5) * b8 + a / (V(2) * b);
}

template<typename V>
V bbp_tail() {
	V sum(0), factor(1);
	for (int k = 15; k <= 40; ++k) {
		sum += factor * (V(4) / V(8 * k + 1) - V(2) / V(8 * k + 4) - V(1) / V(8 * k + 5) - V(1) / V(8 * k + 6));
		factor /= V(16);
	}
	V scale(1);
	for (int k = 0; k < 15; ++k)
		scale /= V(16);
	const V                      remainder = factor * V(4) / V(8 * 41 + 1) * V(16) / V(15);
	const typename V::poxel_type zeroPxl(0);
	const V                      remainderRange(zeroPxl, remainder.ub());
	return scale * (sum + remainderRange);
}

}  // namespace

int main() {
	using namespace sw::universal;
	using Poxel8 = poxel<8, 1>;
	using V32    = valid<32, 2, std::uint32_t>;
	using V64    = valid<64, 2, std::uint64_t>;

	// A single poxel is not an enclosure after sticky arithmetic.
	const Poxel8 pxl = Poxel8(1) / Poxel8(3);
	check(pxl.ubit(), "poxel third is open");
	check(to_valid(pxl).lb() == pxl && to_valid(pxl).ub() == pxl, "manual poxel conversion");
	check(add_valid(Poxel8(1), pxl) == to_valid(Poxel8(1)) + to_valid(pxl), "poxel addition to valid");
	check(add_valid(Poxel8(1), pxl) == Poxel8(1) + to_valid(pxl), "mixed poxel and valid addition");
	check(sub_valid(Poxel8(1), pxl) == to_valid(Poxel8(1)) - to_valid(pxl), "poxel subtraction to valid");
	check(mul_valid(Poxel8(3), pxl) == to_valid(Poxel8(3)) * to_valid(pxl), "poxel multiplication to valid");
	check(div_valid(Poxel8(1), pxl) == to_valid(Poxel8(1)) / to_valid(pxl), "poxel division to valid");
	check(contains(mul_valid(Poxel8(3), pxl), 1.0L), "valid multiplication retains exact one");
	const V32 broad = V32(1) / V32(0.001, 0.002);
	check(contains(broad, 500.0L) && contains(broad, 1000.0L), "reciprocal widens across poxels");
	check(broad.lb() != broad.ub(), "reciprocal needs two poxel bounds");

	// 1. Rump's polynomial: the 32-bit enclosure cannot determine the sign.
	const V32 r = rump<V32>();
	check(contains(r, -54767.0L / 66192.0L), "Rump exact result enclosed");
	check(r.sign() == valid_sign::undecidable, "Rump sign undecidable");

	// 2. Muller-Kahan: uncertainty widens rather than converging falsely to 100.
	V32 x0 = V32(11) / V32(2), x1 = V32(61) / V32(11);
	for (int n = 1; n < 15; ++n) {
		V32 next = V32(111) - (V32(1130) - V32(3000) / x0) / x1;
		x0       = x1;
		x1       = next;
	}
	const long double exact14 = (std::pow(6.0L, 15) + std::pow(5.0L, 15)) / (std::pow(6.0L, 14) + std::pow(5.0L, 14));
	check(contains(x0, exact14), "Muller-Kahan exact iterate enclosed");

	// 3. Near-singular matrix: the first determinant is provably positive.
	const V64 phi  = V64(161803398875.0) / V64(1.0e11);
	const V64 phi1 = V64(61803398875.0) / V64(1.0e11);
	check((phi * phi1 - V64(1)).sign() == valid_sign::positive, "golden-ratio matrix determinant positive");

	// 4. BBP tail: the extended posit regime preserves a strictly positive bound.
	const auto tail = bbp_tail<valid<16, 3>>();
	check(tail.sign() == valid_sign::positive, "BBP tail is positive");
	check(tail.upper<long double>() < 1.0e-16L, "BBP tail upper bound");

	// 5. Griewank: the enclosing cosine recovers the small positive sign.
	const V64 x(1.0e-8);
	const V64 g = V64(1) - cos(x) * cos(x) - (x * x + x * x) / V64(4000);
	check(g.sign() == valid_sign::positive, "Griewank sign positive");

	// 6. Quadratic: stable algebra avoids the cancellation of the direct formula.
	using V16 = valid<16, 2>;
	const V16 a(3), b(100), c(2);
	const V16 root   = sqrt(b * b - V16(4) * a * c);
	const V16 direct = (-b + root) / (V16(2) * a);
	const V16 stable = (V16(2) * c) / (-b - root);
	check(contains(direct, -0.0200120144216363534L), "direct quadratic root enclosed");
	check(contains(stable, -0.0200120144216363534L), "stable quadratic root enclosed");
	check(stable.sign() == valid_sign::negative, "stable quadratic root sign");
	check(stable.ub().key() - stable.lb().key() < direct.ub().key() - direct.lb().key(),
	      "stable formula spans fewer poxels");

	if (failures == 0)
		std::cout << "valid tutorial: PASS\n";
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
