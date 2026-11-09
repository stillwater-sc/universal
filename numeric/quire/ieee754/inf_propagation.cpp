//  inf_propagation.cpp : the generalized quire keeps signed infinities apart from NaN (#2019)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// IEEE-754 keeps an infinity an infinity: a dot product with an infinite term and
// no NaN is that signed infinity, and only opposite infinities, inf * 0 or a NaN
// operand make it NaN. The quire records +inf, -inf and NaN as separate sticky
// states and resolves them that way for float, double and cfloat; a posit, which
// has no infinities, still resolves any non-finite state to NaR.
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/fdp.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/quire/quire.hpp>   // brings in the native float/double quire_mul and quire_resolve
#include <universal/verification/test_reporters.hpp>

#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

int g_failures = 0;
bool g_report  = true;

void expect(bool cond, const std::string& tag, const std::string& what) {
	if (!cond) {
		++g_failures;
		if (g_report) std::cout << "FAIL(" << tag << "): " << what << '\n';
	}
}

// the same special-value class and, for an infinity, the same sign
template<typename Real>
bool same_special(Real got, Real want) {
	if (std::isnan(want)) return std::isnan(got);
	if (std::isinf(want)) return std::isinf(got) && std::signbit(got) == std::signbit(want);
	return got == want;
}

template<typename Real>
std::string show(Real v) { return std::to_string(v); }

// accumulate the terms one at a time; the reference is the plain IEEE sum
template<typename Real>
void VerifySumTable(const std::string& tag) {
	using namespace sw::universal;
	const Real inf = std::numeric_limits<Real>::infinity();
	const Real nan = std::numeric_limits<Real>::quiet_NaN();
	const std::vector<std::vector<Real>> cases = {
		{ Real(1), inf },          // 1 + inf       = +inf
		{ inf, inf },              // inf + inf     = +inf
		{ -inf, -inf },            // -inf + -inf   = -inf
		{ inf, -inf },             // inf + -inf    = NaN
		{ nan },                   // NaN
		{ -inf, Real(1), Real(2) },// later finite terms leave -inf alone
		{ Real(1), -inf, inf },    // the opposite infinity still turns it NaN
		{ inf, nan },              // a NaN after an infinity is still NaN
		{ nan, inf },              // and an infinity after a NaN does not clear it
		{ Real(3), Real(-1) },     // the finite path is unchanged
	};
	for (const auto& terms : cases) {
		quire<Real> q;
		Real ref = Real(0);
		std::string text;
		for (Real t : terms) {
			q += quire_mul(t, Real(1));
			ref += t;
			text += show(t) + " ";
		}
		Real got = quire_resolve(q);
		expect(same_special(got, ref), tag, "sum of { " + text + "} resolved to " + show(got) + ", IEEE gives " + show(ref));
		expect(q.isnan() == bool(std::isnan(ref)), tag, "isnan() for { " + text + "}");
		expect(q.isinf() == bool(std::isinf(ref)), tag, "isinf() for { " + text + "}");
		if (std::isinf(ref)) expect(q.sign() == std::signbit(ref), tag, "sign() of the infinity for { " + text + "}");
	}
}

// the product: inf * nonzero is an infinity signed by the XOR, inf * 0 and NaN are NaN
template<typename Real>
void VerifyProducts(const std::string& tag) {
	using namespace sw::universal;
	const Real inf = std::numeric_limits<Real>::infinity();
	const Real nan = std::numeric_limits<Real>::quiet_NaN();
	const std::vector<std::pair<Real, Real>> cases = {
		{ inf, Real(2) }, { -inf, Real(2) }, { Real(2), -inf }, { -inf, Real(-3) },
		{ inf, Real(-0.0) }, { Real(0), -inf }, { inf, nan }, { nan, Real(1) }, { inf, -inf },
	};
	for (auto [a, b] : cases) {
		auto p = quire_mul(a, b);
		Real ref = a * b;
		std::string what = show(a) + " * " + show(b);
		expect(p.isnan() == bool(std::isnan(ref)), tag, "quire_mul(" + what + ") NaN state");
		expect(p.isinf() == bool(std::isinf(ref)), tag, "quire_mul(" + what + ") inf state");
		if (std::isinf(ref)) expect(p.sign() == std::signbit(ref), tag, "quire_mul(" + what + ") sign");

		quire<Real> q;
		q += p;
		Real got = quire_resolve(q);
		expect(same_special(got, ref), tag, what + " through the quire resolved to " + show(got) + ", IEEE gives " + show(ref));
	}
}

// fused dot products over vectors with mixed infinities
template<typename Real>
void VerifyDotProducts(const std::string& tag) {
	using namespace sw::universal;
	const Real inf = std::numeric_limits<Real>::infinity();
	struct Case { std::vector<Real> x, y; };
	const std::vector<Case> cases = {
		{ { Real(1), inf, Real(3) },  { Real(2), Real(-2), Real(4) } },   // -inf
		{ { inf, -inf },              { Real(1), Real(1) } },             // NaN
		{ { inf, inf },               { Real(-1), Real(-1) } },           // -inf
		{ { inf, Real(1) },           { Real(0), Real(1) } },             // inf * 0: NaN
		{ { -inf, Real(5), inf },     { Real(-1), Real(7), Real(2) } },   // +inf twice
		{ { Real(0.5), Real(0.25) },  { Real(4), Real(8) } },             // finite: 4
	};
	for (const auto& c : cases) {
		quire<Real> q;
		Real ref = Real(0);
		for (std::size_t i = 0; i < c.x.size(); ++i) {
			q += quire_mul(c.x[i], c.y[i]);
			ref += c.x[i] * c.y[i];
		}
		Real got = quire_resolve(q);
		expect(same_special(got, ref), tag, "dot product resolved to " + show(got) + ", IEEE gives " + show(ref));
	}
}

// selectors, modifiers, quire + quire, comparisons and printing of each state
template<typename Real>
void VerifyStates(const std::string& tag) {
	using namespace sw::universal;
	using Q = quire<Real>;
	const Real inf = std::numeric_limits<Real>::infinity();

	Q pinf(SpecificValue::infpos), ninf(SpecificValue::infneg), nan(SpecificValue::qnan);
	Q one;  one += quire_mul(Real(1), Real(1));
	Q zero;

	expect( pinf.isinf() && !pinf.isnan() &&  pinf.isnar() && !pinf.sign() && !pinf.iszero(), tag, "+inf selectors");
	expect( ninf.isinf() && !ninf.isnan() &&  ninf.isnar() &&  ninf.sign() && !ninf.iszero(), tag, "-inf selectors");
	expect(!nan.isinf()  &&  nan.isnan()  &&  nan.isnar()  && !nan.iszero(),                   tag, "NaN selectors");
	expect(!one.isinf()  && !one.isnan()  && !one.isnar(),                                     tag, "finite selectors");

	Q s; s.setinf(true);
	expect(s.isinf() && s.sign() && quire_resolve(s) == -inf, tag, "setinf(true) is -inf");
	s.setinf(false);
	expect(s.isinf() && !s.sign() && quire_resolve(s) == inf, tag, "setinf(false) is +inf");
	s.setnan();
	expect(s.isnan() && !s.isinf(), tag, "setnan() is NaN");

	// a finite assignment clears the non-finite state
	Q a(pinf);
	a = quire_mul(Real(2), Real(3));
	expect(!a.isnar() && quire_resolve(a) == Real(6), tag, "finite assignment after +inf resolves to 6");
	a = quire_mul(-inf, Real(1));
	expect(a.isinf() && a.sign(), tag, "assigning -inf sets the -inf state");
	a.clear();
	expect(a.iszero() && !a.isnar(), tag, "clear() leaves the non-finite state");

	// quire + quire
	expect((pinf + one).isinf() && !(pinf + one).sign(), tag, "+inf + 1 is +inf");
	expect((one + ninf).isinf() &&  (one + ninf).sign(), tag, "1 + -inf is -inf");
	expect((pinf + pinf).isinf(),                        tag, "+inf + +inf is +inf");
	expect((pinf + ninf).isnan(),                        tag, "+inf + -inf is NaN");
	expect((nan + pinf).isnan() && (pinf + nan).isnan(), tag, "NaN absorbs an infinity");
	Q d(pinf);  d -= pinf;
	expect(d.isnan(), tag, "+inf - +inf is NaN");
	Q e(one);   e -= ninf;
	expect(e.isinf() && !e.sign(), tag, "1 - -inf is +inf");
	Q f;  f -= quire_mul(inf, Real(1));
	expect(f.isinf() && f.sign(), tag, "0 - inf (blocktriple) is -inf");

	// abs
	expect(abs(ninf).isinf() && !abs(ninf).sign(), tag, "abs(-inf) is +inf");

	// comparisons: an infinite quire compares as that infinity; NaN is unordered
	expect(  pinf == pinf  && !(pinf != pinf), tag, "+inf == +inf");
	expect(!(pinf == ninf) &&  (pinf != ninf), tag, "+inf != -inf");
	expect(  ninf <  pinf  &&   ninf <  one  &&  one < pinf && zero < pinf, tag, "-inf < 1 < +inf");
	expect(!(pinf <  pinf) && !(ninf <  ninf) && !(pinf < one) && !(one < ninf), tag, "no value below -inf or above +inf");
	expect(  pinf <= pinf  &&   pinf >= one  &&  ninf <= one && ninf >= ninf, tag, "<= and >= with infinities");
	expect(  pinf >  one   &&   one  >  ninf, tag, "> with infinities");
	expect(!(nan == pinf) && !(nan < pinf) && !(pinf < nan) && !(nan <= pinf) && !(pinf >= nan), tag, "NaN unordered against inf");

	// conversion and printing
	expect(pinf.template convert_to<double>() ==  std::numeric_limits<double>::infinity(), tag, "convert_to<double>(+inf)");
	expect(ninf.template convert_to<double>() == -std::numeric_limits<double>::infinity(), tag, "convert_to<double>(-inf)");
	expect(pinf.template convert_to<int>() == std::numeric_limits<int>::max(),    tag, "convert_to<int>(+inf) saturates");
	expect(ninf.template convert_to<int>() == std::numeric_limits<int>::lowest(), tag, "convert_to<int>(-inf) saturates");
	expect(pinf.to_blocktriple().isinf() && !pinf.to_blocktriple().sign(), tag, "to_blocktriple(+inf)");
	expect(ninf.to_blocktriple().isinf() &&  ninf.to_blocktriple().sign(), tag, "to_blocktriple(-inf)");
	expect(to_binary(pinf) == "+inf" && to_binary(ninf) == "-inf" && to_binary(nan) == "nar", tag, "to_binary of the non-finite states");
}

// cfloat goes through its own quire_mul and quire_resolve
template<typename Cfloat>
void VerifyCfloat(const std::string& tag) {
	using namespace sw::universal;
	Cfloat inf; inf.setinf(false);
	Cfloat ninf; ninf.setinf(true);
	Cfloat nan; nan.setnan(NAN_TYPE_QUIET);
	Cfloat two(2.0f), zero(0.0f), one(1.0f);

	auto resolve = [](std::initializer_list<std::pair<Cfloat, Cfloat>> terms) {
		quire<Cfloat> q;
		for (auto [a, b] : terms) q += quire_mul(a, b);
		return quire_resolve(q);
	};
	Cfloat r;
	r = resolve({ { inf, two } });               expect(r.isinf() && !r.sign(), tag, "inf * 2 is +inf");
	r = resolve({ { inf, -two } });              expect(r.isinf() &&  r.sign(), tag, "inf * -2 is -inf");
	r = resolve({ { ninf, -two } });             expect(r.isinf() && !r.sign(), tag, "-inf * -2 is +inf");
	r = resolve({ { inf, zero } });              expect(r.isnan(),              tag, "inf * 0 is NaN");
	r = resolve({ { inf, nan } });               expect(r.isnan(),              tag, "inf * NaN is NaN");
	r = resolve({ { one, one }, { ninf, one } });expect(r.isinf() &&  r.sign(), tag, "1 + -inf is -inf");
	r = resolve({ { inf, one }, { ninf, one } });expect(r.isnan(),              tag, "inf + -inf is NaN");
	r = resolve({ { two, two }, { one, one } }); expect(!r.isnan() && !r.isinf() && double(r) == 5.0, tag, "finite sum is 5");
}

// a posit has no infinities: an infinite quire still resolves to NaR
template<typename Posit>
void VerifyPositNaR(const std::string& tag) {
	using namespace sw::universal;
	quire<Posit> pinf(SpecificValue::infpos), ninf(SpecificValue::infneg);
	expect(quire_resolve(pinf).isnar(), tag, "+inf quire resolves to NaR");
	expect(quire_resolve(ninf).isnar(), tag, "-inf quire resolves to NaR");
	Posit nar; nar.setnar();
	quire<Posit> q; q += quire_mul(nar, Posit(1));
	expect(q.isnan() && quire_resolve(q).isnar(), tag, "NaR product resolves to NaR");
}

}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 1
#	define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "quire signed infinity propagation (#2019)";
	std::string test_tag    = "quire inf";
	bool reportTestCases    = true;
	g_report                = reportTestCases;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	VerifySumTable<float>("float");

	ReportTestSuiteResults(test_suite, g_failures);
	return EXIT_SUCCESS;  // ignore errors
#else

#	if REGRESSION_LEVEL_1
	VerifySumTable<float>("float");
	VerifySumTable<double>("double");
	VerifyProducts<float>("float");
	VerifyProducts<double>("double");
	VerifyDotProducts<float>("float");
	VerifyDotProducts<double>("double");
	VerifyStates<float>("float");
	VerifyStates<double>("double");
	VerifyCfloat<cfloat<32, 8, std::uint32_t, true, false, false>>("cfloat<32,8>");
	VerifyCfloat<cfloat<16, 5, std::uint16_t, true, false, false>>("cfloat<16,5>");
	VerifyPositNaR<posit<16, 1>>("posit<16,1>");
#	endif

#	if REGRESSION_LEVEL_2
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
#	endif

	ReportTestSuiteResults(test_suite, g_failures);
	return (g_failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
