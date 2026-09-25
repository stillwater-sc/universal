// blocktype_parity.cpp: a cfloat's limb width must not be observable in its values
//
// cfloat<nbits, es, BlockType> stores its encoding in an array of BlockType limbs. The
// limb width is a STORAGE choice: how many blocks the encoding is cut into, and how wide
// the carries are inside the block arithmetic. It is not part of the format. Two cfloats
// with the same nbits and es therefore have to agree bit-for-bit on every value and every
// operation, whatever their limb width.
//
// That makes the two configurations each other's reference, so this suite needs no
// oracle. A divergence is unambiguously a bug in the wider path -- the limb arithmetic,
// its carry propagation, or the handling of a partially filled most significant limb.
//
// Why it exists (#1587): quad was the last large cfloat on 32-bit limbs, and it moves to
// uint64_t in this same PR, joining duble, xtndd and octo. It had reportedly been moved
// once before and reverted because a regression test failed, which would mean a
// limb-width-dependent defect. Nothing in the suite could have caught one:
// large_types.cpp, the test named for large multi-block configurations, hardcodes
// uint32_t for every configuration it covers.
//
// A caution about what this suite can and cannot see. It compares two limb widths
// against each other, so it finds anything that depends on the limb width -- and is
// blind to anything the two widths share. The subnormal-addition defect fixed in this
// PR is exactly that: fbits is the same for both widths, both took the same wrong
// branch, and both returned the same wrong answer, so the parity assertion passed.
// UBSan is what named it. Parity is necessary here, not sufficient.
//
// The configurations below deliberately include widths that are NOT a multiple of the
// limb width -- 80 and 160 bits -- because that leaves the most significant limb
// partially filled, which is where a mask or a carry is most likely to be wrong.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	// to_binary walks the encoding bit by bit through at(), so it renders the same text
	// for any limb width. It is the comparator: two configurations agree exactly when
	// their renderings are identical.
	template<typename A, typename B>
	bool agree(const A& a, const B& b) {
		return to_binary(a) == to_binary(b);
	}

	// Build a value from an explicit bit pattern, so both configurations are given
	// provably identical input rather than input from two RNG streams assumed to match.
	template<typename C>
	C fromBits(const std::vector<bool>& bits) {
		C v;
		v.clear();
		for (unsigned i = 0; i < bits.size() && i < C::nbits; ++i) {
			if (bits[i]) v.setbit(i, true);
		}
		return v;
	}

	// A spread of encodings: the special values, the range boundaries, and random
	// patterns across the whole width including the exponent field.
	template<unsigned nbits>
	std::vector<std::vector<bool>> bitPatterns(unsigned count, unsigned seed) {
		std::vector<std::vector<bool>> out;
		std::mt19937_64 rng(seed);
		std::uniform_int_distribution<int> coin(0, 1);

		// all zeros, all ones, and the single-bit patterns at both ends
		out.emplace_back(nbits, false);
		out.emplace_back(nbits, true);
		for (unsigned b : { 0u, 1u, nbits / 2u, nbits - 2u, nbits - 1u }) {
			std::vector<bool> p(nbits, false);
			if (b < nbits) p[b] = true;
			out.push_back(p);
		}
		// limb-boundary patterns: a run of ones straddling every 8/16/32/64-bit edge,
		// which is where a carry crosses from one limb into the next
		for (unsigned edge : { 8u, 16u, 32u, 64u }) {
			for (unsigned k = edge; k + 2 < nbits; k += edge) {
				std::vector<bool> p(nbits, false);
				for (unsigned i = (k >= 3 ? k - 3 : 0); i < k + 3 && i < nbits; ++i) p[i] = true;
				out.push_back(p);
			}
		}
		// random full-width patterns
		for (unsigned i = 0; i < count; ++i) {
			std::vector<bool> p(nbits);
			for (unsigned b = 0; b < nbits; ++b) p[b] = (coin(rng) != 0);
			out.push_back(p);
		}
		return out;
	}

	int report(int& fails, bool ok, const std::string& what, const std::string& detail, bool reportTestCases) {
		if (ok) return 0;
		++fails;
		if (reportTestCases && fails <= 8) {
			std::cout << "    FAIL " << what << '\n';
			if (!detail.empty()) std::cout << "         " << detail << '\n';
		}
		return 1;
	}

	// ---- the contract, for one (format, reference limb, candidate limb) ----------------

	template<unsigned nbits, unsigned es, typename RefBT, typename CanBT>
	int VerifyParity(const std::string& tag, bool reportTestCases) {
		using Ref = cfloat<nbits, es, RefBT, true, false, false>;
		using Can = cfloat<nbits, es, CanBT, true, false, false>;
		int fails = 0;

		// the two configurations describe the same format
		static_assert(Ref::nbits == Can::nbits, "same width");
		static_assert(Ref::es == Can::es, "same exponent field");
		static_assert(Ref::fbits == Can::fbits, "same fraction field");

		// 1. the encoding itself survives the change of limb width
		const auto patterns = bitPatterns<nbits>(24, 20260923u);
		for (const auto& p : patterns) {
			Ref r = fromBits<Ref>(p);
			Can c = fromBits<Can>(p);
			report(fails, agree(r, c), tag + ": encoding round-trip",
				"ref " + to_binary(r) + "\n         can " + to_binary(c), reportTestCases);
		}

		// 2. arithmetic agrees on every pair of those encodings
		for (std::size_t i = 0; i < patterns.size(); ++i) {
			for (std::size_t j = i; j < patterns.size(); ++j) {
				Ref ra = fromBits<Ref>(patterns[i]), rb = fromBits<Ref>(patterns[j]);
				Can ca = fromBits<Can>(patterns[i]), cb = fromBits<Can>(patterns[j]);

				report(fails, agree(Ref(ra + rb), Can(ca + cb)), tag + ": add",
					to_binary(ra) + " + " + to_binary(rb) + "\n         ref " + to_binary(Ref(ra + rb))
					+ "\n         can " + to_binary(Can(ca + cb)), reportTestCases);
				report(fails, agree(Ref(ra - rb), Can(ca - cb)), tag + ": sub",
					to_binary(ra) + " - " + to_binary(rb), reportTestCases);
				report(fails, agree(Ref(ra * rb), Can(ca * cb)), tag + ": mul",
					to_binary(ra) + " * " + to_binary(rb) + "\n         ref " + to_binary(Ref(ra * rb))
					+ "\n         can " + to_binary(Can(ca * cb)), reportTestCases);
				report(fails, agree(Ref(ra / rb), Can(ca / cb)), tag + ": div",
					to_binary(ra) + " / " + to_binary(rb), reportTestCases);
				// fma is the strongest check here: it builds a cfloat of nbits + fbits + 2
				// with the SAME BlockType and does a real multiply and add in it, so it
				// exercises limb arithmetic at a width wider than the format under test.
				report(fails, agree(Ref(fma(ra, rb, ra)), Can(fma(ca, cb, ca))), tag + ": fma",
					to_binary(ra) + " fma " + to_binary(rb), reportTestCases);
			}
		}

		// 3. the special encodings
		for (auto sv : { SpecificValue::maxpos, SpecificValue::minpos, SpecificValue::maxneg,
		                 SpecificValue::minneg, SpecificValue::infpos, SpecificValue::infneg,
		                 SpecificValue::qnan,   SpecificValue::snan,   SpecificValue::zero }) {
			Ref r(sv); Can c(sv);
			report(fails, agree(r, c), tag + ": specific value",
				"ref " + to_binary(r) + "\n         can " + to_binary(c), reportTestCases);
		}

		// 4. the ulp boundaries at both ends of the range, where a carry ripples the
		//    furthest across limbs
		{
			Ref r(SpecificValue::minpos); Can c(SpecificValue::minpos);
			for (int i = 0; i < 96; ++i) {
				++r; ++c;
				report(fails, agree(r, c), tag + ": increment from minpos",
					"step " + std::to_string(i) + "\n         ref " + to_binary(r)
					+ "\n         can " + to_binary(c), reportTestCases);
			}
		}
		{
			Ref r(SpecificValue::maxpos); Can c(SpecificValue::maxpos);
			for (int i = 0; i < 96; ++i) {
				--r; --c;
				report(fails, agree(r, c), tag + ": decrement from maxpos",
					"step " + std::to_string(i), reportTestCases);
			}
		}

		// 5. subnormal arithmetic, which walks the fraction without a hidden bit
		{
			Ref rm(SpecificValue::minpos); Can cm(SpecificValue::minpos);
			report(fails, agree(Ref(rm + rm), Can(cm + cm)), tag + ": minpos + minpos", "", reportTestCases);
			report(fails, agree(Ref(rm * Ref(2.0)), Can(cm * Can(2.0))), tag + ": minpos * 2", "", reportTestCases);
			report(fails, agree(Ref(rm / Ref(2.0)), Can(cm / Can(2.0))), tag + ": minpos / 2", "", reportTestCases);
			Ref rs = rm; Can cs = cm;
			for (int i = 0; i < 8; ++i) { rs = rs + rm; cs = cs + cm; }
			report(fails, agree(rs, cs), tag + ": subnormal accumulation", "", reportTestCases);
		}

		// sqrt, now that it earns a place here. It used to be
		// `cfloat(std::sqrt((double)a))` and exercised no limb arithmetic at all, so it
		// was excluded; since #1589 it is a Newton iteration in the cfloat's own
		// arithmetic, which makes it one of the heavier limb-level exercises in the suite
		// -- every step is a division and an addition at full width. Kept to a short list
		// of arguments rather than the pattern pairs, because each call iterates.
		for (double d : { 2.0, 3.0, 0.5, 1.0e10, 1.0e-10, 1.0 }) {
			Ref r(d); Can c(d);
			report(fails, agree(Ref(sqrt(r)), Can(sqrt(c))), tag + ": sqrt",
				"sqrt(" + std::to_string(d) + ")\n         ref " + to_binary(Ref(sqrt(r)))
				+ "\n         can " + to_binary(Can(sqrt(c))), reportTestCases);
		}
		{   // and at the top of the range, where the seed cannot come from a double
			Ref rm(SpecificValue::maxpos); Can cm(SpecificValue::maxpos);
			report(fails, agree(Ref(sqrt(rm)), Can(sqrt(cm))), tag + ": sqrt(maxpos)",
				"ref " + to_binary(Ref(sqrt(rm))) + "\n         can " + to_binary(Can(sqrt(cm))),
				reportTestCases);
		}

		// 7. conversion in both directions
		for (long long v : { 0ll, 1ll, -1ll, 42ll, -42ll, 65535ll, 65536ll,
		                     4294967295ll, 4294967296ll, 9007199254740993ll, -9007199254740993ll }) {
			Ref r(v); Can c(v);
			report(fails, agree(r, c), tag + ": integer conversion",
				"from " + std::to_string(v) + "\n         ref " + to_binary(r)
				+ "\n         can " + to_binary(c), reportTestCases);
			report(fails, (long long)(r) == (long long)(c), tag + ": integer round-trip",
				"to " + std::to_string((long long)(r)) + " vs " + std::to_string((long long)(c)), reportTestCases);
		}
		for (double d : { 0.0, 1.0, -1.0, 0.5, 1.0 / 3.0, 1.0e300, 1.0e-300, 3.14159265358979 }) {
			Ref r(d); Can c(d);
			report(fails, agree(r, c), tag + ": double conversion",
				"from " + std::to_string(d), reportTestCases);
			report(fails, double(r) == double(c), tag + ": double round-trip",
				"to " + std::to_string(double(r)) + " vs " + std::to_string(double(c)), reportTestCases);
		}

		return fails;
	}

	// Every candidate limb width against the uint32_t reference, for one format.
	template<unsigned nbits, unsigned es>
	int VerifyFormat(const char* name, bool reportTestCases) {
		int fails = 0;
		const std::string n(name);
		fails += VerifyParity<nbits, es, std::uint32_t, std::uint8_t >(n + " u32 vs u8",  reportTestCases);
		fails += VerifyParity<nbits, es, std::uint32_t, std::uint16_t>(n + " u32 vs u16", reportTestCases);
		fails += VerifyParity<nbits, es, std::uint32_t, std::uint64_t>(n + " u32 vs u64", reportTestCases);
		return fails;
	}

}  // anonymous namespace

#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat BlockType parity (#1587)";
	std::string test_tag    = "blocktype parity";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	{
		// the configuration the issue is about, side by side
		using Q32 = cfloat<128, 15, std::uint32_t, true, false, false>;
		using Q64 = cfloat<128, 15, std::uint64_t, true, false, false>;
		std::cout << "cfloat<128,15> blocks: uint32_t=" << Q32::nrBlocks
		          << "  uint64_t=" << Q64::nrBlocks << '\n';
		Q32 a32(1.0), b32(3.0);
		Q64 a64(1.0), b64(3.0);
		std::cout << "  1/3  u32: " << to_binary(Q32(a32 / b32)) << '\n';
		std::cout << "  1/3  u64: " << to_binary(Q64(a64 / b64)) << '\n';
	}
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	// quad, the configuration #1587 is about
	nrOfFailedTestCases += ReportTestResult(VerifyFormat<128, 15>("cfloat<128,15>", reportTestCases),
		test_tag, "cfloat<128,15> (quad)");
	// a width that is NOT a multiple of 64, so the most significant limb is partial
	nrOfFailedTestCases += ReportTestResult(VerifyFormat<80, 15>("cfloat<80,11>", reportTestCases),
		test_tag, "cfloat<80,15> (xtndd, partial MSU)");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyFormat<160, 15>("cfloat<160,15>", reportTestCases),
		test_tag, "cfloat<160,15> (partial MSU)");
	nrOfFailedTestCases += ReportTestResult(VerifyFormat<256, 19>("cfloat<256,19>", reportTestCases),
		test_tag, "cfloat<256,19> (octo)");
#endif

#if REGRESSION_LEVEL_3
	// the small formats, where a single limb holds the whole encoding for the wider types
	nrOfFailedTestCases += ReportTestResult(VerifyFormat<32, 8>("cfloat<32,8>", reportTestCases),
		test_tag, "cfloat<32,8> (single)");
	nrOfFailedTestCases += ReportTestResult(VerifyFormat<64, 11>("cfloat<64,11>", reportTestCases),
		test_tag, "cfloat<64,11> (duble)");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyFormat<96, 15>("cfloat<96,15>", reportTestCases),
		test_tag, "cfloat<96,15>");
	// es < 21 is a hard limit in cfloat_impl.hpp, so the widest exponent this can carry is 20
	nrOfFailedTestCases += ReportTestResult(VerifyFormat<512, 20>("cfloat<512,20>", reportTestCases),
		test_tag, "cfloat<512,20>");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
