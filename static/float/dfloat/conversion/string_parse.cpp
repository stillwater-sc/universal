// string_parse.cpp: comprehensive regression tests for decimal-string parsing
//                  of dfloat (Resolves #852, extending Phase E of #835)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// Phase E (PR #858) added operator>> hygiene plus a grammar pre-validator
// inside dfloat::parse() but only spot-tested decimal64 (BID). This file
// now exercises the full AC matrix from #852:
//   - decimal32 / decimal64 / decimal128 (BID encoding)
//   - decimal32_dpd / decimal64_dpd / decimal128_dpd (DPD encoding)
//   - decimal floating-point literals and integer literals
//   - nan / inf / infinity tokens (case-insensitive, optional sign) -- with
//     resulting state asserted, not just parse-success boolean
//   - Empty / no-digit rejection
//   - Trailing-junk rejection
//   - operator>> failbit on a bad token + EOF
//
// hfloat-style native hex format is NOT defined for dfloat; that bullet
// of the AC is intentionally a no-op.

#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_reporters.hpp>

namespace {
struct CerrSilencer {
	std::ostringstream sink;
	std::streambuf*    old;
	CerrSilencer() : old(std::cerr.rdbuf(sink.rdbuf())) {}
	~CerrSilencer() { std::cerr.rdbuf(old); }
	CerrSilencer(const CerrSilencer&)            = delete;
	CerrSilencer& operator=(const CerrSilencer&) = delete;
};

// Verify canonical-decimal round-trips for a given dfloat configuration.
// Each input string is parsed; the result is compared against the value
// obtained by constructing the same dfloat from the corresponding double.
// Caller picks inputs that are exact in double (so the comparison is a
// meaningful test of parse, not a precision-loss exercise).
template <typename Dfloat>
int check_canonical(bool reportTestCases, const char* label) {
	int failures = 0;
	struct Case { const char* s; double v; };
	const Case cases[] = {
		{ "0",     0.0   },
		{ "0.0",   0.0   },
		{ "1.0",   1.0   },
		{ "-1.0", -1.0   },
		{ "1.5",   1.5   },
		{ "-3.25", -3.25 },
		{ "0.5",   0.5   },
		{ "2.0",   2.0   },
		{ "1.25e3", 1250.0 },
		{ "42",    42.0  },
		{ "-1000", -1000.0 },
		{ "0.0625", 0.0625 },  // 2^-4, exact in any binary or decimal
	};
	for (const auto& c : cases) {
		Dfloat ours, ref(c.v);
		if (!parse(c.s, ours)) {
			++failures;
			if (reportTestCases) std::cout << "  " << label << " parse failed: " << c.s << '\n';
			continue;
		}
		if (ours != ref) {
			++failures;
			if (reportTestCases) std::cout << "  " << label << " mismatch on \"" << c.s << "\"\n";
		}
	}
	return failures;
}

// nan / inf state assertions across all spelling combinations.
//
// NaN-sign note: like cfloat, dfloat encodes only QUIET NaN (and a
// separate SIGNALLING NaN reachable via setnan(NAN_TYPE_SIGNALLING)).
// `parse()` routes every "nan" spelling -- with or without leading sign
// -- to setnan(NAN_TYPE_QUIET), which has sign=0. There is no IEEE-style
// per-call NaN sign preservation. We only assert that `+nan` / `-nan`
// parse to A NaN; sign is intentionally NOT asserted for those.
template <typename Dfloat>
int check_special(bool reportTestCases, const char* label) {
	int failures = 0;
	Dfloat p;
	// NaN spellings (state: isnan true). Sign-bit assertion omitted per the
	// documented limitation above.
	for (const char* s : { "nan", "NaN", "NAN", "+nan", "-nan" }) {
		if (!parse(s, p)) { ++failures; if (reportTestCases) std::cout << "  " << label << " nan parse fail: " << s << '\n'; continue; }
		if (!p.isnan())   { ++failures; if (reportTestCases) std::cout << "  " << label << " not isnan: " << s << '\n'; }
	}
	// Positive infinity spellings (state: isinf true, sign false)
	for (const char* s : { "inf", "Inf", "infinity", "INFINITY", "+inf" }) {
		if (!parse(s, p)) { ++failures; if (reportTestCases) std::cout << "  " << label << " inf parse fail: " << s << '\n'; continue; }
		if (!p.isinf())   { ++failures; if (reportTestCases) std::cout << "  " << label << " not isinf: " << s << '\n'; }
		if (p.sign())     { ++failures; if (reportTestCases) std::cout << "  " << label << " unexpectedly negative: " << s << '\n'; }
	}
	// Negative infinity spellings (state: isinf true, sign true)
	for (const char* s : { "-inf", "-Inf", "-infinity" }) {
		if (!parse(s, p)) { ++failures; if (reportTestCases) std::cout << "  " << label << " neg-inf parse fail: " << s << '\n'; continue; }
		if (!p.isinf())   { ++failures; if (reportTestCases) std::cout << "  " << label << " not isinf: " << s << '\n'; }
		if (!p.sign())    { ++failures; if (reportTestCases) std::cout << "  " << label << " expected negative: " << s << '\n'; }
	}
	return failures;
}

// Malformed input rejection: confirms the grammar pre-validator rejects
// strings that aren't well-formed decimal floating-point or recognized
// special-value tokens.
template <typename Dfloat>
int check_malformed(bool reportTestCases, const char* label) {
	int failures = 0;
	Dfloat p;
	for (const char* s : {
	     // Empty / no-digit
	     "", "   ", "+", "-", ".",
	     // Bare exponent / partial exponent
	     "e10", "1e", "1ea", "1.e", "+e5", "+.",
	     // Multiple decimal points / trailing junk
	     "1.5.0", "1.5abc", "1abc", "abc",
	     // Double sign
	     "++1", "--1",
	     // Garbled special tokens
	     "nax", "infx", "naninf", "nfini",
	}) {
		if (parse(s, p)) {
			++failures;
			if (reportTestCases) std::cout << "  " << label << " unexpectedly accepted: \"" << s << "\"\n";
		}
	}
	return failures;
}
}  // namespace

int main()
try {
	using namespace sw::universal;
	std::string test_suite  = "dfloat comprehensive string parse (issue #852)";
	bool reportTestCases    = false;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- decimal32 (BID, 7 digits, es=6) -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += check_canonical<decimal32>(reportTestCases, "decimal32");
		nrOfFailedTestCases += check_special<decimal32>(reportTestCases, "decimal32");
		nrOfFailedTestCases += check_malformed<decimal32>(reportTestCases, "decimal32");
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: decimal32 (BID)\n";
	}

	// ----- decimal64 (BID, 16 digits, es=8) -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += check_canonical<decimal64>(reportTestCases, "decimal64");
		nrOfFailedTestCases += check_special<decimal64>(reportTestCases, "decimal64");
		nrOfFailedTestCases += check_malformed<decimal64>(reportTestCases, "decimal64");
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: decimal64 (BID)\n";
	}

	// ----- decimal128 (BID, 34 digits, es=12) -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += check_canonical<decimal128>(reportTestCases, "decimal128");
		nrOfFailedTestCases += check_special<decimal128>(reportTestCases, "decimal128");
		nrOfFailedTestCases += check_malformed<decimal128>(reportTestCases, "decimal128");
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: decimal128 (BID)\n";
	}

	// ----- decimal32_dpd (DPD encoding) -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += check_canonical<decimal32_dpd>(reportTestCases, "decimal32_dpd");
		nrOfFailedTestCases += check_special<decimal32_dpd>(reportTestCases, "decimal32_dpd");
		nrOfFailedTestCases += check_malformed<decimal32_dpd>(reportTestCases, "decimal32_dpd");
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: decimal32_dpd (DPD)\n";
	}

	// ----- decimal64_dpd (DPD encoding) -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += check_canonical<decimal64_dpd>(reportTestCases, "decimal64_dpd");
		nrOfFailedTestCases += check_special<decimal64_dpd>(reportTestCases, "decimal64_dpd");
		nrOfFailedTestCases += check_malformed<decimal64_dpd>(reportTestCases, "decimal64_dpd");
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: decimal64_dpd (DPD)\n";
	}

	// ----- decimal128_dpd (DPD encoding) -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += check_canonical<decimal128_dpd>(reportTestCases, "decimal128_dpd");
		nrOfFailedTestCases += check_special<decimal128_dpd>(reportTestCases, "decimal128_dpd");
		nrOfFailedTestCases += check_malformed<decimal128_dpd>(reportTestCases, "decimal128_dpd");
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: decimal128_dpd (DPD)\n";
	}

	// ----- Large scientific notation that's still finite in dfloat range -----
	{
		int start = nrOfFailedTestCases;
		decimal128 p;
		// decimal128 has 34 digits of precision and large exponent range
		for (const char* s : { "1e10", "1e-10", "1e50", "1e-50", "1.5e30",
		                       "3.14159265358979323846e100" }) {
			if (!parse(s, p)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  parse failed: " << s << '\n';
				continue;  // skip downstream checks to avoid spurious failures
			}
			if (p.iszero()) ++nrOfFailedTestCases;
			if (p.isnan() || p.isinf()) ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: decimal128 large scientific notation\n";
	}

	// ----- operator>> sets failbit on a bad token -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("not-a-number");
		decimal64 p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: dfloat operator>> failbit\n";
	}

	// ----- operator>> handles EOF (empty stream) -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("");
		decimal64 p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: dfloat operator>> EOF\n";
	}

	// ----- operator>> reads a stream of canonical inputs in sequence -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("1.5 -3.25 1e10 nan inf");
		decimal64 p, ref;
		ref = 1.5; is >> p; if (is.fail() || p != ref) ++nrOfFailedTestCases;
		ref = -3.25; is >> p; if (is.fail() || p != ref) ++nrOfFailedTestCases;
		ref = 1e10; is >> p; if (is.fail() || p != ref) ++nrOfFailedTestCases;
		is >> p; if (is.fail() || !p.isnan()) ++nrOfFailedTestCases;
		is >> p; if (is.fail() || !p.isinf() || p.sign()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: dfloat operator>> stream sequence\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
