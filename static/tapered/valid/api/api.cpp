#include <cstdlib>
#include <sstream>
#include <string>

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
	using Posit = posit<8, 1>;
	using Valid = valid<8, 1>;

	const Valid zero;
	check(zero.isexact(), "default construction is exact");
	check(zero.iszero(), "default construction is zero");
	check(zero.lb() == Posit(0) && zero.ub() == Posit(0), "zero bounds");

	const Valid one(Posit(1));
	check(one.isexact(), "posit construction is exact");
	check(one.lb() == Posit(1) && one.ub() == Posit(1), "point bounds");

	const Valid inexact(0.1);
	check(inexact.isinterval(), "inexact native construction creates an interval");
	check(inexact.lower_open() && inexact.upper_open(), "rounded native bounds are open");
	check(static_cast<long double>(inexact.lb()) < 0.1L, "native lower enclosure");
	check(static_cast<long double>(inexact.ub()) > 0.1L, "native upper enclosure");

	const Valid range(Posit(1), Posit(2), true, false);
	check(range.lb() == Posit(1) && range.ub() == Posit(2), "explicit bounds");
	check(range.lubit() && !range.uubit(), "explicit openness");
	check((one + 2).isexact() && (2 + one).lb() == Posit(3), "native addition operators");
	check((one * 3).isexact() && (3 * one).lb() == Posit(3), "native multiplication operators");

	const Valid invalid(Posit(2), Posit(1));
	check(invalid.isnar(), "reversed bounds form NaR");
	const Valid reversedNative(0.101, 0.1);
	check(reversedNative.isnar(), "reversed native bounds form NaR before conversion");
	const Valid reversedMixed(-1, 1u);
	check(!reversedMixed.isnar(), "mixed-sign integral bounds preserve ordering");
	const Valid reversedMixedInvalid(1u, -1);
	check(reversedMixedInvalid.isnar(), "reversed mixed-sign integral bounds form NaR");
	const Valid nativeOverflow(1.0, 1.0e300);
	check(nativeOverflow.isnar(), "native overflow forms NaR");

	std::ostringstream text;
	text << range;
	check(!text.str().empty() && text.str().front() == '(' && text.str().back() == ']', "interval stream formatting");
	check(type_tag(range) == "valid<8,1>", "type tag");
	const std::string bits = to_binary(range);
	check(!bits.empty() && bits.front() == '(' && bits.back() == ']', "binary interval formatting");

	const auto nar = std::numeric_limits<Valid>::quiet_NaN();
	check(nar.isnar(), "numeric_limits quiet_NaN");

	if (failures == 0)
		std::cout << "valid API: PASS\n";
	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
