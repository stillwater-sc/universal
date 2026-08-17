#include <cassert>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>

template <typename T>
constexpr bool has_negative_zero_sign(const T& value) {
	return (std::bit_cast<std::uint64_t>(value[0]) >> 63) != 0;
}

constexpr sw::universal::dd dd_positive_positive =
	sw::universal::dd(2.0) / sw::universal::dd(sw::universal::SpecificValue::infpos);
constexpr sw::universal::dd dd_negative_positive =
	sw::universal::dd(-2.0) / sw::universal::dd(sw::universal::SpecificValue::infpos);
constexpr sw::universal::dd dd_positive_negative =
	sw::universal::dd(2.0) / sw::universal::dd(sw::universal::SpecificValue::infneg);
constexpr sw::universal::dd dd_negative_negative =
	sw::universal::dd(-2.0) / sw::universal::dd(sw::universal::SpecificValue::infneg);

static_assert(dd_positive_positive.iszero() && !has_negative_zero_sign(dd_positive_positive));
static_assert(dd_negative_positive.iszero() && has_negative_zero_sign(dd_negative_positive));
static_assert(dd_positive_negative.iszero() && has_negative_zero_sign(dd_positive_negative));
static_assert(dd_negative_negative.iszero() && !has_negative_zero_sign(dd_negative_negative));

constexpr sw::universal::qd qd_positive_positive =
	sw::universal::qd(2.0) / sw::universal::qd(sw::universal::SpecificValue::infpos);
constexpr sw::universal::qd qd_negative_positive =
	sw::universal::qd(-2.0) / sw::universal::qd(sw::universal::SpecificValue::infpos);
constexpr sw::universal::qd qd_positive_negative =
	sw::universal::qd(2.0) / sw::universal::qd(sw::universal::SpecificValue::infneg);
constexpr sw::universal::qd qd_negative_negative =
	sw::universal::qd(-2.0) / sw::universal::qd(sw::universal::SpecificValue::infneg);

static_assert(qd_positive_positive.iszero() && !has_negative_zero_sign(qd_positive_positive));
static_assert(qd_negative_positive.iszero() && has_negative_zero_sign(qd_negative_positive));
static_assert(qd_positive_negative.iszero() && has_negative_zero_sign(qd_positive_negative));
static_assert(qd_negative_negative.iszero() && !has_negative_zero_sign(qd_negative_negative));

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
