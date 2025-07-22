#pragma once
// hyperbolic.hpp: hyperbolic function support for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

#if !QUADDOUBLE_NATIVE_HYPERBOLIC
	// hyperbolic sine of an angle of x radians
	inline qd sinh(qd x) {
		return qd(std::sinh(double(x)));
	}

	// hyperbolic cosine of an angle of x radians
	inline qd cosh(qd x) {
		return qd(std::cosh(double(x)));
	}

	// hyperbolic tangent of an angle of x radians
	inline qd tanh(qd x) {
		return qd(std::tanh(double(x)));
	}

	// hyperbolic cotangent of an angle of x radians
	inline qd atanh(qd x) {
		return qd(std::atanh(double(x)));
	}

	// hyperbolic cosecant of an angle of x radians
	inline qd acosh(qd x) {
		return qd(std::acosh(double(x)));
	}

	// hyperbolic secant of an angle of x radians
	inline qd asinh(qd x) {
		return qd(std::asinh(double(x)));
	}

#else 

    inline qd sinh(const qd& a) {
        if (a.iszero()) return 0.0;

        if (abs(a) > 0.05) {
            qd ea = exp(a);
            return mul_pwr2(ea - reciprocal(ea), 0.5);
        }

        /* Since a is small, using the above formula gives
           a lot of cancellation.   So use Taylor series. */
        qd s = a;
        qd t = a;
        qd r = sqr(t);
        double m = 1.0;
        double thresh = std::abs(double(a) * qd_eps);

        do {
            m += 2.0;
            t *= r;
            t /= (m - 1) * m;

            s += t;
        } while (abs(t) > thresh);

        return s;
    }

    inline qd cosh(const qd& a) {
        if (a.iszero()) return 1.0;

        qd ea = exp(a);
        return mul_pwr2(ea + reciprocal(ea), 0.5);
    }

    inline qd tanh(const qd& a) {
        if (a.iszero()) return 0.0;

        if (std::abs(double(a)) > 0.05) {
            qd ea = exp(a);
            qd inv_ea = reciprocal(ea);
            return (ea - inv_ea) / (ea + inv_ea);
        }
        else {
            qd s, c;
            s = sinh(a);
            c = sqrt(1.0 + sqr(s));
            return s / c;
        }
    }

    inline void sincosh(const qd& a, qd& s, qd& c) {
        if (std::abs(double(a)) <= 0.05) {
            s = sinh(a);
            c = sqrt(1.0 + sqr(s));
        }
        else {
            qd ea = exp(a);
            qd inv_ea = reciprocal(ea);
            s = mul_pwr2(ea - inv_ea, 0.5);
            c = mul_pwr2(ea + inv_ea, 0.5);
        }
    }

    inline qd log(const qd&);
    inline qd asinh(const qd& a) {
        return log(a + sqrt(sqr(a) + 1.0));
    }

    inline qd acosh(const qd& a) {
        if (a < 1.0) {
            std::cerr << "(acosh): Argument out of domain\n";
            return qd(SpecificValue::snan);
        }

        return log(a + sqrt(sqr(a) - 1.0));
    }

    inline qd atanh(const qd& a) {
        if (abs(a) >= 1.0) {
            std::cerr << "(atanh): Argument out of domain\n";
            return qd(SpecificValue::snan);
        }

        return mul_pwr2(log((1.0 + a) / (1.0 - a)), 0.5);
    }

#endif

}} // namespace sw::universal
