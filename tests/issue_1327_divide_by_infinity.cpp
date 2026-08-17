#include <cassert>
#include <cmath>
#include <limits>

#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>

template <typename T>
void verify_finite_dividend_by_infinity() {
	const T positive = T(2.0) / std::numeric_limits<T>::infinity();
	const T negative = T(-2.0) / std::numeric_limits<T>::infinity();

	assert(positive.iszero());
	assert(!std::signbit(positive[0]));
	assert(negative.iszero());
	assert(std::signbit(negative[0]));
}

int main() {
	verify_finite_dividend_by_infinity<sw::universal::dd>();
	verify_finite_dividend_by_infinity<sw::universal::qd>();
	verify_finite_dividend_by_infinity<sw::universal::dd_cascade>();
	verify_finite_dividend_by_infinity<sw::universal::td_cascade>();
	verify_finite_dividend_by_infinity<sw::universal::qd_cascade>();
}
