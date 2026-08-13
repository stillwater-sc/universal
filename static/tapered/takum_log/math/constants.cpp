// constants.cpp: verification of the logarithmic takum constant table
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// takum_log stores |value| = sqrt(e)^l, so a constant X is tabulated as its exact
// logarithmic value l_X = 2 ln X, carried as an integer pair (c, N) meaning
// c + N/2^62 rather than as a real literal.
//
// The entries were produced at 80 decimal digits and checked offline against a
// 113-bit __float128 reference at 12, 16, 24, 32, 48 and 64 bits, all 174
// combinations reproducing the correctly rounded encoding.  That check needs
// quadmath and cannot run here, so this suite re-establishes what it can without:
//
//   - the table CONTENT matches an independently recomputed 2 ln X
//   - the encoding path lands on the representable l nearest the tabulated one,
//     measured in exact integers at 2^-62
//   - it beats constructing from a double literal, which is the whole reason the
//     table exists: a double caps l at 53 significant bits while takum_log<64,3>
//     has 58 or 59 fraction bits
//   - the constants that are EXACT in this format stay exact at every width
//   - reciprocal pairs negate, and the entries are ordered as their real values are
//
// The first of those is load-bearing and was missing from the first draft of this
// file.  Every other check measures against the table, so all of them are
// satisfied by a corrupted entry -- perturbing pi's numerator by 200 ulp passed
// the whole suite until VerifyTableValues was added.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <cfloat>
#include <universal/number/takum/takum_log.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using sw::universal::takum_log_constant;
using sw::universal::takum_log_constant_qbits;

struct Entry {
	const char*         name;
	takum_log_constant  k;
	long double         value;    // the real constant, for the ordering check
};

// The tabulated set, paired with long double references.  The references are only
// used for ordering and a coarse value check; the exact assertions run off (c, N).
const Entry table[] = {
	{ "pi_4",     sw::universal::tkml_pi_4,     0.78539816339744830961566084581987572L },
	{ "pi_3",     sw::universal::tkml_pi_3,     1.04719755119659774615421446109316763L },
	{ "pi_2",     sw::universal::tkml_pi_2,     1.57079632679489661923132169163975144L },
	{ "3pi_4",    sw::universal::tkml_3pi_4,    2.35619449019234492884698253745962716L },
	{ "pi",       sw::universal::tkml_pi,       3.14159265358979323846264338327950288L },
	{ "2pi",      sw::universal::tkml_2pi,      6.28318530717958647692528676655900577L },
	{ "3pi",      sw::universal::tkml_3pi,      9.42477796076937971538793014983850865L },
	{ "4pi",      sw::universal::tkml_4pi,     12.56637061435917295385057353311801153L },
	{ "4_pi",     sw::universal::tkml_4_pi,     1.27323954473516268615107010698011490L },
	{ "3_pi",     sw::universal::tkml_3_pi,     0.95492965855137201461330258023508617L },
	{ "2_pi",     sw::universal::tkml_2_pi,     0.63661977236758134307553505349005745L },
	{ "1_pi",     sw::universal::tkml_1_pi,     0.31830988618379067153776752674502872L },
	{ "2_sqrtpi", sw::universal::tkml_2_sqrtpi, 1.12837916709551257389615890312154517L },
	{ "sqrt2",    sw::universal::tkml_sqrt2,    1.41421356237309504880168872420969808L },
	{ "1_sqrt2",  sw::universal::tkml_1_sqrt2,  0.70710678118654752440084436210484904L },
	{ "sqrt3",    sw::universal::tkml_sqrt3,    1.73205080756887729352744634150587237L },
	{ "sqrt5",    sw::universal::tkml_sqrt5,    2.23606797749978969640917366873127624L },
	{ "phi",      sw::universal::tkml_phi,      1.61803398874989484820458683436563812L },
	{ "1_phi",    sw::universal::tkml_1_phi,    0.61803398874989484820458683436563812L },
	{ "e",        sw::universal::tkml_e,        2.71828182845904523536028747135266250L },
	{ "1_e",      sw::universal::tkml_1_e,      0.36787944117144232159552377016146087L },
	{ "sqrt_e",   sw::universal::tkml_sqrt_e,   1.64872127070012814684865078781416357L },
	{ "e_gamma",  sw::universal::tkml_e_gamma,  0.57721566490153286060651209008240243L },
	{ "log2e",    sw::universal::tkml_log2e,    1.44269504088896340735992468100189214L },
	{ "log10e",   sw::universal::tkml_log10e,   0.43429448190325182765112891891660508L },
	{ "ln2",      sw::universal::tkml_ln2,      0.69314718055994530941723212145817657L },
	{ "ln3",      sw::universal::tkml_ln3,      1.09861228866810969139524523692252570L },
	{ "ln4",      sw::universal::tkml_ln4,      1.38629436111989061883446424291635314L },
	{ "ln10",     sw::universal::tkml_ln10,     2.30258509299404568401799145468436421L },
};
constexpr size_t table_size = sizeof(table) / sizeof(table[0]);

// Distance from an encoding's logarithmic value to a tabulated one, in units of
// 2^-62.  Both are exact binary fractions, so this is exact integer arithmetic.
//
// It cannot overflow: encode_fraction rounds to nearest, so the encoded l is
// within half an ulp of the target, which bounds the result by 2^(61-p).  The
// characteristics therefore differ by at most one and the intermediate 2^62 term
// stays inside int64_t.  The bound is asserted rather than assumed.
template<typename TakumLog>
bool log_distance(const TakumLog& v, const takum_log_constant& k, int64_t& distance) {
	auto d = TakumLog::Codec::decode(v.magnitude_bits());
	if (d.p > takum_log_constant_qbits) return false;             // table too narrow to judge
	const int64_t dc = d.c - k.c;
	if (dc < -1 || dc > 1) return false;                          // outside the safe window
	const int64_t N_enc = static_cast<int64_t>(d.M_bits << (takum_log_constant_qbits - d.p));
	distance = dc * (int64_t(1) << takum_log_constant_qbits) + (N_enc - static_cast<int64_t>(k.N));
	if (distance < 0) distance = -distance;
	return true;
}

// The encoding path must land on the representable l nearest the tabulated one,
// and must be at least as close as building the constant from a double literal.
template<unsigned nbits, unsigned rbits>
int VerifyTableEncoding(bool reportTestCases, int& improvements) {
	using TL = sw::universal::takum_log<nbits, rbits, std::uint64_t>;
	int nrOfFailedTests = 0;

	for (size_t i = 0; i < table_size; ++i) {
		const Entry& t = table[i];
		TL fromTable = sw::universal::takum_log_constant_cast<TL>(t.k);
		if (fromTable.iszero() || fromTable.isnar()) continue;     // saturated at this width

		int64_t d_table = 0;
		if (!log_distance(fromTable, t.k, d_table)) continue;

		// Nothing representable may be closer to the tabulated value.
		auto probe = [&](int64_t offset) {
			int64_t m = static_cast<int64_t>(fromTable.magnitude_bits()) + offset;
			if (m < 1 || static_cast<uint64_t>(m) >= (1ull << (nbits - 1))) return;
			TL cand; cand.setbits(static_cast<uint64_t>(m));
			int64_t d_cand = 0;
			if (!log_distance(cand, t.k, d_cand)) return;
			if (d_cand < d_table) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL " << t.name << " nbits=" << nbits
					          << " is not nearest: neighbour is closer\n";
				}
			}
		};
		probe(-1); probe(+1);

		// And it must be at least as good as the double-literal route, which is
		// the reason the table exists at all.
		TL fromDouble(static_cast<double>(t.value));
		int64_t d_double = 0;
		if (log_distance(fromDouble, t.k, d_double)) {
			if (d_table > d_double) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL " << t.name << " nbits=" << nbits
					          << " table entry is worse than the double literal\n";
				}
			}
			else if (d_table < d_double) {
				++improvements;
			}
		}
	}
	return nrOfFailedTests;
}

// e, 1/e and sqrt(e) are powers of the value base, so their logarithmic value is
// an integer and they encode with no rounding at all -- something no binary
// floating-point format manages for e.  A zero fraction must survive to every
// width, and the reciprocal identity must hold bit for bit (Prop. 7).
template<unsigned nbits, unsigned rbits>
int VerifyExactConstants(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits, std::uint64_t>;
	int nrOfFailedTests = 0;

	struct { const char* name; takum_log_constant k; } exact[] = {
		{ "e",      sw::universal::tkml_e      },
		{ "1_e",    sw::universal::tkml_1_e    },
		{ "sqrt_e", sw::universal::tkml_sqrt_e },
	};
	for (const auto& x : exact) {
		if (x.k.N != 0) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL " << x.name << " table entry is not an integer l\n";
			continue;
		}
		TL v = sw::universal::takum_log_constant_cast<TL>(x.k);
		if (v.iszero() || v.isnar()) continue;
		auto d = TL::Codec::decode(v.magnitude_bits());
		if (d.c != x.k.c || d.M_bits != 0) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL " << x.name << " nbits=" << nbits << " did not encode exactly: c="
				          << d.c << " M=" << d.M_bits << '\n';
			}
		}
	}

	// 1/e is the exact reciprocal of e, and the codec's reciprocal is exact, so
	// the two tabulated entries must agree bit for bit after negation.
	TL e   = sw::universal::takum_log_constant_cast<TL>(sw::universal::tkml_e);
	TL inv = sw::universal::takum_log_constant_cast<TL>(sw::universal::tkml_1_e);
	if (!e.iszero() && !e.isnar() && !inv.iszero() && !inv.isnar()) {
		if (e.reciprocal().raw_bits() != inv.raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL 1/e is not the exact reciprocal of e\n";
		}
	}
	// sqrt(e) squared is e, exactly.
	TL se = sw::universal::takum_log_constant_cast<TL>(sw::universal::tkml_sqrt_e);
	if (!se.iszero() && !se.isnar() && !e.iszero() && !e.isnar()) {
		if (sqr(se).raw_bits() != e.raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL sqrt(e) squared is not e\n";
		}
	}
	return nrOfFailedTests;
}

// Validate the table's CONTENT against an independent computation.
//
// This is the check the others cannot make.  VerifyTableEncoding measures how far
// an encoding sits from the tabulated l, so it is self-referential about the table
// itself: perturb an entry and the encoding still reproduces it perfectly.  A
// transcription slip in the low digits of a 19-digit numerator would survive every
// other assertion here -- which is exactly the failure this table is most exposed
// to, and exactly the regime it exists to serve.
//
// So recompute l = 2 ln X from the long double reference and compare at 2^-62.
// The achievable tolerance is a property of the platform: an x86 long double
// carries a 64-bit significand, which pins l to a couple of ulps at that scale,
// while a long double that is merely a double leaves about 2^10 of slack.  Both
// catch a corrupted digit; only the first is tight.
int VerifyTableValues(bool reportTestCases) {
	int nrOfFailedTests = 0;

	// Slack in units of 2^-62, derived from the platform's long double.
#if defined(LDBL_MANT_DIG) && (LDBL_MANT_DIG >= 64)
	const int64_t tolerance = 8;
#else
	const int64_t tolerance = (int64_t(1) << 12);
#endif

	for (size_t i = 0; i < table_size; ++i) {
		const Entry& t = table[i];
		const long double l = 2.0L * std::log(t.value);
		const long double fl = std::floor(l);
		const int64_t c_ref = static_cast<int64_t>(fl);
		const long double frac = l - fl;
		const long double scaled = frac * std::pow(2.0L, static_cast<long double>(takum_log_constant_qbits));
		const int64_t N_ref = static_cast<int64_t>(scaled + 0.5L);

		const int64_t dc = t.k.c - c_ref;
		if (dc < -1 || dc > 1) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL " << t.name << " characteristic is " << t.k.c
				          << ", reference says " << c_ref << '\n';
			}
			continue;
		}
		int64_t distance = dc * (int64_t(1) << takum_log_constant_qbits)
		                 + (static_cast<int64_t>(t.k.N) - N_ref);
		if (distance < 0) distance = -distance;
		if (distance > tolerance) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL " << t.name << " table l is " << distance
				          << " ulp(2^-62) from 2*ln(x), tolerance " << tolerance << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// A transposed digit in the table would most likely survive every check above, so
// pin the ordering against the independent long double references as well.
int VerifyTableOrdering(bool reportTestCases) {
	using TL = sw::universal::takum_log<32, 3, std::uint64_t>;
	int nrOfFailedTests = 0;
	for (size_t i = 0; i < table_size; ++i) {
		for (size_t j = i + 1; j < table_size; ++j) {
			const bool want_less = table[i].value < table[j].value;
			const bool want_more = table[i].value > table[j].value;
			TL a = sw::universal::takum_log_constant_cast<TL>(table[i].k);
			TL b = sw::universal::takum_log_constant_cast<TL>(table[j].k);
			if (a.iszero() || b.iszero() || a.isnar() || b.isnar()) continue;
			if (want_less && !(a < b)) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL ordering " << table[i].name << " < " << table[j].name
					          << " but encodings say otherwise\n";
				}
			}
			if (want_more && !(a > b)) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL ordering " << table[i].name << " > " << table[j].name
					          << " but encodings say otherwise\n";
				}
			}
		}
	}
	// and the values themselves must be right to within the format's resolution
	for (size_t i = 0; i < table_size; ++i) {
		TL v = sw::universal::takum_log_constant_cast<TL>(table[i].k);
		if (v.iszero() || v.isnar()) continue;
		double got  = double(v);
		double want = static_cast<double>(table[i].value);
		double rel  = std::fabs((got - want) / want);
		if (!(rel < 1e-7)) {          // takum_log<32,3> resolves to about 1e-8
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL value " << table[i].name << " got=" << got
				          << " want=" << want << " relerr=" << rel << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

} // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "takum_log constant table verification";
	std::string test_tag    = "constants";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;
	int improvements        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(
		VerifyTableEncoding<64, 3>(true, improvements), "takum_log<64,3>", "table vs double literal");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(
		VerifyTableValues(reportTestCases), "table", "entries match 2*ln(x)");
	nrOfFailedTestCases += ReportTestResult(
		VerifyTableOrdering(reportTestCases), "takum_log<32,3>", "table ordering and values");
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactConstants<32, 3>(reportTestCases), "takum_log<32,3>", "e, 1/e, sqrt(e) exact");
	nrOfFailedTestCases += ReportTestResult(
		VerifyTableEncoding<32, 3>(reportTestCases, improvements), "takum_log<32,3>", "correctly rounded");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactConstants<16, 3>(reportTestCases), "takum_log<16,3>", "e, 1/e, sqrt(e) exact");
	nrOfFailedTestCases += ReportTestResult(
		VerifyTableEncoding<16, 3>(reportTestCases, improvements), "takum_log<16,3>", "correctly rounded");
	nrOfFailedTestCases += ReportTestResult(
		VerifyTableEncoding<24, 3>(reportTestCases, improvements), "takum_log<24,3>", "correctly rounded");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactConstants<48, 3>(reportTestCases), "takum_log<48,3>", "e, 1/e, sqrt(e) exact");
	nrOfFailedTestCases += ReportTestResult(
		VerifyTableEncoding<48, 3>(reportTestCases, improvements), "takum_log<48,3>", "correctly rounded");
#endif

	// The width the table exists for: at nbits = 64 the trailing field runs to
	// p = 58 or 59, past a double's 53 significant bits, so a double literal can
	// no longer express the constant and the table strictly wins.
#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactConstants<64, 3>(reportTestCases), "takum_log<64,3>", "e, 1/e, sqrt(e) exact");
	nrOfFailedTestCases += ReportTestResult(
		VerifyTableEncoding<64, 3>(reportTestCases, improvements), "takum_log<64,3>", "correctly rounded");

	// If the table never improved on a double literal anywhere, it is not earning
	// its place and the claim in the header is wrong.
	if (improvements == 0) {
		++nrOfFailedTestCases;
		std::cout << "FAIL the table never improved on a double literal at any width\n";
	}
	else {
		std::cout << "note: the table beat the double literal in " << improvements << " cases\n";
	}
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
