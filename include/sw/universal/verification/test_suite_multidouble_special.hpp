#pragma once
// test_suite_multidouble_special.hpp: verification of the hyperbolic, error and gamma functions
// of the multi-double types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Shared by the regression suites of dd, qd, dd_cascade, td_cascade and qd_cascade (#1667).  Every
// reference is far more precise than the type under test, P = numeric_limits<Real>::digits:
//
//   erf, erfc, lgamma, tgamma   mpmath at 300 digits, six doubles each (multidouble_special_vectors.hpp);
//                               required to agree to P - 22 bits, relative (lgamma relative to
//                               max(|lgamma|, 1), since it vanishes at 1 and 2)
//   the hyperbolic functions    ereal<8>, about 424 bits; required to agree to P - 12 bits
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/multidouble_special_vectors.hpp>

namespace sw { namespace universal { namespace multidouble_verification {

	using Oracle = ereal<8>;

	template<typename Real>
	constexpr int precision = std::numeric_limits<Real>::digits;

	template<typename Real>
	Oracle to_oracle(const Real& x) {
		Oracle e(0.0);
		for (int i = 0; i < precision<Real> / 53; ++i) e += Oracle(x[i]);
		return e;
	}

	// bits of agreement between got and ref, relative to max(|ref|, floor)
	template<typename Real>
	double agreement(const Real& got, const Oracle& ref, double floor) {
		if (got.isnan() || got.isinf()) return 0.0;
		const Oracle diff = to_oracle(got) - ref;
		if (diff.iszero()) return 1000.0;
		const double scale = std::fmax(std::fabs(double(ref)), floor);
		return -std::log2(std::fabs(double(diff)) / scale);
	}

	inline Oracle six(const double (&v)[6]) {
		Oracle e(0.0);
		for (double d : v) e += Oracle(d);
		return e;
	}

	template<typename Real>
	int VerifySpecialFunction(const std::string& tag, const char* name, Real (*f)(const Real&),
	                          const double (SpecialVector::*field)[6], double floor, bool report) {
		const double required = precision<Real> - 22.0;
		int fails = 0;
		double worst = 1000.0;
		for (const auto& v : special_vectors) {
			const double (&ref)[6] = v.*field;
			if (ref[0] == 0.0 && ref[1] == 0.0) continue;              // not finite, or below double, there
			const double bits = agreement(f(Real(v.x)), six(ref), floor);
			worst = std::fmin(worst, bits);
			if (bits < required) {
				++fails;
				if (report) std::cerr << "  FAIL " << tag << ' ' << name << '(' << v.x << "): " << bits << " bits, want " << required << '\n';
			}
		}
		if (report) std::cout << "  " << tag << ' ' << name << ": worst agreement " << worst << " of " << precision<Real> << " bits\n";
		return fails;
	}

	template<typename Real> Real call_erf(const Real& x)    { return erf(x); }
	template<typename Real> Real call_erfc(const Real& x)   { return erfc(x); }
	template<typename Real> Real call_lgamma(const Real& x) { return lgamma(x); }
	template<typename Real> Real call_tgamma(const Real& x) { return tgamma(x); }

	template<typename Real>
	int VerifyErrorAndGamma(const std::string& tag, bool report) {
		int fails = 0;
		fails += VerifySpecialFunction<Real>(tag, "erf", call_erf<Real>, &SpecialVector::erf, 0.0, report);
		fails += VerifySpecialFunction<Real>(tag, "erfc", call_erfc<Real>, &SpecialVector::erfc, 0.0, report);
		fails += VerifySpecialFunction<Real>(tag, "lgamma", call_lgamma<Real>, &SpecialVector::lgamma, 1.0, report);
		fails += VerifySpecialFunction<Real>(tag, "tgamma", call_tgamma<Real>, &SpecialVector::tgamma, 0.0, report);
		return fails;
	}

	// identities and special values
	template<typename Real>
	int VerifyErrorAndGammaSpecialValues(const std::string& tag, bool report) {
		int fails = 0;
		auto expect = [&](bool ok, const char* what) { if (!ok) { ++fails; if (report) std::cerr << "  FAIL " << tag << ' ' << what << '\n'; } };
		expect(erf(Real(0.0)).iszero(), "erf(0) = 0");
		expect(lgamma(Real(1.0)).iszero(), "lgamma(1) = 0");
		expect(lgamma(Real(2.0)).iszero(), "lgamma(2) = 0");
		expect(lgamma(Real(0.0)).isinf() && lgamma(Real(-3.0)).isinf(), "lgamma at the poles is +inf");
		expect(tgamma(Real(0.0)).isnan() && tgamma(Real(-3.0)).isnan(), "tgamma at the poles is NaN");
		// Gamma(n) = (n - 1)!
		Real factorial(1.0);
		for (int n = 1; n <= 30; ++n) {
			if (n > 1) factorial *= Real(static_cast<double>(n - 1));
			const Real d = tgamma(Real(static_cast<double>(n))) - factorial;
			expect(std::fabs(d[0]) <= std::ldexp(factorial[0], -(precision<Real> - 17)), "tgamma(n) = (n - 1)!");
		}
		// close to a large negative integer Gamma(1 - x) overflows double, but Gamma(x) does not
		// underflow: Gamma(-171 + 2^-20) = -8.449e-304 (mpmath)
		{
			const Real x = Real(-171.0) + Real(std::ldexp(1.0, -20));
			const double ref[6] = { -0x1.28aacdd8c4b60p-1007, -0x0.000000000188dp-1022, 0.0, 0.0, 0.0, 0.0 };
			expect(agreement(tgamma(x), six(ref), 0.0) >= precision<Real> - 22.0, "tgamma(-171 + 2^-20) = -8.449e-304");
		}
		// erf is odd, and erf + erfc = 1
		for (double x : { 0.1, 0.7, 1.3, 2.9, 3.1, 5.0, 7.5 }) {
			expect(erf(Real(-x)) == -erf(Real(x)), "erf is odd");
			const Real s = erf(Real(x)) + erfc(Real(x)) - Real(1.0);
			expect(std::fabs(s[0]) <= std::ldexp(1.0, -(precision<Real> - 12)), "erf + erfc = 1");
		}
		return fails;
	}

	// the hyperbolic functions against ereal<8>, at the arguments where their textbook forms fail
	template<typename Real>
	int VerifyHyperbolic(const std::string& tag, bool report) {
		const double required = precision<Real> - 12.0;
		int fails = 0;
		double worst = 1000.0;
		auto check = [&](const char* name, double x, const Real& got, const Oracle& ref) {
			const double bits = agreement(got, ref, 0.0);
			worst = std::fmin(worst, bits);
			if (bits < required) {
				++fails;
				if (report) std::cerr << "  FAIL " << tag << ' ' << name << '(' << x << "): " << bits << " bits, want " << required << '\n';
			}
		};
		// tiny, small, either side of the 1/2 switch, moderate, and large enough that e^-x vanishes
		for (double v : { 1.0e-300, 1.0e-30, 0.001, 0.4999, 0.5, 2.5, 30.0, 300.0 }) {
			for (double x : { v, -v }) {
				const Oracle o(x);
				check("sinh", x, sinh(Real(x)), sinh(o));
				check("cosh", x, cosh(Real(x)), cosh(o));
				check("tanh", x, tanh(Real(x)), tanh(o));
				check("asinh", x, asinh(Real(x)), asinh(o));
				if (std::fabs(x) < 1.0) check("atanh", x, atanh(Real(x)), atanh(o));
			}
		}
		for (double v : { 1.0e100, 1.0e300 }) {
			check("asinh", v, asinh(Real(v)), asinh(Oracle(v)));
			check("acosh", v, acosh(Real(v)), acosh(Oracle(v)));
		}
		// near 1: acosh(1 + d) and atanh(1 - d), d exact
		for (int k : { 1, 30, 60, 100 }) {
			if (k >= precision<Real> - 10) continue;
			const Real d = Real(std::ldexp(1.0, -k));
			const Oracle od(std::ldexp(1.0, -k));
			check("acosh(1 + 2^-k)", k, acosh(Real(1.0) + d), acosh(Oracle(1.0) + od));
			check("atanh(1 - 2^-k)", k, atanh(Real(1.0) - d), atanh(Oracle(1.0) - od));
		}
		for (double v : { 1.5, 3.0, 50.0 }) check("acosh", v, acosh(Real(v)), acosh(Oracle(v)));
		if (report) std::cout << "  " << tag << " hyperbolic: worst agreement " << worst << " of " << precision<Real> << " bits\n";
		// the odd functions are exactly odd
		for (double v : { 0.3, 0.6, 4.0 }) {
			if (!(sinh(Real(-v)) == -sinh(Real(v))) || !(tanh(Real(-v)) == -tanh(Real(v))) || !(asinh(Real(-v)) == -asinh(Real(v)))) {
				++fails;
				if (report) std::cerr << "  FAIL " << tag << " odd symmetry at " << v << '\n';
			}
		}
		return fails;
	}

	// log across the exponent range: beyond 2^500 the Newton step's exp(-x) used to reach the
	// bottom of double's range and lose its low limbs
	template<typename Real>
	int VerifyLogRange(const std::string& tag, bool report) {
		const double required = precision<Real> - 8.0;
		int fails = 0;
		for (double v : { 1.0e-300, std::ldexp(1.0, -600), 1.0e-200, 1.0e-5, 0.75, 3.0, 1.0e5, 1.0e200, std::ldexp(1.5, 600), 1.0e300, std::numeric_limits<double>::max() }) {
			const double bits = agreement(log(Real(v)), log(Oracle(v)), 0.0);
			if (bits < required) {
				++fails;
				if (report) std::cerr << "  FAIL " << tag << " log(" << v << "): " << bits << " bits, want " << required << '\n';
			}
		}
		return fails;
	}

}}}  // namespace sw::universal::multidouble_verification
