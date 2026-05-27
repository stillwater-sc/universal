// parse.cpp: parse-cost benchmark for ereal decimal-string parsing (issue #1013)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Tracks ereal<N>::parse cost as a function of string length. parse() used to
// blow up to O(N^3) because the Horner accumulator grew one component per digit
// and never respected maxlimbs -- a 320-digit string took >120s (issue #913).
// That is fixed (expansion_product renormalize #981 + the maxlimbs digit/result
// caps in #1006/#1011); this benchmark reports the cost and guards against a
// reintroduction of the complexity blowup.
//
// The guard threshold is deliberately enormous (2 seconds for a 320-digit parse)
// so it never flakes on a slow or loaded CI runner -- it exists only to catch the
// catastrophic O(N^3) regression (which was >120s, ~5ms today), not to police
// normal timing variation.

#include <universal/utility/directives.hpp>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <universal/number/ereal/ereal.hpp>

namespace sw { namespace universal {

	// average wall-clock milliseconds for parsing `s` into ereal<maxlimbs>, over
	// `reps` runs (total elapsed / reps). Counts any parse() failures into
	// `parseFailures` -- parse returns false and leaves the value unchanged on a
	// malformed input, which would otherwise be timed as if it were valid work.
	template<unsigned maxlimbs>
	double time_parse_ms(const std::string& s, int reps, int& parseFailures) {
		auto t0 = std::chrono::steady_clock::now();
		volatile double sink = 0.0;
		for (int i = 0; i < reps; ++i) {
			ereal<maxlimbs> v;
			if (!v.parse(s)) { ++parseFailures; continue; }
			sink += double(v);
		}
		auto t1 = std::chrono::steady_clock::now();
		(void)sink;
		return std::chrono::duration<double, std::milli>(t1 - t0).count() / reps;
	}

	// repeating-decimal fraction string of `digits` fractional digits ("0.142857...")
	std::string fraction_string(unsigned digits) {
		std::string s = "0.";
		const char* rep = "142857";  // 1/7
		for (unsigned i = 0; i < digits; ++i) s += rep[i % 6];
		return s;
	}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::cout << "ereal parse-cost benchmark (issue #1013)\n";
	std::cout << "string length vs parse time (ms/call)\n\n";
	std::cout << std::setw(8) << "digits"
	          << std::setw(14) << "ereal<2>"
	          << std::setw(14) << "ereal<8>"
	          << std::setw(14) << "ereal<19>" << "\n";

	const unsigned lengths[] = { 32, 64, 128, 320, 440 };
	const int reps = 50;
	const double GUARD_MS = 2000.0;  // catastrophic-regression guard (was >120000ms; ~5ms today)
	int failures = 0;
	int parseFailures = 0;  // every input below is a valid decimal, so this must stay 0

	std::cout << std::fixed << std::setprecision(4);
	for (unsigned len : lengths) {
		std::string s = fraction_string(len);
		double t2  = time_parse_ms<2>(s, reps, parseFailures);
		double t8  = time_parse_ms<8>(s, reps, parseFailures);
		double t19 = time_parse_ms<19>(s, reps, parseFailures);
		std::cout << std::setw(8) << len
		          << std::setw(14) << t2
		          << std::setw(14) << t8
		          << std::setw(14) << t19 << "\n";
		if (len >= 320 && (t2 > GUARD_MS || t8 > GUARD_MS || t19 > GUARD_MS)) {
			std::cout << "  *** REGRESSION: " << len << "-digit parse exceeded " << GUARD_MS
			          << " ms -- parse complexity blowup may have returned (#913) ***\n";
			++failures;
		}
	}

	if (parseFailures > 0) {
		std::cout << "  *** " << parseFailures << " parse() failures on valid input ***\n";
		++failures;
	}

	std::cout << "\n" << (failures == 0 ? "PASS" : "FAIL") << " parse cost within bounds\n";
	return (failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
