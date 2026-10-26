#include <cmath>
#include <cstdlib>
#include <iostream>

#include <universal/number/valid/valid.hpp>

namespace {

int failures = 0;

template<typename Valid>
bool contains(const Valid& interval, long double value) {
	if (interval.isnar())
		return false;
	const auto lower      = interval.template lower<long double>();
	const auto upper      = interval.template upper<long double>();
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
	using sw::universal::poxel;
	using sw::universal::SpecificValue;
	using sw::universal::valid;
	using Posit = posit<5, 0>;
	using Valid = valid<6, 0>;

	constexpr unsigned states = 1u << 5;
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

	using WidePosit = posit<7, 1>;
	using WidePoxel = poxel<8, 1>;
	using WideValid = valid<8, 1>;
	const WideValid left(WidePosit(-2), WidePosit(3));
	const WideValid right(WidePosit(4), WidePosit(5));
	const WideValid product = left * right;
	check(contains(product, -10.0L) && contains(product, 15.0L), "interval multiplication", 0, 0);
	check(product.lower_closed() && product.upper_open(), "endpoint poxel openness", 0, 0);

	const WideValid third = WideValid(1) / WideValid(3);
	check(contains(third, 1.0L / 3.0L), "inexact division containment", 0, 0);
	check(third.lower_open() && third.upper_open(), "inexact quotient endpoints are open", 0, 0);

	const WideValid open(WidePosit(1), WidePosit(2), true, false);
	const WideValid translated = open + WideValid(3);
	check(translated.lower<double>() == 4.0 && translated.upper<double>() == 5.0, "interval addition bounds", 0, 0);
	check(translated.lower_open() && translated.upper_closed(), "input openness propagation", 0, 0);

	const WideValid crossingZero(WidePosit(-1), WidePosit(1));
	check(!((WideValid(1) / crossingZero).isnar()), "division across zero is an unbounded enclosure", 0, 0);
	check((WideValid(1) / WideValid(0)).isnar(), "division by exact zero is NaR", 0, 0);

	const WideValid validNar = WideValid::nar();
	check((WideValid(1) + validNar).isnar(), "addition propagates NaR", 0, 0);
	check((WideValid(1) * validNar).isnar(), "multiplication propagates NaR", 0, 0);
	check((WideValid(1) / validNar).isnar(), "division propagates NaR", 0, 0);

	const WideValid squared = square(WideValid(WidePosit(-2), WidePosit(3)));
	check(squared.lower<double>() == 0.0 && contains(squared, 9.0L), "square clamps a cross-zero lower bound", 0, 0);
	check(square(validNar).isnar(), "square propagates NaR", 0, 0);

	check(intersect(validNar, WideValid(1)).isnar(), "intersection with NaR left operand", 0, 0);
	check(intersect(WideValid(1), validNar) == WideValid(1), "intersection with NaR right operand", 0, 0);
	check(intersect(WideValid(WidePosit(1), WidePosit(3)), WideValid(WidePosit(2), WidePosit(4))) ==
	          WideValid(WidePosit(2), WidePosit(3)),
	      "intersection retains overlap", 0, 0);
	check(intersect(WideValid(1), WideValid(3)).isnar(), "disjoint intersection is NaR", 0, 0);
	check(sqrt(validNar).isnar(), "sqrt propagates NaR", 0, 0);
	check(sqrt(WideValid(WidePosit(-3), WidePosit(-1))).isnar(), "sqrt rejects negative interval", 0, 0);
	check(sqrt(WideValid(0)).iszero(), "sqrt exact zero", 0, 0);
	check(sqrt(WideValid(WidePosit(1), WidePosit(4))).contains(2.0), "sqrt positive interval", 0, 0);
	check(sqrt(WideValid(WidePosit(1), WidePosit(4), true, false)).lower_open(), "sqrt open lower bound", 0, 0);
	check(sqrt(WideValid(2)).isinterval(), "sqrt inexact lattice point", 0, 0);
	check(cos(validNar).isnar(), "cos propagates NaR", 0, 0);
	check(cos(WideValid(0)).contains(1.0), "cos exact zero", 0, 0);

	const WideValid approachingZero(WidePosit(0), WidePosit(1), true, false);
	const WideValid positiveUnbounded = WideValid(1) / approachingZero;
	check(positiveUnbounded.lower<double>() == 1.0, "positive zero limit lower bound", 0, 0);
	check(positiveUnbounded.upper_open() && std::isinf(positiveUnbounded.upper<double>()) &&
	          positiveUnbounded.upper<double>() > 0.0,
	      "positive zero limit upper bound", 0, 0);

	const WideValid approachingNegativeZero(WidePosit(-1), WidePosit(0), false, true);
	const WideValid negativeUnbounded = WideValid(1) / approachingNegativeZero;
	check(negativeUnbounded.upper<double>() == -1.0, "negative zero limit upper bound", 0, 0);
	check(negativeUnbounded.lower_open() && std::isinf(negativeUnbounded.lower<double>()) &&
	          negativeUnbounded.lower<double>() < 0.0,
	      "negative zero limit lower bound", 0, 0);

	const WideValid unboundedPositive(WidePoxel(WidePosit(1)), WidePoxel(SpecificValue::infpos));
	const WideValid unboundedNegative(WidePoxel(SpecificValue::infneg), WidePoxel(WidePosit(-1)));
	check((unboundedPositive + unboundedNegative) == WideValid::entire(), "opposite infinities widen addition", 0, 0);
	check((unboundedPositive * WideValid(0)).iszero(), "zero times infinity is zero", 0, 0);
	check((unboundedPositive * WideValid(2)).upper_open(), "nonzero times infinity is unbounded", 0, 0);

	const WideValid finiteOverInfinity = WideValid(1) / unboundedPositive;
	check(!finiteOverInfinity.isnar() && finiteOverInfinity.contains(0.0), "finite divided by infinity contains zero",
	      0, 0);
	check((unboundedPositive / unboundedPositive) == WideValid::entire(), "infinity divided by infinity widens", 0, 0);
	check((unboundedPositive / approachingZero).upper_open(), "infinity divided by open zero remains unbounded", 0, 0);
	check((unboundedNegative / approachingZero).lower_open(),
	      "negative infinity divided by open zero remains unbounded", 0, 0);

	const WideValid openZeroNumerator(WidePosit(0), WidePosit(1), true, false);
	const WideValid openZeroQuotient = openZeroNumerator / approachingZero;
	check(openZeroQuotient.lower_open() && openZeroQuotient.lower<double>() == 0.0,
	      "open zero over open zero has open zero lower bound", 0, 0);
	check(openZeroQuotient.upper_open() && std::isinf(openZeroQuotient.upper<double>()) &&
	          openZeroQuotient.upper<double>() > 0.0,
	      "open zero over open zero has positive unbounded upper bound", 0, 0);
	check((WideValid(0) / approachingZero).iszero(), "exact zero over open zero is zero", 0, 0);

	const WidePosit maxpos(sw::universal::SpecificValue::maxpos);
	const WidePosit maxneg(sw::universal::SpecificValue::maxneg);
	const WidePosit minpos(sw::universal::SpecificValue::minpos);
	const WideValid reachesMax(WidePosit(1), maxpos);
	const WideValid reachesMin(maxneg, WidePosit(-1));
	check((reachesMax + WideValid(1)).upper_open(), "addition past maxpos", 0, 0);
	check((reachesMin - WideValid(1)).lower_open(), "subtraction past maxneg", 0, 0);
	check((reachesMax * WideValid(2)).upper_open(), "multiplication past maxpos", 0, 0);
	check((reachesMax / WideValid(minpos)).upper_open(), "division past maxpos", 0, 0);

	using MultiLimbValid              = valid<80, 2, std::uint32_t>;
	const MultiLimbValid multiLimbSum = MultiLimbValid(2) + MultiLimbValid(3);
	check(multiLimbSum.isexact() && multiLimbSum.lower<double>() == 5.0, "multi-limb exact addition", 0, 0);
	const MultiLimbValid multiLimbThird = MultiLimbValid(1) / MultiLimbValid(3);
	check(!multiLimbThird.isnar() && multiLimbThird.isinterval() && multiLimbThird.lower_open() &&
	          multiLimbThird.upper_open(),
	      "multi-limb inexact division", 0, 0);

	if (failures == 0)
		std::cout << "valid arithmetic: PASS\n";
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
