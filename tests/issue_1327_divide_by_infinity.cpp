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
	const T positive_inf = std::numeric_limits<T>::infinity();
	const T negative_inf = -positive_inf;
	const T positive = T(2.0) / positive_inf;
	const T negative = T(-2.0) / positive_inf;
	const T positive_by_negative = T(2.0) / negative_inf;
	const T negative_by_negative = T(-2.0) / negative_inf;

	assert(positive.iszero());
	assert(!std::signbit(positive[0]));
	assert(negative.iszero());
	assert(std::signbit(negative[0]));
	assert(positive_by_negative.iszero());
	assert(std::signbit(positive_by_negative[0]));
	assert(negative_by_negative.iszero());
	assert(!std::signbit(negative_by_negative[0]));

	const T inf_over_inf = positive_inf / positive_inf;
	assert(inf_over_inf.isnan());
}

int main() {
	verify_finite_dividend_by_infinity<sw::universal::dd>();
	verify_finite_dividend_by_infinity<sw::universal::qd>();
	verify_finite_dividend_by_infinity<sw::universal::dd_cascade>();
	verify_finite_dividend_by_infinity<sw::universal::td_cascade>();
	verify_finite_dividend_by_infinity<sw::universal::qd_cascade>();
}
