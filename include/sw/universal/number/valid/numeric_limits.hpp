#pragma once

#include <limits>
#include <universal/number/valid/core.hpp>
#include <universal/utility/decimal_digits.hpp>

namespace std {

template<unsigned nbits, unsigned es, typename bt>
class numeric_limits<sw::universal::valid<nbits, es, bt>> {
	using Valid = sw::universal::valid<nbits, es, bt>;
	using Posit = typename Valid::posit_type;

  public:
	static constexpr bool               is_specialized = true;
	static constexpr Valid              min() noexcept { return Valid(Posit(sw::universal::SpecificValue::minpos)); }
	static constexpr Valid              max() noexcept { return Valid(Posit(sw::universal::SpecificValue::maxpos)); }
	static constexpr Valid              lowest() noexcept { return Valid(Posit(sw::universal::SpecificValue::maxneg)); }
	static constexpr Valid              epsilon() noexcept { return Valid(std::numeric_limits<Posit>::epsilon()); }
	static constexpr Valid              round_error() noexcept { return Valid(Posit(0.5)); }
	static constexpr Valid              denorm_min() noexcept { return min(); }
	static constexpr Valid              infinity() noexcept { return max(); }
	static constexpr Valid              quiet_NaN() noexcept { return Valid(Posit(sw::universal::SpecificValue::nar)); }
	static constexpr Valid              signaling_NaN() noexcept { return quiet_NaN(); }
	static constexpr int                digits            = std::numeric_limits<Posit>::digits;
	static constexpr int                digits10          = sw::universal::decimal_digits10(digits);
	static constexpr int                max_digits10      = sw::universal::decimal_max_digits10(digits);
	static constexpr bool               is_signed         = true;
	static constexpr bool               is_integer        = false;
	static constexpr bool               is_exact          = false;
	static constexpr int                radix             = 2;
	static constexpr int                min_exponent      = std::numeric_limits<Posit>::min_exponent;
	static constexpr int                max_exponent      = std::numeric_limits<Posit>::max_exponent;
	static constexpr bool               has_infinity      = false;
	static constexpr bool               has_quiet_NaN     = true;
	static constexpr bool               has_signaling_NaN = true;
	static constexpr float_denorm_style has_denorm        = denorm_absent;
	static constexpr bool               has_denorm_loss   = false;
	static constexpr bool               is_iec559         = false;
	static constexpr bool               is_bounded        = true;
	static constexpr bool               is_modulo         = false;
	static constexpr bool               traps             = false;
	static constexpr bool               tinyness_before   = false;
	static constexpr float_round_style  round_style       = round_to_nearest;
};

}  // namespace std
