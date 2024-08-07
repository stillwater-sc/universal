// numeric_constants.cpp: experiments with mixed-precision representations of important numerical constants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/integer/integer.hpp>
#include <numbers>   // since C++20

namespace sw {
	namespace universal {

		/*
		template <class _Ty>
		struct _Invalid {
			static_assert(_Always_false<_Ty>, "A program that instantiates a primary template of a mathematical constant "
				"variable template is ill-formed. (N4950 [math.constants]/3)");
		};

		template <class _Ty>
			inline constexpr _Ty e_v = _Invalid<_Ty>{};
		template <class _Ty>
			inline constexpr _Ty log2e_v = _Invalid<_Ty>{};
		template <class _Ty>
			inline constexpr _Ty log10e_v = _Invalid<_Ty>{};
		template <class _Ty>
			inline constexpr _Ty pi_v = _Invalid<_Ty>{};
		template <class _Ty>
			inline constexpr _Ty inv_pi_v = _Invalid<_Ty>{};
		template <class _Ty>
			inline constexpr _Ty inv_sqrtpi_v = _Invalid<_Ty>{};
		template <class _Ty>
			inline constexpr _Ty ln2_v = _Invalid<_Ty>{};
		template <class _Ty>
			inline constexpr _Ty ln10_v = _Invalid<_Ty>{};
		template <class _Ty>
			inline constexpr _Ty sqrt2_v = _Invalid<_Ty>{};
		template <class _Ty>
			inline constexpr _Ty sqrt3_v = _Invalid<_Ty>{};
		template <class _Ty>
			inline constexpr _Ty inv_sqrt3_v = _Invalid<_Ty>{};
		template <class _Ty>
			inline constexpr _Ty egamma_v = _Invalid<_Ty>{};
		template <class _Ty>
			inline constexpr _Ty phi_v = _Invalid<_Ty>{};
		*/

		template <typename _Ty>
		inline CONSTEXPRESSION _Ty e_v = static_cast<_Ty>(2.718281828459045);
		template <typename _Ty>
		inline CONSTEXPRESSION _Ty log2e_v = static_cast<_Ty>(1.4426950408889634);
		template <typename _Ty>
		inline CONSTEXPRESSION _Ty log10e_v = static_cast<_Ty>(0.4342944819032518);
		template <typename _Ty>
		inline CONSTEXPRESSION _Ty pi_v = static_cast<_Ty>(3.141592653589793);
		template <typename _Ty>
		inline CONSTEXPRESSION _Ty inv_pi_v = static_cast<_Ty>(0.3183098861837907);
		template <typename _Ty>
		inline CONSTEXPRESSION _Ty inv_sqrtpi_v = static_cast<_Ty>(0.5641895835477563);
		template <typename _Ty>
		inline CONSTEXPRESSION _Ty ln2_v = static_cast<_Ty>(0.6931471805599453);
		template <typename _Ty>
		inline CONSTEXPRESSION _Ty ln10_v = static_cast<_Ty>(2.302585092994046);
		template <typename _Ty>
		inline CONSTEXPRESSION _Ty sqrt2_v = static_cast<_Ty>(1.4142135623730951);
		template <typename _Ty>
		inline CONSTEXPRESSION _Ty sqrt3_v = static_cast<_Ty>(1.7320508075688772);
		template <typename _Ty>
		inline CONSTEXPRESSION _Ty inv_sqrt3_v = static_cast<_Ty>(0.5773502691896257);
		template <typename _Ty>
		inline CONSTEXPRESSION _Ty egamma_v = static_cast<_Ty>(0.5772156649015329);
		template <typename _Ty>
		inline CONSTEXPRESSION _Ty phi_v = static_cast<_Ty>(1.618033988749895);

		inline CONSTEXPRESSION half e_h          = e_v<half>;
		inline CONSTEXPRESSION half log2e_h      = log2e_v<half>;
		inline CONSTEXPRESSION half log10e_h     = log10e_v<half>;
		inline CONSTEXPRESSION half pi_h         = pi_v<half>;
		inline CONSTEXPRESSION half inv_pi_h     = inv_pi_v<half>;
		inline CONSTEXPRESSION half inv_sqrtpi_h = inv_sqrtpi_v<half>;
		inline CONSTEXPRESSION half ln2_h        = ln2_v<half>;
		inline CONSTEXPRESSION half ln10_h       = ln10_v<half>;
		inline CONSTEXPRESSION half sqrt2_h      = sqrt2_v<half>;
		inline CONSTEXPRESSION half sqrt3_h      = sqrt3_v<half>;
		inline CONSTEXPRESSION half inv_sqrt3_h  = inv_sqrt3_v<half>;
		inline CONSTEXPRESSION half egamma_h     = egamma_v<half>;
		inline CONSTEXPRESSION half phi_h        = phi_v<half>;

		inline CONSTEXPRESSION single e_f          = e_v<single>;
		inline CONSTEXPRESSION single log2e_f      = log2e_v<single>;
		inline CONSTEXPRESSION single log10e_f     = log10e_v<single>;
		inline CONSTEXPRESSION single pi_f         = pi_v<single>;
		inline CONSTEXPRESSION single inv_pi_f     = inv_pi_v<single>;
		inline CONSTEXPRESSION single inv_sqrtpi_f = inv_sqrtpi_v<single>;
		inline CONSTEXPRESSION single ln2_f        = ln2_v<single>;
		inline CONSTEXPRESSION single ln10_f       = ln10_v<single>;
		inline CONSTEXPRESSION single sqrt2_f      = sqrt2_v<single>;
		inline CONSTEXPRESSION single sqrt3_f      = sqrt3_v<single>;
		inline CONSTEXPRESSION single inv_sqrt3_f  = inv_sqrt3_v<single>;
		inline CONSTEXPRESSION single egamma_f     = egamma_v<single>;
		inline CONSTEXPRESSION single phi_f        = phi_v<single>;

		inline CONSTEXPRESSION double e_d          = std::numbers::e_v<double>;
		inline CONSTEXPRESSION double log2e_d      = std::numbers::log2e_v<double>;
		inline CONSTEXPRESSION double log10e_d     = std::numbers::log10e_v<double>;
		inline CONSTEXPRESSION double pi_d         = std::numbers::pi_v<double>;
		inline CONSTEXPRESSION double inv_pi_d     = std::numbers::inv_pi_v<double>;
		inline CONSTEXPRESSION double inv_sqrtpi_d = std::numbers::inv_sqrtpi_v<double>;
		inline CONSTEXPRESSION double ln2_d        = std::numbers::ln2_v<double>;
		inline CONSTEXPRESSION double ln10_d       = std::numbers::ln10_v<double>;
		inline CONSTEXPRESSION double sqrt2_d      = std::numbers::sqrt2_v<double>;
		inline CONSTEXPRESSION double sqrt3_d      = std::numbers::sqrt3_v<double>;
		inline CONSTEXPRESSION double inv_sqrt3_d  = std::numbers::inv_sqrt3_v<double>;
		inline CONSTEXPRESSION double egamma_d     = std::numbers::egamma_v<double>;
		inline CONSTEXPRESSION double phi_d        = std::numbers::phi_v<double>;
	}
}

template<typename _Ty>
void Compare(const std::string& tag, _Ty c, double ref) {
	using namespace sw::universal;

	std::cout << tag << " : ";
	auto old = std::cout.precision();
	std::cout << std::setprecision(std::numeric_limits<_Ty>::digits10);
	std::cout << to_binary(c) << " : " << std::left << std::scientific << std::setw(std::numeric_limits<_Ty>::digits10+2) << std::setfill('0') << c << " : ";
	double error = ref - double(c);
	std::cout << std::left << std::scientific << std::setw(std::numeric_limits<_Ty>::digits10 + 3) << std::setfill('0') << std::abs(error) << '\n';
	std::cout << std::setprecision(old);
}

void CompareHalfPrecisionConstants() {
	using namespace sw::universal;
	using namespace std::numbers;

	std::cout << "constant     : binary               : value :    error\n";
	Compare("e_h         ", e_h, e);
	Compare("log2e_h     ", log2e_h, log2e);
	Compare("log10e_h    ", log10e_h, log10e);
	Compare("pi_h        ", pi_h, pi);
	Compare("inv_pi_h    ", inv_pi_h, inv_pi);
	Compare("inv_sqrtpi_h", inv_sqrtpi_h, inv_sqrtpi);
	Compare("ln2_h       ", ln2_h, ln2);
	Compare("ln10_h      ", ln10_h, ln10);
	Compare("sqrt2_h     ", sqrt2_h, sqrt2);
	Compare("inv_sqrt3_h ", inv_sqrt3_h, inv_sqrt3);
	Compare("egamma_h    ", egamma_h, egamma);
	Compare("phi_h       ", phi_h, phi);
}

void CompareSinglePrecisionConstants() {
	using namespace sw::universal;
	using namespace std::numbers;

	std::cout << "constant     : binary                               :   value   :    error\n";
	Compare("e_f         ", e_f, e);
	Compare("log2e_f     ", log2e_f, log2e);
	Compare("log10e_f    ", log10e_f, log10e);
	Compare("pi_f        ", pi_f, pi);
	Compare("inv_pi_f    ", inv_pi_f, inv_pi);
	Compare("inv_sqrtpi_f", inv_sqrtpi_f, inv_sqrtpi);
	Compare("ln2_f       ", ln2_f, ln2);
	Compare("ln10_f      ", ln10_f, ln10);
	Compare("sqrt2_f     ", sqrt2_f, sqrt2);
	Compare("inv_sqrt3_f ", inv_sqrt3_f, inv_sqrt3);
	Compare("egamma_f    ", egamma_f, egamma);
	Compare("phi_f       ", phi_f, phi);
}

void CompareDoublePrecisionConstants() {
	using namespace sw::universal;
	using namespace std::numbers;

	std::cout << "constant     : binary                                                               :         value         :        error\n";
	Compare("e_d         ", e_d, e);
	Compare("log2e_d     ", log2e_d, log2e);
	Compare("log10e_d    ", log10e_d, log10e);
	Compare("pi_d        ", pi_d, pi);
	Compare("inv_pi_d    ", inv_pi_d, inv_pi);
	Compare("inv_sqrtpi_d", inv_sqrtpi_d, inv_sqrtpi);
	Compare("ln2_d       ", ln2_d, ln2);
	Compare("ln10_d      ", ln10_d, ln10);
	Compare("sqrt2_d     ", sqrt2_d, sqrt2);
	Compare("inv_sqrt3_d ", inv_sqrt3_d, inv_sqrt3);
	Compare("egamma_d    ", egamma_d, egamma);
	Compare("phi_d       ", phi_d, phi);
}

int main()
try {
	using namespace sw::universal;

	CompareHalfPrecisionConstants();
	CompareSinglePrecisionConstants();
	CompareDoublePrecisionConstants();

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

