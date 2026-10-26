#include <cstdlib>
#include <iostream>

#include <universal/number/valid/valid.hpp>

namespace {

int failures = 0;

void check(bool condition, const char* message) {
	if (!condition) {
		++failures;
		std::cerr << "FAIL: " << message << '\n';
	}
}

}  // namespace

int main() {
	using namespace sw::universal;
	using Posit = posit<7, 1>;
	using Poxel = poxel<8, 1>;
	using Valid = valid<8, 1>;

	const Valid a(Posit(1), Posit(2));
	const Valid same(Posit(1), Posit(2));
	const Valid differentlyOpen(Posit(1), Posit(2), true, false);
	const Valid b(Posit(3), Posit(4));
	const Valid touching(Posit(2), Posit(3));
	const Valid touchingOpen(Posit(2), Posit(3), true, false);

	check(a == same, "structural equality");
	check(a != differentlyOpen, "openness participates in equality");
	check(a < b, "disjoint interval less-than ordering");
	check(b > a, "disjoint interval greater-than ordering");
	check(!(a < touching), "closed touching intervals are not strictly ordered");
	check(a < touchingOpen, "an excluded touching point is strictly ordered");
	check(a <= same, "non-strict less-than equality");
	check(a >= same, "non-strict greater-than equality");

	const Valid nar = Valid::nar();
	check(nar.sign() == valid_sign::undecidable, "NaR sign is undecidable");
	check(Valid(0).sign() == valid_sign::zero, "exact zero sign");
	check(Valid(Posit(1), Posit(2)).sign() == valid_sign::positive, "positive interval sign");
	check(Valid(Posit(-2), Posit(-1)).sign() == valid_sign::negative, "negative interval sign");
	check(Valid(Posit(-1), Posit(1)).sign() == valid_sign::undecidable, "cross-zero interval sign");
	check(Valid(Posit(0), Posit(1)).sign() == valid_sign::undecidable, "zero-touching interval sign");

	check(!(nar < a) && !(a < nar), "NaR intervals are unordered");

	const Valid positiveUnbounded(Poxel(Posit(1)), Poxel(SpecificValue::infpos));
	const Valid negativeUnbounded(Poxel(SpecificValue::infneg), Poxel(Posit(-1)));
	check(!(positiveUnbounded < b), "positive infinity upper bound is unordered");
	check(!(a < negativeUnbounded), "negative infinity lower bound is unordered");

	check(hull(nar, a) == a && hull(a, nar) == a, "hull ignores NaR inputs");
	check(hull(a, b) == Valid(Posit(1), Posit(4)), "hull spans both intervals");

	if (failures == 0)
		std::cout << "valid logic: PASS\n";
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
