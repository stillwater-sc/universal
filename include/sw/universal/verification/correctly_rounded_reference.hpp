#pragma once
// correctly_rounded_reference.hpp : a reference for a binary operator, computed through double
// and rounded once to the type under test
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <algorithm>
#include <cmath>
#include <limits>

#include <universal/internal/expansion/expansion_ops.hpp>          // two_sum, two_prod: the exact error
#include <universal/number/shared/specific_value_encoding.hpp>   // SpecificValue: maxpos, inf

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

	// A type whose range reaches past double's -- cfloat<64,11> with max-exponent values, or
	// es > 11 -- has finite operands and results that double holds only as inf, or as zero or a
	// rounded subnormal (#1863). Its reference is computed at a scale double can hold: each
	// operand is m * 2^e with m in [0.5,1) (frexp, exact), the operation runs on the fractions,
	// and the exponent goes back on afterwards.
	//   - In the normal range the type's precision does not depend on scale, so the result is
	//     rounded once to it at that scale (correctlyRoundedReference) and rescaled exactly.
	//   - Below it every value is a multiple of the subnormal quantum 2^q, so the result is
	//     rounded to the nearest integer multiple of 2^q directly: a sum there is exact, and a
	//     product or quotient breaks its ties with the sign of its exact error.
	// The type's ldexp only encodes a value that is already exact; one it cannot hold finitely
	// is an overflow, which becomes inf, or maxpos for a saturating type.
	//
	// This needs a type with frexp and ldexp whose precision is fixed across its normal range,
	// such as cfloat; a tapered type such as posit is not one.
	template<typename TestType>
	inline constexpr bool has_scaled_reference =
		std::numeric_limits<TestType>::is_specialized &&
		std::numeric_limits<TestType>::digits <= std::numeric_limits<double>::digits &&
		requires(const TestType& a, int* e, int k) {
			frexp(a, e);
			ldexp(a, k);
			a.isinf();
			a.isnan();
			a.iszero();
		};

	namespace detail_reference {

		// ref = m * 2^e, m in [0.5,1) holding at most TestType's precision. A value the type
		// cannot hold finitely and exactly -- ldexp gives inf, or a value that does not come
		// back through frexp -- is an overflow: inf, or maxpos for a saturating type, which is
		// also what a type gives whose maxpos shares its encoding with inf (saturating cfloat
		// with max-exponent values).
		template<typename TestType>
		void encodeScaled(double m, int e, TestType& ref) {
			const TestType y = ldexp(TestType(m), e);
			if (!y.isinf() && !y.isnan()) {
				int ey{ 0 };
				if (double(frexp(y, &ey)) == m && ey == e) { ref = y; return; }
			}
			bool saturate = false;
			if constexpr (requires { TestType::isSaturating; }) saturate = TestType::isSaturating;
			if (saturate) ref = TestType(m > 0.0 ? SpecificValue::maxpos : SpecificValue::maxneg);
			else          ref = TestType(m > 0.0 ? SpecificValue::infpos : SpecificValue::infneg);
		}

		// round y_exact = y + (a tiny err) to the nearest integer, ties to even, for |y| < 2^53
		inline double roundToInteger(double y, double err) {
			const double n = std::nearbyint(y);
			if (err != 0.0 && std::fabs(y - std::trunc(y)) == 0.5) {
				return (err > 0.0) ? std::floor(y) + 1.0 : std::floor(y);
			}
			return n;
		}

		// a op b, for a result below the smallest normal, in units of the subnormal quantum 2^q
		// and rounded to the nearest integer; a = ma * 2^ea, b = mb * 2^eb, the result's exponent
		// is shift + q
		inline double subnormalUnits(char op, double ma, int ea, double mb, int eb, int shift, int q) {
			if (op == '+' || op == '-') {
				// both operands are integer multiples of 2^q and so is their sum, which is small
				const double sb = (op == '-') ? -mb : mb;
				return std::ldexp(ma, ea - q) + std::ldexp(sb, eb - q);
			}
			if (shift <= -1000) return 0.0;
			const double a2 = std::ldexp(ma, shift);
			const double y = (op == '*') ? a2 * mb : a2 / mb;
			if (std::fabs(y) < 0.25) return 0.0;   // the nearest integer is 0
			double err{ 0 };
			if (op == '*') {
				double p{ 0 };
				expansion_ops::two_prod(a2, mb, p, err);
			}
			else {
				double p{ 0 }, perr{ 0 };
				expansion_ops::two_prod(y, mb, p, perr);
				const double residual = (a2 - p) - perr;
				err = (mb > 0.0) ? residual : -residual;
			}
			return roundToInteger(y, err);
		}

	}  // namespace detail_reference

	// ref = a op b rounded once to TestType through the scaled computation above; false when it
	// does not apply (an operand inf, NaN or zero, a result of a type without subnormals that
	// rounded to exactly its smallest normal, an op other than '+', '-', '*', '/'), and ref is
	// then left alone
	template<typename TestType>
	bool scaledReference(char op, const TestType& a, const TestType& b, TestType& ref) {
		if constexpr (has_scaled_reference<TestType>) {
			using limits = std::numeric_limits<TestType>;
			if (a.isinf() || b.isinf() || a.isnan() || b.isnan() || a.iszero() || b.iszero()) return false;
			int ea{ 0 }, eb{ 0 };
			const double ma = double(frexp(a, &ea)), mb = double(frexp(b, &eb));
			if (!std::isfinite(ma) || !std::isfinite(mb)) return false;
			const double sb = (op == '-') ? -mb : mb;

			// the normal range: round at the operands' scale, then put the exponent back
			int e0{ 0 };
			TestType t{ 0 };
			switch (op) {
			case '*':
				t = correctlyRoundedReference<TestType>('*', ma, mb, ma * mb);
				e0 = ea + eb;
				break;
			case '/':
				t = correctlyRoundedReference<TestType>('/', ma, mb, ma / mb);
				e0 = ea - eb;
				break;
			case '+':
			case '-': {
				e0 = std::max(ea, eb);
				// an operand below 2^-1000 of the other is under half its ulp: the sum is the other
				if (ea - e0 < -1000) { ref = (op == '-') ? -b : b; return true; }
				if (eb - e0 < -1000) { ref = a; return true; }
				const double xa = std::ldexp(ma, ea - e0), xb = std::ldexp(sb, eb - e0);   // exact
				if (xa + xb == 0.0) { ref = TestType(0.0); return true; }
				t = correctlyRoundedReference<TestType>('+', xa, xb, xa + xb);
				break;
			}
			default:
				return false;
			}
			int et{ 0 };
			const double mt = double(frexp(t, &et));
			// At least twice the smallest normal, the exact result is normal too. In the binade
			// above the smallest normal it is as well, except when it rounded up to that smallest
			// normal: a type with subnormals takes that binade on the subnormal grid below, which
			// has the same spacing there, and a type without them is left alone.
			bool normal = e0 + et > limits::min_exponent;
			if constexpr (limits::has_denorm != std::denorm_present) {
				normal = normal || (e0 + et == limits::min_exponent && std::fabs(mt) != 0.5);
			}
			if (normal) {
				detail_reference::encodeScaled(mt, e0 + et, ref);
				return true;
			}

			// below it, a type without subnormals flushes to zero, in conversion and arithmetic
			// alike; one that rounded up to exactly the smallest normal may have come from
			// below it, and is left alone
			if constexpr (limits::has_denorm != std::denorm_present) {
				if (e0 + et < limits::min_exponent) { ref = TestType(mt < 0.0 ? -0.0 : 0.0); return true; }
				return false;
			}
			// with subnormals: the nearest multiple of the subnormal quantum 2^q
			else {
				constexpr int q = limits::min_exponent - limits::digits;
				const double n = detail_reference::subnormalUnits(op, ma, ea, mb, eb, e0 - q, q);
				if (n == 0.0) {
					const bool negative = (op == '+' || op == '-') ? false : ((ma < 0.0) != (mb < 0.0));
					ref = TestType(negative ? -0.0 : 0.0);
					return true;
				}
				ref = ldexp(TestType(n), q);   // n has at most digits bits: exact
				return true;
			}
		}
		else {
			(void)op; (void)a; (void)b; (void)ref;
			return false;
		}
	}

}} // namespace sw::universal
