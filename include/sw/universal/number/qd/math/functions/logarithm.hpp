#pragma once
// logarithm.hpp: logarithm functions for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cerrno>
#include <limits>

#include <universal/number/shared/multidouble_exact.hpp>

namespace sw { namespace universal {

	inline qd log(const qd& a) {
		if (a.isnan() || a.isinf()) return a;

		if (a.iszero()) return qd(SpecificValue::infneg);

		if (a.isone()) return 0.0;

        if (a[0] <= 0.0) {
			errno = EDOM;
            return qd(SpecificValue::qnan);
        }

		/* Strategy.  The Taylor series for log converges much more
		   slowly than that of exp, due to the lack of the factorial
		   term in the denominator.  Hence this routine instead tries
		   to determine the root of the function

			   f(x) = exp(x) - a

		   using Newton iteration.  The iteration is given by

			   x' = x - f(x)/f'(x)
				  = x - (1 - a * exp(-x))
				  = x + a * exp(-x) - 1.

		   Two iteration is needed, since Newton's iteration
		   approximately doubles the number of digits per iteration.
		 */

        // A huge or tiny a puts exp(-x) in the Newton steps below near the bottom of double's range,
        // where its low limbs are lost: log(1e300) kept 89 bits.  Take the binary exponent out first;
        // beyond 2^500 k ln 2 dominates the result, so nothing cancels (#1667).
        {
            const int k = std::ilogb(a[0]);
            if (k > 500 || k < -500) return log(ldexp(a, -k)) + multidouble::constant<qd>(multidouble::constants::ln2) * qd(static_cast<double>(k));
        }

        qd x = std::log(a[0]);   // Initial approximation

        x = x + a * exp(-x) - 1.0;
        x = x + a * exp(-x) - 1.0;
        x = x + a * exp(-x) - 1.0;

        return x;
    }

	/// <summary>
	/// binary logarithm (base = 2)
	/// </summary>
	/// <param name="a">input</param>
	/// <returns>binary logarithm of a</returns>
	inline qd log2(const qd& a) {
		if (a.isnan() || a.isinf()) return a;

		if (a.iszero()) return qd(SpecificValue::infneg);

		if (a.isone()) return 0.0;

		if (a.sign()) {
			errno = EDOM;
			return qd(SpecificValue::qnan);
		}

		return log(a) * qd_lge;
	}

	/// <summary>
	/// decimal logarithm (base = 10)
	/// </summary>
	/// <param name="a">input</param>
	/// <returns>binary logarithm of a</returns>
	inline qd log10(const qd& a) {
		if (a.isnan() || a.isinf()) return a;

		if (a.iszero()) return qd(SpecificValue::infneg);

		if (a.isone()) return 0.0;

		if (a.sign()) {
			errno = EDOM;
			return qd(SpecificValue::qnan);
		}


		return log(a) / qd_ln10;
	}

	/// <summary>
	/// Natural logarithm of 1+x
	/// </summary>
	/// <param name="a">input</param>
	/// <returns>binary logarithm of a</returns>
	inline qd log1p(const qd& a) {
		if (a.isnan() || a.isinf()) return a;

		if (a.iszero()) return qd(0.0);

		if (a == -1.0) return qd(SpecificValue::infneg);

		if (a < -1.0) {
			errno = EDOM;
			return qd(SpecificValue::qnan);
		}

		if (a.isinf()) return a;

		if ((a >= 2.0) || (a <= -0.5))			//	a >= 2.0 - no loss of significant bits - use log()
			return log(1.0 + a);

		// near 0, 1 + a would drop every limb of a below 1's last bit: log1p(1e-70) was 0 (#1667)
		if (std::fabs(a[0]) < 0.25) return multidouble::log1p_small(a);

		// at this point, -1.0 < a < 2.0 and |a| >= 1/4: 1 + a loses no relative accuracy
		return log(1.0 + a);
	}

}} // namespace sw::universal
