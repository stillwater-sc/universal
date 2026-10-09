// test_precision_profile.cpp: the precision-vs-magnitude profile against direct computation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// precision_profile_of<T> decodes every encoding of a small type and sorts the values; a wide
// type is sampled per binade, with the spacing from ++ (nextafter for native floats).  Checked:
//   - exhaustive, 8-bit types: one point per positive finite encoding but the last, ascending,
//     with decimals = log10(2 x / (x+ - x)) for the neighbour x+ found independently by walking
//     the bit patterns in value order
//   - closed forms: integers log10(2x), fixed-point log10(2x 2^rbits), float normals constant
//     per binade at the start, 0 decimals outside [minpos, maxpos]
//   - sampled == exhaustive: forcing the sampled path on 16-bit types must reproduce the
//     exhaustive decimals at every sampled magnitude; native float sampled == cfloat<32,8>
//   - bposit never drops below its guaranteed fraction bits F_min = nbits - 1 - rs - es
#include <universal/utility/directives.hpp>
#include <cmath>
#include <map>
#include <string>
#include <universal/number/bposit/bposit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/precision_profile.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// exhaustive profile of an 8-bit type against an independent decode of every bit pattern
template<typename T>
int VerifyExhaustive(const std::string& tag, bool report) {
	int fails = 0;
	auto fail = [&](const std::string& what) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << tag << ": " << what << '\n'; };
	std::map<double, int> values;   // positive finite values, ordered
	for (unsigned bits = 0; bits < 256u; ++bits) {
		T x;
		x.setbits(bits);
		const double d = double(x);
		if (std::isfinite(d) && d > 0.0) values[d] = 1;
	}
	const precision_profile p = precision_profile_of<T>(tag);
	if (!p.exhaustive) fail("an 8-bit type must be profiled exhaustively");
	if (p.points.size() != values.size()) fail("expected " + std::to_string(values.size()) + " points, got " + std::to_string(p.points.size()));
	auto it = values.begin();
	for (std::size_t i = 0; i + 1 < p.points.size() && it != values.end(); ++i, ++it) {
		const double x = it->first, xp = std::next(it)->first;
		const double want = std::log10(2.0 * x / (xp - x));
		if (p.points[i].magnitude != x || std::abs(p.points[i].decimals - want) > 1.0e-12) { fail("point " + std::to_string(i) + " at " + std::to_string(x)); break; }
		if (std::abs(p.points[i].fraction_bits - std::log2(x / (xp - x))) > 1.0e-12) { fail("fraction bits at " + std::to_string(x)); break; }
	}
	if (decimals_at(p, p.minpos / 2.0) != 0.0 || decimals_at(p, p.maxpos * 2.0) != 0.0) fail("0 decimals outside [minpos, maxpos]");
	return fails;
}

int VerifyClosedForms(bool report) {
	int fails = 0;
	auto expect = [&](bool ok, const std::string& what) { if (!ok) { ++fails; if (report) std::cerr << "FAIL " << what << '\n'; } };
	const auto i8 = precision_profile_of<integer<8, std::uint8_t>>("int8");
	for (const precision_point& q : i8.points) expect(std::abs(q.decimals - std::log10(2.0 * q.magnitude)) < 1e-12, "integer<8> decimals = log10(2x) at " + std::to_string(q.magnitude));
	const auto q15 = precision_profile_of<fixpnt<16, 15, Modulo, std::uint16_t>>("Q15");
	for (const precision_point& q : q15.points) {
		if (std::abs(q.decimals - std::log10(2.0 * q.magnitude * 32768.0)) > 1e-9) { expect(false, "fixpnt<16,15> decimals = log10(2x 2^15) at " + std::to_string(q.magnitude)); break; }
	}
	expect(decimals_at(q15, 1.0) == 0.0 && decimals_at(q15, 0.5) > 4.5, "Q15 holds 4.8 decimals just below 1 and none at 1");
	const auto i16 = precision_profile_of<std::int16_t>("int16");
	expect(i16.exhaustive && i16.points.size() == 32767u && decimals_at(i16, 0.5) == 0.0, "int16: 32767 points, 0 decimals below 1");
	// IEEE half normals: at the start of every binade exactly log10(2 2^10) decimals
	const auto h = precision_profile_of<cfloat<16, 5, std::uint16_t, true, false, false>>("fp16");
	for (int k = -14; k <= 15; ++k) expect(std::abs(decimals_at(h, std::ldexp(1.0, k)) - std::log10(2048.0)) < 1e-12, "fp16 binade start 2^" + std::to_string(k));
	return fails;
}

// force the sampled path on a 16-bit type: it must agree with the exhaustive profile
template<typename T>
int VerifySampledMatchesExhaustive(const std::string& tag, bool report) {
	int fails = 0;
	const precision_profile ex = precision_profile_of<T>(tag);
	const precision_profile sa = precision_profile_of<T>(tag, 4, 8);   // exhaustiveLimit 8: sampled
	if (!ex.exhaustive || sa.exhaustive || sa.points.size() < 8) { if (report) std::cerr << "FAIL " << tag << " sampling setup\n"; return 1; }
	for (const precision_point& q : sa.points) {
		// the same encoding in the exhaustive profile; its spacing is to the next value up,
		// except at maxpos, where both use the spacing below
		auto it = std::lower_bound(ex.points.begin(), ex.points.end(), q.magnitude, [](const precision_point& a, double v) { return a.magnitude < v; });
		if (it == ex.points.end() || it->magnitude != q.magnitude) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << tag << " sampled magnitude " << q.magnitude << " is not an encoding\n"; continue; }
		if (std::abs(it->decimals - q.decimals) > 1e-9) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << tag << " at " << q.magnitude << ": sampled " << q.decimals << " exhaustive " << it->decimals << '\n'; }
	}
	return fails;
}

int VerifyNativeFloat(bool report) {
	int fails = 0;
	const auto f = precision_profile_of<float>("float");
	const auto c = precision_profile_of<cfloat<32, 8, std::uint32_t, true, false, false>>("cfloat32");
	if (f.points.size() != c.points.size()) { if (report) std::cerr << "FAIL float and cfloat<32,8> sample counts differ\n"; return 1; }
	for (std::size_t i = 0; i < f.points.size(); ++i) {
		if (f.points[i].magnitude != c.points[i].magnitude || std::abs(f.points[i].decimals - c.points[i].decimals) > 1e-12) {
			++fails; if (report && fails < 10) std::cerr << "FAIL float vs cfloat<32,8> at " << f.points[i].magnitude << '\n';
		}
	}
	return fails;
}

template<typename B>
int VerifyBpositFloor(const std::string& tag, unsigned fmin, bool report) {
	int fails = 0;
	const auto p = precision_profile_of<B>(tag);
	for (const precision_point& q : p.points) {
		if (q.fraction_bits < double(fmin) - 1e-9) { ++fails; if (report && fails < 5) std::cerr << "FAIL " << tag << " has " << q.fraction_bits << " fraction bits at " << q.magnitude << '\n'; }
	}
	return fails;
}

}  // anonymous namespace

int main()
try {
	std::string test_suite  = "precision profile";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<integer<8, std::uint8_t>>("integer<8>", reportTestCases), "integer<8>", "exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<fixpnt<8, 4, Modulo, std::uint8_t>>("fixpnt<8,4>", reportTestCases), "fixpnt<8,4>", "exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<cfloat<8, 2, std::uint8_t, true, false, false>>("cfloat<8,2>", reportTestCases), "cfloat<8,2>", "exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<8, 0>>("posit<8,0>", reportTestCases), "posit<8,0>", "exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<bposit<8, 3, 1, std::uint8_t>>("bposit<8,3,1>", reportTestCases), "bposit<8,3,1>", "exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<lns<8, 3, std::uint8_t>>("lns<8,3>", reportTestCases), "lns<8,3>", "exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifyClosedForms(reportTestCases), "int, fixpnt, fp16", "closed forms");
	nrOfFailedTestCases += ReportTestResult(VerifySampledMatchesExhaustive<cfloat<16, 5, std::uint16_t, true, false, false>>("fp16", reportTestCases), "fp16", "sampled == exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifySampledMatchesExhaustive<posit<16, 2>>("posit<16,2>", reportTestCases), "posit<16,2>", "sampled == exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifySampledMatchesExhaustive<bposit<16, 6, 5, std::uint16_t>>("bposit<16,6,5>", reportTestCases), "bposit<16,6,5>", "sampled == exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifySampledMatchesExhaustive<fixpnt<16, 15, Modulo, std::uint16_t>>("Q15", reportTestCases), "fixpnt<16,15>", "sampled == exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifySampledMatchesExhaustive<lns<16, 8, std::uint16_t>>("lns<16,8>", reportTestCases), "lns<16,8>", "sampled == exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeFloat(reportTestCases), "float", "native == cfloat<32,8>");
	nrOfFailedTestCases += ReportTestResult(VerifyBpositFloor<bposit<16, 6, 5, std::uint16_t>>("bposit<16,6,5>", 4, reportTestCases), "bposit<16,6,5>", "F_min = 4 floor");
	nrOfFailedTestCases += ReportTestResult(VerifyBpositFloor<bposit<32, 6, 5, std::uint32_t>>("bposit<32,6,5>", 20, reportTestCases), "bposit<32,6,5>", "F_min = 20 floor");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
