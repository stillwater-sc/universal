//  mathlib.cpp : mathlib performance of the multi-component types: dd, dd_cascade, td_cascade, qd, qd_cascade
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// issue #1315: both families ship a complete mathlib (math/functions), so function cost is
// directly comparable. This is where an implementation difference shows up most clearly: a
// transcendental is an argument reduction plus a polynomial, and it inherits whatever the
// underlying add/multiply/divide costs, amplified by the number of terms.
//
// Measurement note: each function is measured inside an accumulation loop, because the result has
// to be consumed for the call to survive the optimizer. The 'accumulate only' row measures that
// same loop with the function call removed, so the accumulation overhead can be subtracted:
//
//     cost(f) ~= reported(f) - reported(accumulate only)
//
// For double that overhead is a meaningful fraction of a sqrt; for qd it is noise next to an exp.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "hp_types.hpp"

namespace {

	constexpr std::size_t TABLE_SIZE = 64;   // power of two: the index wrap is a mask, not a division

	// arguments in [0.5, 2.0): inside the domain of log() and sqrt(), and small enough that exp()
	// stays far from overflow
	template<typename Scalar>
	const std::vector<Scalar>& arguments() {
		static const std::vector<Scalar> table = [] {
			std::vector<double> data = hpbench::sampleData(TABLE_SIZE, 0x8888ull);
			std::vector<Scalar> t(TABLE_SIZE);
			for (std::size_t i = 0; i < TABLE_SIZE; ++i) t[i] = Scalar(data[i]);
			return t;
			}();
		return table;
	}

	// evaluate f over the argument table and accumulate, alternating the sign of the accumulation
	// so that a long run cannot drift into a regime where the adds themselves behave differently
	template<typename Scalar, typename Function>
	void accumulate(std::size_t nrOps, Function f) {
		const std::vector<Scalar>& table = arguments<Scalar>();
		Scalar acc(0.0);
		for (std::size_t i = 0; i < nrOps; ++i) {
			Scalar r = f(table[i & (TABLE_SIZE - 1)]);
			if (i & 1) acc = acc + r; else acc = acc - r;
		}
		hpbench::consume(acc);
	}

	template<typename Scalar>
	void AccumulateOnlyWorkload(std::size_t nrOps) {
		accumulate<Scalar>(nrOps, [](const Scalar& x) { return x; });
	}

	template<typename Scalar>
	void SqrtWorkload(std::size_t nrOps) {
		accumulate<Scalar>(nrOps, [](const Scalar& x) { using std::sqrt; return sqrt(x); });
	}

	template<typename Scalar>
	void ExpWorkload(std::size_t nrOps) {
		accumulate<Scalar>(nrOps, [](const Scalar& x) { using std::exp; return exp(x); });
	}

	template<typename Scalar>
	void LogWorkload(std::size_t nrOps) {
		accumulate<Scalar>(nrOps, [](const Scalar& x) { using std::log; return log(x); });
	}

	template<typename Scalar>
	void SinWorkload(std::size_t nrOps) {
		accumulate<Scalar>(nrOps, [](const Scalar& x) { using std::sin; return sin(x); });
	}

	template<typename Scalar>
	void CosWorkload(std::size_t nrOps) {
		accumulate<Scalar>(nrOps, [](const Scalar& x) { using std::cos; return cos(x); });
	}

	template<typename Scalar>
	void measureType(hpbench::Suite& suite, const std::string& type) {
		suite.measure("accumulate only", type, AccumulateOnlyWorkload<Scalar>);
		suite.measure("sqrt", type, SqrtWorkload<Scalar>);
		suite.measure("exp", type, ExpWorkload<Scalar>);
		suite.measure("log", type, LogWorkload<Scalar>);
		suite.measure("sin", type, SinWorkload<Scalar>);
		suite.measure("cos", type, CosWorkload<Scalar>);
	}

}  // anonymous namespace

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	hpbench::Suite suite("multi-component mathlib performance (universal#1315)", hpbench::targetWindow(argc, argv));
	suite.reportEnvironment();
	std::cout << "  note           : every row includes one accumulate; subtract the 'accumulate only' row\n\n";

	measureType<double>(suite, "double");
	measureType<dd>(suite, "dd");
	measureType<dd_cascade>(suite, "dd_cascade");
	measureType<td_cascade>(suite, "td_cascade");
	measureType<qd>(suite, "qd");
	measureType<qd_cascade>(suite, "qd_cascade");

	suite.reportLatency();
	suite.reportThroughput();
	suite.reportRatios(hpbench::cascadeVsClassic());

	std::cout << "\ndone.\n";
	return EXIT_SUCCESS;
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
