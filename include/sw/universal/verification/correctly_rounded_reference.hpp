#pragma once
// correctly_rounded_reference.hpp : a reference for a binary operator, computed through double
// and rounded once to the type under test
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <limits>

#include <universal/internal/expansion/expansion_ops.hpp>   // two_sum, two_prod: the exact error

namespace sw { namespace universal {

	// The random verifiers compute a reference as double's correctly rounded result, converted
	// to TestType: two roundings. For a TestType of p < 53 bits that goes wrong when 2p+1 > 53
	// (cfloat<48,8>, posit<48,2>): the first rounding can land exactly on a TestType midpoint the
	// exact result is not on, and the second then breaks a tie that was never there (#1861).
	// The sign of exact - reference is computable exactly, so it says which side of that
	// midpoint the exact result lies on, and the reference is rounded once.
	//
	// Every midpoint of a p < 53 bit type is a double, and its neighbours are at least two double
	// ulps apart, so only a result exactly on a midpoint can round differently from the exact one.
	// That holds for a type that rounds to the nearest value (cfloat, bfloat16, posit wherever it
	// has a fraction bit) and whose values are doubles, as the verifiers' double operands already
	// assume; a type whose rounding boundaries are not midpoints, such as lns, would need its own.
	// Wider types (c64, dd, qd, posit<64,x>) keep the converted double.
	namespace detail_reference {

		// sign of the exact a op b minus reference, op one of '*' and '/', for finite nonzero a, b.
		// Scaling both operands to [1,2) keeps every error term clear of underflow, also when
		// reference is subnormal or zero.
		inline double scaledErrorSign(char op, double a, double b, double reference) {
			const int ka = std::ilogb(a), kb = std::ilogb(b);
			const double a1 = std::ldexp(a, -ka), b1 = std::ldexp(b, -kb);
			double r1{ 0 }, e{ 0 };   // exact * 2^-k == r1 + e, |e| <= ulp(r1)/2 in sign
			int k{ 0 };
			if (op == '*') {
				expansion_ops::two_prod(a1, b1, r1, e);
				k = ka + kb;
			}
			else {
				r1 = a1 / b1;
				double prod{ 0 }, perr{ 0 };
				expansion_ops::two_prod(r1, b1, prod, perr);
				const double residual = (a1 - prod) - perr;   // a1 - r1*b1, exact
				e = (b1 > 0.0) ? residual : -residual;         // the sign of a1/b1 - r1
				k = ka - kb;
			}
			// reference is r1 * 2^k rounded to double's grid there: the same as r1 unless that grid
			// is coarser (subnormal or zero). Then the difference, exact by Sterbenz, is a nonzero
			// multiple of ulp(r1) and outweighs e.
			const double d = r1 - std::ldexp(reference, -k);
			return (d != 0.0) ? d : e;
		}

	}  // namespace detail_reference

	// op is '+', '-', '*' or '/'; any other leaves the converted double.
	template<typename TestType>
	TestType correctlyRoundedReference(char op, double da, double db, double reference) {
		TestType ref(reference);
		if constexpr (std::numeric_limits<TestType>::is_specialized &&
		              std::numeric_limits<TestType>::digits < std::numeric_limits<double>::digits) {
			if (!std::isfinite(reference) || !std::isfinite(da) || !std::isfinite(db)) return ref;
			double sum{ 0 }, err{ 0 };
			switch (op) {
			case '+':
				expansion_ops::two_sum(da, db, sum, err);   // exact for every finite sum
				break;
			case '-':
				expansion_ops::two_sum(da, -db, sum, err);
				break;
			case '*':
			case '/':
				if (da == 0.0 || db == 0.0) return ref;
				err = detail_reference::scaledErrorSign(op, da, db, reference);
				break;
			default:
				return ref;
			}
			if (err == 0.0) return ref;
			constexpr double inf = std::numeric_limits<double>::infinity();
			const TestType below(std::nextafter(reference, -inf));
			const TestType above(std::nextafter(reference, inf));
			if (below == above) return ref;
			// the midpoint, exactly: hi - lo spans at least two double ulps, and lo + hi would
			// overflow in the top binade
			const double lo = double(below), hi = double(above);
			if (!std::isfinite(lo) || !std::isfinite(hi) || lo + (hi - lo) / 2.0 != reference) return ref;
			return (err > 0.0) ? above : below;
		}
		return ref;
	}

}} // namespace sw::universal
