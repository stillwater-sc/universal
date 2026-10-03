#include <cstdlib>
#include <iostream>

#include <universal/number/valid/valid.hpp>

namespace {

int failures = 0;

template<typename Valid>
bool contains(const Valid& interval, long double value) {
	if (interval.isnar())
		return false;
	const auto lower      = static_cast<long double>(interval.lb());
	const auto upper      = static_cast<long double>(interval.ub());
	const bool aboveLower = interval.lower_open() ? value > lower : value >= lower;
	const bool belowUpper = interval.upper_open() ? value < upper : value <= upper;
	return aboveLower && belowUpper;
}

void check(bool condition, const char* operation, unsigned lhs, unsigned rhs) {
	if (!condition) {
		++failures;
		if (failures <= 20) {
			std::cerr << "FAIL: " << operation << " patterns " << lhs << ", " << rhs << '\n';
		}
	}
}

}  // namespace

int main() {
	using sw::universal::posit;
	using sw::universal::valid;
	using Posit = posit<6, 0>;
	using Valid = valid<6, 0>;

	constexpr unsigned states = 1u << 6;
	for (unsigned i = 0; i < states; ++i) {
		Posit a;
		a.setbits(i);
		if (a.isnar())
			continue;
		for (unsigned j = 0; j < states; ++j) {
			Posit b;
			b.setbits(j);
			if (b.isnar())
				continue;

			const auto  da = static_cast<long double>(a);
			const auto  db = static_cast<long double>(b);
			const Valid va(a);
			const Valid vb(b);

			const Valid sum = va + vb;
			if (!sum.isnar())
				check(contains(sum, da + db), "addition", i, j);

			const Valid difference = va - vb;
			if (!difference.isnar()) {
				check(contains(difference, da - db), "subtraction", i, j);
			}

			const Valid product = va * vb;
			if (!product.isnar()) {
				check(contains(product, da * db), "multiplication", i, j);
			}

			if (!b.iszero()) {
				const Valid quotient = va / vb;
				if (!quotient.isnar()) {
					check(contains(quotient, da / db), "division", i, j);
				}
			}
		}
	}

	using WidePosit = posit<8, 1>;
	using WideValid = valid<8, 1>;
	const WideValid left(WidePosit(-2), WidePosit(3));
	const WideValid right(WidePosit(4), WidePosit(5));
	const WideValid product = left * right;
	check(product.lb() == WidePosit(-10) && product.ub() == WidePosit(15), "interval multiplication", 0, 0);
	check(product.lower_closed() && product.upper_closed(), "exact product endpoints are closed", 0, 0);

	const WideValid third = WideValid(1) / WideValid(3);
	check(contains(third, 1.0L / 3.0L), "inexact division containment", 0, 0);
	check(third.lower_open() && third.upper_open(), "inexact quotient endpoints are open", 0, 0);

	const WideValid open(WidePosit(1), WidePosit(2), true, false);
	const WideValid translated = open + WideValid(3);
	check(translated.lb() == WidePosit(4) && translated.ub() == WidePosit(5), "interval addition bounds", 0, 0);
	check(translated.lower_open() && translated.upper_closed(), "input openness propagation", 0, 0);

	const WideValid crossingZero(WidePosit(-1), WidePosit(1));
	check((WideValid(1) / crossingZero).isnar(), "division across zero", 0, 0);
	const WideValid approachingZero(WidePosit(0), WidePosit(1), true, false);
	check((WideValid(1) / approachingZero).isnar(), "division approaching zero", 0, 0);

	using MultiLimbValid              = valid<80, 2, std::uint32_t>;
	const MultiLimbValid multiLimbSum = MultiLimbValid(2) + MultiLimbValid(3);
	check(multiLimbSum.isexact() && multiLimbSum.lb() == typename MultiLimbValid::posit_type(5),
	      "multi-limb exact addition", 0, 0);
	const MultiLimbValid multiLimbThird = MultiLimbValid(1) / MultiLimbValid(3);
	check(!multiLimbThird.isnar() && multiLimbThird.lb() < multiLimbThird.ub(), "multi-limb inexact division", 0, 0);

	if (failures == 0)
		std::cout << "valid arithmetic: PASS\n";
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
