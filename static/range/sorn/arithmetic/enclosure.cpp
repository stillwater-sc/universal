// enclosure.cpp: test suite runner verifying that SORN arithmetic encloses the exact result
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iomanip>
#include <universal/number/sorn/sorn.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// contains: is the real value v inside the sorn's interval
	template<typename SornType>
	bool contains(const SornType& s, double v) {
		const auto& i = s.sornIntVal;
		const bool aboveLower = (v > i.lowerBound) || (v == i.lowerBound && !i.lowerIsOpen);
		const bool belowUpper = (v < i.upperBound) || (v == i.upperBound && !i.upperIsOpen);
		return aboveLower && belowUpper;
	}

	// samples: finite points of an interval, its closed finite bounds among them
	template<typename Interval>
	std::vector<double> samples(const Interval& i) {
		std::vector<double> pts;
		const double lo = i.lowerBound, hi = i.upperBound;
		if (lo == hi) { pts.push_back(lo); return pts; }
		// stand in a finite stretch for an infinite bound (both bounds infinite: [-64, 64])
		const double flo = std::isinf(lo) ? (std::isinf(hi) ? -64.0 : hi - 64.0) : lo;
		const double fhi = std::isinf(hi) ? (std::isinf(lo) ?  64.0 : lo + 64.0) : hi;
		if (!i.lowerIsOpen && !std::isinf(lo)) pts.push_back(lo);
		for (double t : { 0.0625, 0.5, 0.9375 }) pts.push_back(flo + t * (fhi - flo));
		if (!i.upperIsOpen && !std::isinf(hi)) pts.push_back(hi);
		return pts;
	}

	// all contiguous spans of the lattice, or only its single intervals
	template<typename SornType>
	std::vector<SornType> operands(bool spans) {
		std::vector<SornType> ops;
		constexpr size_t N = SornType::sornBits;
		for (size_t i = 0; i < N; ++i) {
			for (size_t j = i; j < (spans ? N : i + 1); ++j) {
				std::bitset<N> bits;
				for (size_t b = i; b <= j; ++b) bits[b] = true;
				SornType s;
				s.setBits(bits);
				ops.push_back(s);
			}
		}
		return ops;
	}

	// every operator result must contain op(x, y) for every sample x of a and y of b. The operands
	// are const, which the binary operators did not accept before #1972.
	template<typename SornType>
	int VerifyEnclosure(char op, bool spans, bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const std::vector<SornType> ops = operands<SornType>(spans);
		for (const SornType& a : ops) {
			for (const SornType& b : ops) {
				SornType c;
				switch (op) {
				case '+': c = a + b; break;
				case '-': c = a - b; break;
				case '*': c = a * b; break;
				case '/': c = a / b; break;
				}
				for (double x : samples(a.sornIntVal)) {
					for (double y : samples(b.sornIntVal)) {
						if (op == '/' && y == 0.0) continue;
						double ref = 0;
						switch (op) {
						case '+': ref = x + y; break;
						case '-': ref = x - y; break;
						case '*': ref = x * y; break;
						case '/': ref = x / y; break;
						}
						// a value beyond the lattice saturates to its outermost interval
						ref = std::max(ref, double(SornType::datatype().front().lowerBound));
						ref = std::min(ref, double(SornType::datatype().back().upperBound));
						if (!contains(c, ref)) {
							++nrOfFailedTestCases;
							if (reportTestCases) std::cerr << "FAIL: " << a << ' ' << op << ' ' << b << " = " << c << " misses " << x << ' ' << op << ' ' << y << " = " << ref << '\n';
							if (nrOfFailedTestCases > 24) return nrOfFailedTestCases;
						}
					}
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// abs and negation enclose |x| and -x
	template<typename SornType>
	int VerifyUnaryEnclosure(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		for (const SornType& a : operands<SornType>(true)) {
			const SornType n = -a;
			const SornType m = abs(a);
			for (double x : samples(a.sornIntVal)) {
				if (!contains(n, -x) || !contains(m, std::fabs(x))) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: -" << a << " = " << n << ", abs = " << m << " for x = " << x << '\n';
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// NaN propagates through every operator; nan * x used to come out as zero
	template<typename SornType>
	int VerifyNaNPropagation(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const SornType nan(SpecificValue::qnan), x(1.25), zero(0);
		const SornType results[] = { nan + x, x + nan, nan - x, x - nan, nan * x, x * nan, nan * zero,
		                             nan / x, x / nan, nan + 1, 2.0 * nan, nan / 2.0f, abs(nan), -nan };
		for (const SornType& r : results) {
			if (!r.isnan()) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: NaN operand produced " << r << '\n';
			}
		}
		return nrOfFailedTestCases;
	}

	// scalar operands of any arithmetic type; a long used to be ambiguous between int, float and double
	template<typename SornType>
	int VerifyScalarOperands(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const SornType a(1.25);
		const SornType results[] = { a + 1L, 1L + a, a - 1u, 3ll - a, a * 2L, 2L * a, a / 2L, 4L / a };
		const double   refs[]    = { 2.25,   2.25,   0.25,   1.75,    2.5,    2.5,    0.625,  3.2 };
		for (size_t k = 0; k < sizeof(refs) / sizeof(refs[0]); ++k) {
			if (!contains(results[k], refs[k])) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: scalar case " << k << " = " << results[k] << " misses " << refs[k] << '\n';
			}
		}
		// multiplying an interval that reaches infinity by zero is zero, not NaN
		const SornType big(SpecificValue::infpos);
		if (!(big * 0).iszero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: " << big << " * 0 = " << (big * 0) << '\n';
		}
		// and the exact zero times an infinite scalar is zero, as zero * sorn(infpos) is; the scalar
		// path used to give NaN (#2010)
		const SornType zero(0), infneg(SpecificValue::infneg);
		constexpr double inf = std::numeric_limits<double>::infinity();
		const SornType products[] = { zero * inf, inf * zero, zero * -inf, zero * std::numeric_limits<float>::infinity(),
		                              zero * big, big * zero, zero * infneg };
		for (const SornType& p : products) {
			if (!p.iszero() || p != zero * big) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: 0 * inf = " << p << ", expected " << (zero * big) << '\n';
			}
		}
		return nrOfFailedTestCases;
	}

	// scalar operands that are not floats, just above and below the lattice points: the result
	// encloses the exact one. They used to be rounded to float, and the bound arithmetic to
	// nearest, so (0,0.5] + 1e-12 came out as (0,0.5], which misses 0.5 + 1e-12 (#2010).
	template<typename SornType>
	int VerifyInexactScalarEnclosure(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const long double lo = SornType::datatype().front().lowerBound;
		const long double hi = SornType::datatype().back().upperBound;
		auto check = [&](const SornType& c, long double ref, const SornType& a, char op, long double y) {
			ref = std::min(std::max(ref, lo), hi);   // a value beyond the lattice saturates
			const auto& i = c.sornIntVal;
			const long double l = i.lowerBound, u = i.upperBound;
			const bool inside = ((ref > l) || (ref == l && !i.lowerIsOpen)) && ((ref < u) || (ref == u && !i.upperIsOpen));
			if (!inside) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << a << ' ' << op << ' ' << y << " = " << c << " misses " << ref << '\n';
			}
		};
		const double scalars[] = { 1.0e-12, -1.0e-12, 1.0 + 1.0e-12, 1.0 - 1.0e-12, -(1.0 + 1.0e-12), 0.1, 3.0e-8, 2.0 - 1.0e-15 };
		for (const SornType& a : operands<SornType>(false)) {
			const auto& i = a.sornIntVal;
			// the closed finite bounds of a single interval are in it; use them as the operand x
			for (float xb : { i.lowerBound, i.upperBound }) {
				const long double x = xb;
				const bool member = std::isfinite(xb) && ((xb == i.lowerBound && !i.lowerIsOpen) || (xb == i.upperBound && !i.upperIsOpen));
				if (!member) continue;
				for (double y : scalars) {
					const long double ly = y;
					check(a + y, x + ly, a, '+', ly);
					check(a - y, x - ly, a, '-', ly);
					check(a * y, x * ly, a, '*', ly);
					check(a / y, x / ly, a, '/', ly);
					if (x != 0) check(y / a, ly / x, a, '\\', ly);   // y / a, reported as a \ y
				}
				// a 64-bit integer past 2^24 is not a float either
				const long long n = (1ll << 24) + 1;
				check(a * n, x * static_cast<long double>(n), a, '*', static_cast<long double>(n));
				check(a / n, x / static_cast<long double>(n), a, '/', static_cast<long double>(n));
			}
		}
		// the case of the sweep
		const SornType half(0.5);
		if (half + 1.0e-12 == half) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: " << half << " + 1e-12 = " << (half + 1.0e-12) << '\n';
		}
		return nrOfFailedTestCases;
	}

	// exact bound check for sorn op sorn (#2021). The image of two intervals under + - * / has its
	// bounds at sums, differences, products and quotients of the operand bounds. On a lattice
	// whose points are not short dyadic values, such as a linear one with 3, 5 or 7 steps, those
	// are not floats, and a bound rounded to nearest can land inside the image. The comparisons
	// are exact: a sum or product of two floats is exact in double, and the sign of r - n/d is
	// the sign of r*d - n times that of d, where r*d is exact in double and the sign of a double
	// difference is always right. The operands have finite bounds, and a divisor excludes zero.
	struct BoundValue { double num; double den; bool attained; };

	// sign of v - num/den, exactly, for den != 0
	inline int compareToQuotient(double v, double num, double den) {
		if (std::isinf(v)) return v > 0 ? 1 : -1;
		const double d = v * den - num;
		const int s = (d > 0) - (d < 0);
		return den > 0 ? s : -s;
	}

	template<typename SornType>
	int VerifyExactBounds(char op, bool spans, bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		std::vector<SornType> ops;
		for (const SornType& s : operands<SornType>(spans)) {
			const auto& i = s.sornIntVal;
			if (std::isfinite(i.lowerBound) && std::isfinite(i.upperBound)) ops.push_back(s);
		}
		for (const SornType& a : ops) {
			for (const SornType& b : ops) {
				const auto& x = a.sornIntVal;
				const auto& y = b.sornIntVal;
				if (op == '/' && !((y.lowerBound > 0 || (y.lowerBound == 0 && y.lowerIsOpen)) || (y.upperBound < 0 || (y.upperBound == 0 && y.upperIsOpen)))) continue;
				SornType c;
				std::vector<BoundValue> lower, upper;   // the candidates for the image's bounds
				switch (op) {
				case '+':
					c = a + b;
					lower.push_back({ double(x.lowerBound) + double(y.lowerBound), 1.0, !x.lowerIsOpen && !y.lowerIsOpen });
					upper.push_back({ double(x.upperBound) + double(y.upperBound), 1.0, !x.upperIsOpen && !y.upperIsOpen });
					break;
				case '-':
					c = a - b;
					lower.push_back({ double(x.lowerBound) - double(y.upperBound), 1.0, !x.lowerIsOpen && !y.upperIsOpen });
					upper.push_back({ double(x.upperBound) - double(y.lowerBound), 1.0, !x.upperIsOpen && !y.lowerIsOpen });
					break;
				case '*':
				case '/': {
					c = (op == '*') ? a * b : a / b;
					// a zero divisor bound is open here: it stands for the limit, which an
					// unbounded quotient approaches; skip that corner
					for (int p = 0; p < 2; ++p) {
						for (int q = 0; q < 2; ++q) {
							const double xb = p ? x.upperBound : x.lowerBound, yb = q ? y.upperBound : y.lowerBound;
							const bool closed = !(p ? x.upperIsOpen : x.lowerIsOpen) && !(q ? y.upperIsOpen : y.lowerIsOpen);
							if (op == '*') { lower.push_back({ xb * yb, 1.0, closed }); upper.push_back(lower.back()); }
							else if (yb != 0) { lower.push_back({ xb, yb, closed }); upper.push_back(lower.back()); }
						}
					}
					break;
				}
				}
				const auto& r = c.sornIntVal;
				if (c.isnan()) continue;
				// a value beyond the lattice saturates to its outermost interval
				const double minV = SornType::datatype().front().lowerBound, maxV = SornType::datatype().back().upperBound;
				bool ok = true;
				for (const BoundValue& v : lower) {
					if (compareToQuotient(maxV, v.num, v.den) < 0) continue;
					const int cmp = compareToQuotient(r.lowerBound, v.num, v.den);
					if (cmp > 0 || (cmp == 0 && r.lowerIsOpen && v.attained)) ok = false;
				}
				for (const BoundValue& v : upper) {
					if (compareToQuotient(minV, v.num, v.den) > 0) continue;
					const int cmp = compareToQuotient(r.upperBound, v.num, v.den);
					if (cmp < 0 || (cmp == 0 && r.upperIsOpen && v.attained)) ok = false;
				}
				if (!ok) {
					++nrOfFailedTestCases;
					if (reportTestCases && nrOfFailedTestCases <= 10) {
						std::cerr << std::setprecision(9) << "FAIL: " << a << ' ' << op << ' ' << b << " = " << c << " does not enclose the image of the bounds\n";
					}
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// tightness of sorn op sorn (#2060): every lattice interval the result covers must meet the exact
	// image of the operands. Enclosure alone passed when the division, through an outward-rounded
	// reciprocal, rounded twice and widened bounds that are lattice points ([-4,-3.5) / [-3.5,-3) was
	// (0.5,1.5], the image is (1, 4/3]). The operands have finite bounds, and a divisor excludes zero.
	template<typename SornType>
	int VerifyTightBounds(char op, bool spans, bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		std::vector<SornType> ops;
		for (const SornType& s : operands<SornType>(spans)) {
			const auto& i = s.sornIntVal;
			if (std::isfinite(i.lowerBound) && std::isfinite(i.upperBound)) ops.push_back(s);
		}
		const auto& lattice = SornType::datatype();
		for (const SornType& a : ops) {
			for (const SornType& b : ops) {
				const auto& x = a.sornIntVal;
				const auto& y = b.sornIntVal;
				if (op == '/' && !((y.lowerBound > 0 || (y.lowerBound == 0 && y.lowerIsOpen)) || (y.upperBound < 0 || (y.upperBound == 0 && y.upperIsOpen)))) continue;
				const SornType c = (op == '*') ? a * b : a / b;
				if (c.isnan()) continue;
				// the image's bounds: the extreme corners, exactly, with whether each is attained; a zero
				// divisor bound stands for an unbounded limit
				bool unboundedBelow = false, unboundedAbove = false, first = true;
				BoundValue lo{ 0, 1, false }, hi{ 0, 1, false };
				for (int p = 0; p < 2; ++p) {
					for (int q = 0; q < 2; ++q) {
						const double xb = p ? x.upperBound : x.lowerBound, yb = q ? y.upperBound : y.lowerBound;
						const bool closed = !(p ? x.upperIsOpen : x.lowerIsOpen) && !(q ? y.upperIsOpen : y.lowerIsOpen);
						BoundValue v{ xb, 1.0, closed };
						if (op == '*') v.num = xb * yb;
						else if (yb == 0) {
							if (xb == 0) continue;
							const bool posInf = (xb > 0) == (q == 0);   // a zero lower bound: y -> 0+
							(posInf ? unboundedAbove : unboundedBelow) = true;
							continue;
						}
						else { v.den = yb; if (xb == 0) v.attained = !(p ? x.upperIsOpen : x.lowerIsOpen); }
						if (first) { lo = v; hi = v; first = false; continue; }
						// exact: v < lo  <=>  v.num lo.den < lo.num v.den for a positive den product; the products
						// of two floats are exact in double
						const double lhs = v.num * lo.den * (v.den * lo.den > 0 ? 1 : -1);
						const double rhs = lo.num * v.den * (v.den * lo.den > 0 ? 1 : -1);
						if (lhs < rhs || (lhs == rhs && v.attained)) lo = v;
						const double lhs2 = v.num * hi.den * (v.den * hi.den > 0 ? 1 : -1);
						const double rhs2 = hi.num * v.den * (v.den * hi.den > 0 ? 1 : -1);
						if (lhs2 > rhs2 || (lhs2 == rhs2 && v.attained)) hi = v;
					}
				}
				for (const auto& I : lattice) {
					const double mid = std::isinf(I.lowerBound) ? I.upperBound - 1.0 : (std::isinf(I.upperBound) ? I.lowerBound + 1.0 : 0.5 * (double(I.lowerBound) + double(I.upperBound)));
					if (!contains(c, mid)) continue;                    // not covered by the result
					// does I meet the image? I above the image's upper bound, or below its lower one, fails
					bool below = false, above = false;
					if (!first && !unboundedAbove) {
						const int cmp = compareToQuotient(I.lowerBound, hi.num, hi.den);   // I.lower vs the image's top
						above = (cmp > 0) || (cmp == 0 && (I.lowerIsOpen || !hi.attained));
					}
					if (!first && !unboundedBelow) {
						const int cmp = compareToQuotient(I.upperBound, lo.num, lo.den);
						below = (cmp < 0) || (cmp == 0 && (I.upperIsOpen || !lo.attained));
					}
					if (below || above) {
						++nrOfFailedTestCases;
						if (reportTestCases && nrOfFailedTestCases <= 10) {
							std::cerr << std::setprecision(9) << "FAIL: " << a << ' ' << op << ' ' << b << " = " << c << " covers a lattice interval outside the exact image\n";
						}
						break;
					}
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// the outward-rounding helpers on the branches a lattice rarely reaches: a sum that overflows the
	// float range, and quotients by a zero or of an infinite bound
	inline int VerifyBoundHelpers(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		constexpr float fmax = std::numeric_limits<float>::max(), inf = std::numeric_limits<float>::infinity();
		auto check = [&](bool ok, const char* what) {
			if (!ok) { ++nrOfFailedTestCases; if (reportTestCases) std::cerr << "FAIL: " << what << '\n'; }
		};
		bool inexact = false;
		// the exact sum 2 FLT_MAX is finite: rounded down it is FLT_MAX, rounded up inf, inexact both ways
		check(sornAddOut(fmax, fmax, false, inexact) == fmax && inexact, "sornAddOut(max, max) down is not max, inexact");
		check(sornAddOut(fmax, fmax, true, inexact) == inf && inexact, "sornAddOut(max, max) up is not inf, inexact");
		check(sornAddOut(-fmax, -fmax, true, inexact) == -fmax && inexact, "sornAddOut(-max, -max) up is not -max, inexact");
		check(sornAddOut(inf, 1.0f, false, inexact) == inf && !inexact, "sornAddOut(inf, 1) is not an exact inf");
		check(sornAddOut(1.0f, 2.0f, true, inexact) == 3.0f && !inexact, "sornAddOut(1, 2) is not an exact 3");
		// quotients: a zero divisor stands for the limit from above, an infinite operand is exact
		check(sornDivOut(1.0f, 0.0f, true, inexact) == inf && !inexact, "sornDivOut(1, 0) is not inf");
		check(sornDivOut(-1.0f, 0.0f, false, inexact) == -inf && !inexact, "sornDivOut(-1, 0) is not -inf");
		check(sornDivOut(inf, 2.0f, true, inexact) == inf && !inexact, "sornDivOut(inf, 2) is not inf");
		check(sornDivOut(3.0f, inf, false, inexact) == 0.0f && !inexact, "sornDivOut(3, inf) is not 0");
		check(sornDivOut(0.0f, 3.0f, false, inexact) == 0.0f && !inexact, "sornDivOut(0, 3) is not 0");
		check(sornDivOut(1.0f, 3.0f, false, inexact) < sornDivOut(1.0f, 3.0f, true, inexact) && inexact, "sornDivOut(1, 3) does not straddle 1/3");
		return nrOfFailedTestCases;
	}

	// the sweep's repro values (#2060): bounds of an exact quotient that are lattice points stay put
	template<typename SornType>
	int VerifyDivisionRepro(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		auto interval = [](float lo, float hi, bool loOpen, bool hiOpen) {
			SornType s;
			s.sornIntVal = { lo, hi, loOpen, hiOpen };
			return s;
		};
		const SornType a = interval(-4.0f, -3.5f, false, true);
		const SornType q1 = a / interval(-3.5f, -3.0f, false, true);   // the image is (1, 4/3]
		const SornType q2 = a / interval(1.0f, 3.5f, true, false);     // the image is [-4, -1)
		const auto& r1 = q1.sornIntVal;
		const auto& r2 = q2.sornIntVal;
		if (!(r1.lowerBound == 1.0f && r1.upperBound == 1.5f)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: [-4,-3.5) / [-3.5,-3) = " << q1 << " expected (1,1.5]\n";
		}
		if (!(r2.lowerBound == -4.0f && r2.upperBound == -1.0f)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: [-4,-3.5) / (1,3.5] = " << q2 << " expected [-4,-1)\n";
		}
		return nrOfFailedTestCases;
	}

	// 1 / y through the scalar path: the exact 1, so the reciprocal's own rounding is all there is.
	// Its bounds are 1 / (the bounds of y), which are not floats for most lattice points (#2021).
	template<typename SornType>
	int VerifyReciprocalBounds(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		for (const SornType& y : operands<SornType>(true)) {
			const auto& i = y.sornIntVal;
			if (!std::isfinite(i.lowerBound) || !std::isfinite(i.upperBound)) continue;
			const bool positive = i.lowerBound > 0 || (i.lowerBound == 0 && i.lowerIsOpen);
			const bool negative = i.upperBound < 0 || (i.upperBound == 0 && i.upperIsOpen);
			if (!positive && !negative) continue;
			const SornType c = 1.0f / y;
			const auto& r = c.sornIntVal;
			const double maxV = SornType::datatype().back().upperBound, minV = SornType::datatype().front().lowerBound;
			bool ok = true;
			// 1/hi is the lower bound and 1/lo the upper one; a zero bound stands for an unbounded side
			if (i.upperBound != 0 && compareToQuotient(maxV, 1.0, i.upperBound) >= 0) {
				const int cmp = compareToQuotient(r.lowerBound, 1.0, i.upperBound);
				if (cmp > 0 || (cmp == 0 && r.lowerIsOpen && !i.upperIsOpen)) ok = false;
			}
			if (i.lowerBound != 0 && compareToQuotient(minV, 1.0, i.lowerBound) <= 0) {
				const int cmp = compareToQuotient(r.upperBound, 1.0, i.lowerBound);
				if (cmp < 0 || (cmp == 0 && r.upperIsOpen && !i.lowerIsOpen)) ok = false;
			}
			if (!ok) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 10) std::cerr << std::setprecision(9) << "FAIL: 1 / " << y << " = " << c << " does not enclose the reciprocal\n";
			}
		}
		return nrOfFailedTestCases;
	}

}}  // namespace sw::universal

#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "sorn arithmetic enclosure validation";
	std::string test_tag    = "enclosure";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using SmallSorn  = sorn<0, 2, 2>;                   // linear, 7 intervals
	using LinSorn    = sorn<0, 4, 8>;                   // linear, 19 intervals
	using LogSorn    = sorn<-2, 2, 1, 0, 1, 1, 1, 1>;   // logarithmic
	using NoZeroSorn = sorn<0, 2, 2, 1, 1, 1, 1, 0>;    // linear, no exact zero
	using SeventhsSorn = sorn<0, 3, 7>;                 // linear, points k * 3/7: not dyadic
	using FifthsSorn   = sorn<0, 1, 5>;                 // linear, points k * 0.2: not dyadic
	using ThirdsSorn   = sorn<0, 6, 18>;                // linear, points k / 3: 1/3 and 3 are both points

#if MANUAL_TESTING

	const LinSorn a(1.25), b(-0.75);
	std::cout << a << " / " << b << " = " << (a / b) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	for (char op : { '+', '-', '*', '/' }) {
		const std::string tag = std::string("enclosure ") + op;
		nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<SmallSorn>(op, true, reportTestCases), "sorn<0,2,2> spans", tag);
		nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<NoZeroSorn>(op, true, reportTestCases), "sorn<0,2,2,nozero> spans", tag);
		nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<LinSorn>(op, false, reportTestCases), "sorn<0,4,8>", tag);
		nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<LogSorn>(op, false, reportTestCases), "sorn<-2,2,1,log>", tag);
	}
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryEnclosure<LinSorn>(reportTestCases), "sorn<0,4,8>", "negate and abs");
	nrOfFailedTestCases += ReportTestResult(VerifyNaNPropagation<LinSorn>(reportTestCases), "sorn<0,4,8>", "nan propagation");
	nrOfFailedTestCases += ReportTestResult(VerifyScalarOperands<LinSorn>(reportTestCases), "sorn<0,4,8>", "scalar operands");
	nrOfFailedTestCases += ReportTestResult(VerifyInexactScalarEnclosure<LinSorn>(reportTestCases), "sorn<0,4,8>", "inexact scalar enclosure");
	nrOfFailedTestCases += ReportTestResult(VerifyInexactScalarEnclosure<LogSorn>(reportTestCases), "sorn<-2,2,1,log>", "inexact scalar enclosure");
	nrOfFailedTestCases += ReportTestResult(VerifyInexactScalarEnclosure<NoZeroSorn>(reportTestCases), "sorn<0,2,2,nozero>", "inexact scalar enclosure");
	// lattices whose points are not short dyadic values: their bound sums, products and
	// reciprocals are not floats (#2021)
	for (char op : { '+', '-', '*', '/' }) {
		const std::string tag = std::string("exact bounds ") + op;
		nrOfFailedTestCases += ReportTestResult(VerifyExactBounds<SeventhsSorn>(op, true, reportTestCases), "sorn<0,3,7> spans", tag);
		nrOfFailedTestCases += ReportTestResult(VerifyExactBounds<FifthsSorn>(op, true, reportTestCases), "sorn<0,1,5> spans", tag);
		nrOfFailedTestCases += ReportTestResult(VerifyExactBounds<LinSorn>(op, true, reportTestCases), "sorn<0,4,8> spans", tag);
		nrOfFailedTestCases += ReportTestResult(VerifyExactBounds<LogSorn>(op, true, reportTestCases), "sorn<-2,2,1,log> spans", tag);
	}
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocalBounds<SeventhsSorn>(reportTestCases), "sorn<0,3,7> spans", "exact bounds 1/y");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocalBounds<FifthsSorn>(reportTestCases), "sorn<0,1,5> spans", "exact bounds 1/y");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocalBounds<ThirdsSorn>(reportTestCases), "sorn<0,6,18> spans", "exact bounds 1/y");
	nrOfFailedTestCases += ReportTestResult(VerifyExactBounds<ThirdsSorn>('/', true, reportTestCases), "sorn<0,6,18> spans", "exact bounds /");
	// tightness: the bounds of the exact image that are lattice points are not widened (#2060)
	for (char op : { '*', '/' }) {
		const std::string tag = std::string("tight bounds ") + op;
		nrOfFailedTestCases += ReportTestResult(VerifyTightBounds<LinSorn>(op, false, reportTestCases), "sorn<0,4,8>", tag);
		nrOfFailedTestCases += ReportTestResult(VerifyTightBounds<SeventhsSorn>(op, true, reportTestCases), "sorn<0,3,7> spans", tag);
		nrOfFailedTestCases += ReportTestResult(VerifyTightBounds<LogSorn>(op, true, reportTestCases), "sorn<-2,2,1,log> spans", tag);
	}
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionRepro<LinSorn>(reportTestCases), "sorn<0,4,8>", "division repro");
	nrOfFailedTestCases += ReportTestResult(VerifyBoundHelpers(reportTestCases), "sorn", "outward-rounding helpers");
#endif

#if REGRESSION_LEVEL_2
	for (char op : { '+', '-', '*', '/' }) {
		const std::string tag = std::string("enclosure ") + op;
		nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<LinSorn>(op, true, reportTestCases), "sorn<0,4,8> spans", tag);
		nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<LogSorn>(op, true, reportTestCases), "sorn<-2,2,1,log> spans", tag);
	}
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
