// precision_profiles.cpp: choosing a number type for DSP and FFT pipelines by its precision profile
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
//
// #1636.  For each type, the decimals of accuracy at every magnitude: -log10(ulp / (2|x|)), the
// worst-case relative error of rounding to the nearest encoding (Gustafson, The End of Error).
// Read against DSP needs:
//   - the signal band: samples are normalized to [-1, 1), so the precision just below 1.0
//     sets the quantization noise floor (about 6.02 dB of SQNR per bit)
//   - FFT growth: an N-point FFT grows magnitudes by up to log2(N) bits, so a type needs range
//     above 1.0 -- or a rescale at every stage, as integer and fixed-point formats do
//   - the precision floor: small coefficients and tail energy, around 2^-15 (-90 dBFS),
//     should not fall off a cliff
//
// The comparison follows "Closing the Gap Between Float and Posit Hardware Efficiency"
// (arXiv 2603.01615), which singles out signal processing as a workload where a b-posit with a
// smaller eS suffices: it trades unused dynamic range for guaranteed fraction bits.  Each set
// has the same width.  The lns formats are sized to a DSP-scale range: lns<16,10> spans 2^+-16
// and lns<32,23> spans 2^+-128 (an lns<n, n-1> would have no integer exponent bits at all).
//
// The last table fits the b-posit's range, 2^+-(rs 2^es), to the region of interest
// [2^-15, 2^12]: the standard bposit<16,6,5> spends most of its encodings far outside it.
//
// Usage: dsp_precision_profiles [output directory]
// prints the tables; with a directory, also writes one CSV per type for
// tools/notebooks/plot_precision_profiles.py.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <universal/number/bposit/bposit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/utility/precision_profile.hpp>

namespace {

using namespace sw::universal;

int fails = 0;
void check(bool ok, const std::string& what) {
	std::cout << (ok ? "  ok    " : "  FAIL  ") << what << '\n';
	if (!ok) ++fails;
}

double min_fraction_bits(const precision_profile& p) {
	double m = 1.0e9;
	for (const precision_point& q : p.points) m = std::min(m, q.fraction_bits);
	return m;
}
double min_decimals_over(const precision_profile& p, int log2Lo, int log2Hi) {
	double m = 1.0e9;
	for (int k = log2Lo; k <= log2Hi; ++k) m = std::min(m, decimals_at(p, std::ldexp(1.0, k)));
	return m;
}

// the region of interest: small coefficients at -90 dBFS up to 4096-point FFT growth
constexpr int roi_lo = -15, roi_hi = 12;

// the share of a type's positive encodings that fall in the region of interest (exhaustive
// profiles only: a sampled profile does not hold one point per encoding)
double share_in_roi(const precision_profile& p) {
	if (!p.exhaustive || p.points.empty()) return -1.0;
	std::size_t in = 0;
	for (const precision_point& q : p.points) if (q.magnitude >= std::ldexp(1.0, roi_lo) && q.magnitude <= std::ldexp(1.0, roi_hi)) ++in;
	return double(in) / double(p.points.size());
}

// how well each b-posit configuration fits the region of interest
void range_fit(const std::vector<precision_profile>& candidates) {
	std::cout << std::setw(16) << "type" << std::setw(16) << "range (log2)" << std::setw(18) << "encodings in ROI" << std::setw(14) << "[0.5, 1)"
	          << std::setw(16) << "worst in ROI" << std::setw(14) << "floor (bits)" << '\n';
	std::cout << std::string(94, '-') << '\n';
	for (const precision_profile& p : candidates) {
		std::stringstream range, share;
		range << std::fixed << std::setprecision(0) << std::round(std::log2(p.minpos)) + 0.0 << " .. " << std::round(std::log2(p.maxpos)) + 0.0;
		const double f = share_in_roi(p);
		if (f < 0.0) share << "(sampled)"; else share << std::fixed << std::setprecision(1) << 100.0 * f << " %";
		std::cout << std::setw(16) << p.label << std::setw(16) << range.str() << std::setw(18) << share.str() << std::fixed << std::setprecision(2)
		          << std::setw(14) << decimals_at(p, 0.5) << std::setw(16) << min_decimals_over(p, roi_lo, roi_hi)
		          << std::setw(14) << std::setprecision(1) << min_fraction_bits(p) << '\n';
	}
	std::cout << std::defaultfloat;
}

// the numbers a DSP designer reads off the curves
void summary(const std::vector<precision_profile>& set) {
	std::cout << std::setw(14) << "type" << std::setw(18) << "range (log2)" << std::setw(14) << "signal band" << std::setw(10) << "SQNR" << std::setw(12) << "at 2^-15"
	          << std::setw(12) << "at 2^10" << std::setw(12) << "floor" << '\n';
	std::cout << std::setw(14) << "" << std::setw(18) << "" << std::setw(14) << "[0.5, 1)" << std::setw(10) << "(dB)*" << std::setw(12) << "-90 dBFS"
	          << std::setw(12) << "N = 1024" << std::setw(12) << "(bits)" << '\n';
	std::cout << std::string(92, '-') << '\n';
	for (const precision_profile& p : set) {
		const double band = decimals_at(p, 0.5);
		const double bits = band / std::log10(2.0) - 1.0;   // fraction bits at the start of [0.5, 1)
		std::stringstream range;
		range << std::fixed << std::setprecision(0) << std::round(std::log2(p.minpos)) + 0.0 << " .. " << std::round(std::log2(p.maxpos)) + 0.0;   // + 0.0: no "-0"
		std::cout << std::setw(14) << p.label << std::setw(18) << range.str() << std::fixed << std::setprecision(2) << std::setw(14) << band
		          << std::setw(10) << std::setprecision(0) << (band > 0.0 ? 6.02 * bits : 0.0) << std::setprecision(2)
		          << std::setw(12) << decimals_at(p, std::ldexp(1.0, -15)) << std::setw(12) << decimals_at(p, 1024.0)
		          << std::setw(12) << std::setprecision(1) << min_fraction_bits(p) << '\n';
	}
	std::cout << "* 6.02 dB per fraction bit at the start of [0.5, 1)\n" << std::defaultfloat;
}

}  // anonymous namespace

int main(int argc, char** argv)
try {
	const std::string outdir = (argc > 1) ? argv[1] : "";

	const std::vector<precision_profile> set16{
		precision_profile_of<std::int16_t>("int16"),
		precision_profile_of<fixpnt<16, 15, Modulo, std::uint16_t>>("Q15"),
		precision_profile_of<cfloat<16, 5, std::uint16_t, true, false, false>>("fp16"),
		precision_profile_of<lns<16, 10, std::uint16_t>>("lns<16,10>"),
		precision_profile_of<bposit<16, 6, 5, std::uint16_t>>("bposit<16,6,5>"),
		precision_profile_of<bposit<16, 5, 2, std::uint16_t>>("bposit<16,5,2>"),
	};
	const std::vector<precision_profile> set32{
		precision_profile_of<std::int32_t>("int32"),
		precision_profile_of<fixpnt<32, 31, Modulo, std::uint32_t>>("Q31"),
		precision_profile_of<float>("float"),
		precision_profile_of<lns<32, 23, std::uint32_t>>("lns<32,23>"),
		precision_profile_of<bposit<32, 6, 5, std::uint32_t>>("bposit<32,6,5>"),
		precision_profile_of<bposit<32, 5, 2, std::uint32_t>>("bposit<32,5,2>"),
	};
	const precision_profile narrow = precision_profile_of<bposit<16, 3, 1, std::uint16_t>>("bposit<16,3,1>");
	// fitting the b-posit's range to the region of interest: rs and es set the range, 2^+-(rs 2^es)
	const std::vector<precision_profile> fit16{
		set16[4],
		precision_profile_of<bposit<16, 6, 3, std::uint16_t>>("bposit<16,6,3>"),
		precision_profile_of<bposit<16, 6, 2, std::uint16_t>>("bposit<16,6,2>"),
		set16[5],
		precision_profile_of<bposit<16, 4, 2, std::uint16_t>>("bposit<16,4,2>"),
		narrow,
	};
	const std::vector<precision_profile> fit32{
		set32[4],
		set32[5],
		precision_profile_of<bposit<32, 4, 2, std::uint32_t>>("bposit<32,4,2>"),
	};

	std::cout << "Decimals of accuracy, -log10(ulp / (2|x|)), at magnitude 2^k\n\n16-bit types\n";
	print_precision_table(std::cout, set16, -24, 16, 2, 16);
	std::cout << '\n';
	summary(set16);
	std::cout << "\n32-bit types\n";
	print_precision_table(std::cout, set32, -40, 40, 4, 16);
	std::cout << '\n';
	summary(set32);
	std::cout << "\nThe smaller-eS b-posit must still cover the signal's range: bposit<16,3,1> keeps "
	          << std::fixed << std::setprecision(2) << decimals_at(narrow, 0.5) << " decimals at [0.5, 1), but spans only 2^"
	          << std::setprecision(0) << std::log2(narrow.minpos) << " .. 2^" << std::log2(narrow.maxpos)
	          << ": 0 decimals at 2^-15 and at 2^10.\n" << std::defaultfloat;

	std::cout << "\nFitting the b-posit range to the region of interest [2^" << roi_lo << ", 2^" << roi_hi << "]\n";
	range_fit(fit16);
	std::cout << '\n';
	range_fit(fit32);

	if (!outdir.empty()) {
		std::set<std::string> written;
		for (const auto* set : { &set16, &set32, &fit16, &fit32 }) {
			for (const precision_profile& p : *set) {
				std::string name = p.label;
				for (char& c : name) if (c == '<' || c == '>' || c == ',') c = '_';
				if (!written.insert(name).second) continue;
				const std::string path = outdir + "/" + name + ".csv";
				if (!write_precision_csv(p, path)) { std::cerr << "cannot write " << path << '\n'; return EXIT_FAILURE; }
			}
		}
		std::cout << "\nwrote " << written.size() << " CSV files to " << outdir << '\n';
	}

	const auto& [i16, q15, fp16, lns16, bp16std, bp16dsp] = std::tie(set16[0], set16[1], set16[2], set16[3], set16[4], set16[5]);
	const auto& [i32, q31, f32, lns32, bp32std, bp32dsp] = std::tie(set32[0], set32[1], set32[2], set32[3], set32[4], set32[5]);
	std::cout << "\nassertions:\n";
	check(decimals_at(q15, 1.0) == 0.0 && decimals_at(q31, 1.0) == 0.0 && decimals_at(q15, 0.5) > decimals_at(fp16, 0.5),
	      "Q15 and Q31 are the most precise in the signal band, and have no headroom above 1.0: FFT growth needs a rescale per stage");
	check(decimals_at(i16, 0.5) == 0.0 && decimals_at(i32, 0.5) == 0.0, "int16 and int32 cannot represent the signal band without a scale factor");
	check(decimals_at(q15, std::ldexp(1.0, -15)) < 0.5 && decimals_at(fp16, std::ldexp(1.0, -15)) > 2.9,
	      "at 2^-15 Q15 is down to its last bit; fp16 keeps 3 decimals");
	check(min_fraction_bits(bp16dsp) >= 8.0 - 1e-9 && min_fraction_bits(bp32dsp) >= 24.0 - 1e-9,
	      "bposit<16,5,2> and bposit<32,5,2> never drop below F_min = 8 and 24 fraction bits");
	check(decimals_at(bp16dsp, 0.5) > decimals_at(fp16, 0.5) && decimals_at(bp32dsp, 0.5) > decimals_at(f32, 0.5),
	      "the smaller-eS bposit beats fp16 and float in the signal band");
	check(min_decimals_over(bp16dsp, -15, 12) > 2.7 - 1e-9 && min_decimals_over(bp32dsp, -15, 12) > 7.5 - 1e-9,
	      "... and keeps at least 2.7 (16-bit) and 7.5 (32-bit) decimals from 2^-15 up to 4096-point FFT growth");
	check(decimals_at(bp16std, 0.5) < decimals_at(fp16, 0.5),
	      "the standard bposit<16,6,5> spends its bits on a 2^+-192 range and trails fp16 in the signal band");
	check(decimals_at(narrow, std::ldexp(1.0, -15)) == 0.0 && decimals_at(narrow, 1024.0) == 0.0, "bposit<16,3,1> is too narrow for a DSP pipeline");
	const precision_profile& bp16fit = fit16[4];
	check(share_in_roi(bp16std) < 0.25 && share_in_roi(bp16fit) > 0.90,
	      "bposit<16,6,5> places under a quarter of its encodings in the region of interest; bposit<16,4,2> places over 90%");
	check(min_decimals_over(bp16fit, roi_lo, roi_hi) >= 3.0 && min_fraction_bits(bp16fit) >= 9.0 - 1e-9 && min_fraction_bits(bp16fit) > min_fraction_bits(bp16dsp),
	      "bposit<16,4,2>, the tightest fit, keeps 3 decimals across the region and a 9-bit floor, one more than bposit<16,5,2>");

	std::cout << (fails == 0 ? "PASS\n" : "FAIL\n");
	return (fails == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
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
