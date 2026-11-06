#pragma once
// exercise.hpp: the operations each number system must provide in a build without exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Every translation unit of this test is compiled with -fno-exceptions (#1925). Including a
// header is not enough to show that a number system builds that way: a `throw` inside a
// template whose operand depends on a template parameter is only diagnosed when the template is
// instantiated. So each translation unit instantiates the arithmetic, the comparisons, the
// conversions and the stream output and input of a representative configuration, and runs them, which
// also shows that the default error paths, such as division by zero, neither throw nor abort.
#include <cstdio>
#include <istream>
#include <sstream>
#include <string>
#include <vector>

namespace noexceptions {

// report a failed check and count it
inline int fail(const char* tag, const char* what) {
	std::printf("FAIL %-12s : %s\n", tag, what);
	return 1;
}

// the operations of a real-valued number system, from values every exact configuration holds
// exactly (a logarithmic system does not, and checks only the ordering):
// ((1.5 + 0.75 - 0.75) * 1.5 / 0.75 + 1.5 - 0.75) * 1.5 / 0.75 = 7.5
// Input = false for a number system whose stream input is declared but does not work;
// RequireInput = true for one whose stream input must accept decimal text such as "1.5"
template<typename Real, bool Input = true, bool RequireInput = false>
int exercise(const char* tag, bool exact = true) {
	int fails = 0;
	Real a(1.5), b(0.75f), zero(0);
	Real c = a + b;
	c = c - b;
	c = c * a;
	c = c / b;
	c += a;
	c -= b;
	c *= a;
	c /= b;
	if (exact && double(c) != 7.5) fails += fail(tag, "arithmetic");
	if (!(b < a && a > b && b <= a && a >= b && a != b && !(a == b))) fails += fail(tag, "comparison");
	Real n(-3);
	n = -n;
	if (exact && float(a) != 1.5f) fails += fail(tag, "conversion to float");
	if (exact && double(n) != 3.0) fails += fail(tag, "negation or conversion from int");
	// the error path a build without exceptions must still take quietly
	Real q = a / zero;
	std::ostringstream ostr;
	ostr << c << ' ' << q;
	if (ostr.str().empty()) fails += fail(tag, "stream output");
	// not every number system has stream input yet
	if constexpr (Input && requires(std::istream& is, Real& r) { is >> r; }) {
		std::istringstream istr("1.5");
		Real e(0);
		istr >> e;
		if (RequireInput && istr.fail()) fails += fail(tag, "stream input refused 1.5");
		// a parse may refuse the token, but one that succeeds must produce the value
		if (exact && !istr.fail() && double(e) != 1.5) fails += fail(tag, "stream input");
	}
	return fails;
}

// the fused dot product, whose argument checks used to throw unconditionally (#1925):
// 1.5 * 0.75 + 0.75 * 1.5 = 2.25
template<typename Real>
int exercise_fdp(const char* tag, bool exact = true) {
	int fails = 0;
	std::vector<Real> x = { Real(1.5), Real(0.75) }, y = { Real(0.75), Real(1.5) };
	Real r = fdp(x, y);
	if (exact && double(r) != 2.25) fails += fail(tag, "fused dot product");
	return fails;
}

// the operations of an integer number system:
// ((12 + 4 - 4) * 12 / 4 + 12 - 4) * 12 / 4 = 132
template<typename Integer>
int exercise_integer(const char* tag) {
	int fails = 0;
	Integer a(12), b(4), zero(0);
	Integer c = a + b;
	c = c - b;
	c = c * a;
	c = c / b;
	c += a;
	c -= b;
	c *= a;
	c /= b;
	if (long(c) != 132) fails += fail(tag, "arithmetic");
	if (long(c % Integer(10)) != 2) fails += fail(tag, "remainder");
	if (!(b < a && a > b && b <= a && a >= b && a != b && !(a == b))) fails += fail(tag, "comparison");
	// division by zero has a non-throwing result when exceptions are off (#1894, #1925)
	Integer q = a / zero;
	Integer r = a % zero;
	std::ostringstream ostr;
	ostr << c << ' ' << q << ' ' << r;
	if (ostr.str().empty()) fails += fail(tag, "stream output");
	// not every number system has stream input yet
	if constexpr (requires(std::istream& is, Integer& r) { is >> r; }) {
		std::istringstream istr("12");
		Integer e(0);
		istr >> e;
		if (!istr.fail() && long(e) != 12) fails += fail(tag, "stream input");
	}
	return fails;
}

} // namespace noexceptions
