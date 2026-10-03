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
	using sw::universal::posit;
	using sw::universal::valid;
	using Posit = posit<8, 1>;
	using Valid = valid<8, 1>;

	const Valid a(Posit(1), Posit(2));
	const Valid same(Posit(1), Posit(2));
	const Valid differentlyOpen(Posit(1), Posit(2), true, false);
	const Valid b(Posit(3), Posit(4));
	const Valid touching(Posit(2), Posit(3));
	const Valid touchingOpen(Posit(2), Posit(3), true, false);

	check(a == same, "structural equality");
	check(a != differentlyOpen, "openness participates in equality");
	check(a < b && b > a, "disjoint interval ordering");
	check(!(a < touching), "closed touching intervals are not strictly ordered");
	check(a < touchingOpen, "an excluded touching point is strictly ordered");
	check(a <= same && a >= same, "non-strict equality");

	if (failures == 0)
		std::cout << "valid logic: PASS\n";
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
