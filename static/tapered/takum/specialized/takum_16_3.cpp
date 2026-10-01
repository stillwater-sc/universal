// takum_16_3.cpp: bit-for-bit comparison of the fast takum<16,3,uint16_t> with the generic code
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#define TAKUM_FAST_TAKUM_16 1
#include <universal/number/takum/takum.hpp>
#include <universal/verification/test_suite.hpp>
#include <atomic>
#include <cmath>
#include <cstring>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

// The reference is takum<16,3,uint8_t>: the same generic code, which the
// specialization does not touch (it is keyed on the uint16_t block type), so
// both can live in one program without an ODR conflict.
using Fast = sw::universal::takum<16, 3, std::uint16_t>;
using Ref  = sw::universal::takum<16, 3, std::uint8_t>;

template<typename T> T from_bits(std::uint64_t b) { T t; t.setbits(b); return t; }
template<typename Real> std::uint64_t bits_of(Real v) {
	std::uint64_t u = 0; std::memcpy(&u, &v, sizeof(Real)); return u;
}

// failure log shared by the worker threads; prints the first few
struct Log {
	std::atomic<std::uint64_t> fails{ 0 };
	std::mutex m;
	void fail(const char* what, std::uint64_t a, std::uint64_t b, std::uint64_t got, std::uint64_t want) {
		if (++fails > 10) return;
		std::lock_guard<std::mutex> g(m);
		std::cerr << "FAIL " << what << std::hex << " a=0x" << a << " b=0x" << b
		          << " fast=0x" << got << " generic=0x" << want << std::dec << '\n';
	}
	int result() const { return fails > 0 ? 1 : 0; }
};

// f(i) for i in [0, n), across all hardware threads
template<typename F> void parallel_for(std::uint64_t n, F f) {
	unsigned T = std::max(1u, std::thread::hardware_concurrency());
	std::atomic<std::uint64_t> next{ 0 };
	std::vector<std::thread> pool;
	for (unsigned t = 0; t < T; ++t)
		pool.emplace_back([&] { for (std::uint64_t i; (i = next++) < n; ) f(i); });
	for (auto& th : pool) th.join();
}

// every unary operation, conversion to native types, and the selectors, over all 2^16 patterns
int VerifyUnary() {
	using namespace sw::universal;
	Log lg;
	for (std::uint64_t i = 0; i < 65536; ++i) {
		Fast f = from_bits<Fast>(i); Ref r = from_bits<Ref>(i);
		auto T = [&](const char* w, auto a, auto b) { if (a.raw_bits() != b.raw_bits()) lg.fail(w, i, 0, a.raw_bits(), b.raw_bits()); };
		auto V = [&](const char* w, std::uint64_t a, std::uint64_t b) { if (a != b) lg.fail(w, i, 0, a, b); };
		T("-x", -f, -r);  T("abs", abs(f), abs(r));  T("sqrt", sqrt(f), sqrt(r));  T("rsqrt", rsqrt(f), rsqrt(r));
		{ Fast a(f); Ref b(r); T("++x", ++a, ++b); T("x++", a++, b++); T("x after ++", a, b); }
		{ Fast a(f); Ref b(r); T("--x", --a, --b); T("x--", a--, b--); T("x after --", a, b); }
		T("exp", exp(f), exp(r));  T("log", log(f), log(r));  T("sin", sin(f), sin(r));  T("pow", pow(f, f), pow(r, r));
		T("fma", fma(f, f, f), fma(r, r, r));  T("ulp", ulp(f), ulp(r));
		T("takum(double(x))", Fast(double(f)), Ref(double(r)));
		T("takum(float(x))", Fast(float(f)), Ref(float(r)));
		V("double(x)", bits_of(double(f)), bits_of(double(r)));
		V("float(x)", bits_of(float(f)), bits_of(float(r)));
		if (!f.isnar() && std::fabs(double(r)) < 2147483647.0) {
			V("int(x)", std::uint64_t(int(f)), std::uint64_t(int(r)));
			V("long long(x)", std::uint64_t((long long)(f)), std::uint64_t((long long)(r)));
		}
		V("predicates", (f.isnar() << 0) | (f.iszero() << 1) | (f.isneg() << 2) | (f.ispos() << 3) | (f.sign() << 4) | (f.isinf() << 5),
		                (r.isnar() << 0) | (r.iszero() << 1) | (r.isneg() << 2) | (r.ispos() << 3) | (r.sign() << 4) | (r.isinf() << 5));
		V("scale", std::uint64_t(f.scale()), std::uint64_t(r.scale()));
	}
	return lg.result();
}

int VerifySpecialValues() {
	using namespace sw::universal;
	Log lg;
	SpecificValue codes[] = { SpecificValue::maxpos, SpecificValue::minpos, SpecificValue::zero, SpecificValue::minneg,
	                          SpecificValue::maxneg, SpecificValue::infpos, SpecificValue::infneg, SpecificValue::qnan,
	                          SpecificValue::snan, SpecificValue::nar };
	for (auto c : codes) if (Fast(c).raw_bits() != Ref(c).raw_bits()) lg.fail("SpecificValue", unsigned(c), 0, Fast(c).raw_bits(), Ref(c).raw_bits());
	Fast f; Ref r;
	if (f.maxpos().raw_bits() != r.maxpos().raw_bits()) lg.fail("maxpos", 0, 0, f.raw_bits(), r.raw_bits());
	if (f.minpos().raw_bits() != r.minpos().raw_bits()) lg.fail("minpos", 0, 0, f.raw_bits(), r.raw_bits());
	if (f.maxneg().raw_bits() != r.maxneg().raw_bits()) lg.fail("maxneg", 0, 0, f.raw_bits(), r.raw_bits());
	if (f.minneg().raw_bits() != r.minneg().raw_bits()) lg.fail("minneg", 0, 0, f.raw_bits(), r.raw_bits());
	f.setnar(); r.setnar(); if (f.raw_bits() != r.raw_bits()) lg.fail("setnar", 0, 0, f.raw_bits(), r.raw_bits());
	using LF = std::numeric_limits<Fast>; using LR = std::numeric_limits<Ref>;
	Fast lf[] = { LF::min(), LF::max(), LF::lowest(), LF::epsilon(), LF::round_error(), LF::denorm_min() };
	Ref  lr[] = { LR::min(), LR::max(), LR::lowest(), LR::epsilon(), LR::round_error(), LR::denorm_min() };
	for (int k = 0; k < 6; ++k) if (lf[k].raw_bits() != lr[k].raw_bits()) lg.fail("numeric_limits", k, 0, lf[k].raw_bits(), lr[k].raw_bits());
	return lg.result();
}

// conversion from double over a fixed set of values; the caller supplies the doubles
int VerifyFromDoubles(const std::vector<double>& v) {
	Log lg;
	parallel_for((v.size() + 4095) / 4096, [&](std::uint64_t blk) {
		for (std::size_t i = blk * 4096; i < std::min(v.size(), std::size_t(blk * 4096 + 4096)); ++i) {
			std::uint64_t a = Fast(v[i]).raw_bits(), b = Ref(v[i]).raw_bits();
			if (a != b) lg.fail("takum(double)", bits_of(v[i]), 0, a, b);
		}
	});
	return lg.result();
}

// every takum16 value, every midpoint between neighbours (including 0 | minpos and
// maxpos | 2*maxpos-ish), both doubles adjacent to each, and the special values
std::vector<double> StructuredDoubles() {
	std::vector<double> v = { 0.0, -0.0, std::nan(""), -std::nan(""), INFINITY, -INFINITY,
	                          DBL_MAX, -DBL_MAX, DBL_MIN, -DBL_MIN, DBL_TRUE_MIN, -DBL_TRUE_MIN,
	                          0x1p-255, 0x1p-256, 0x1p254, 0x1p255, 0x1p256 };
	Ref lo; lo.minpos(); Ref hi; hi.maxpos();
	for (double x : { double(lo) * 0.5, double(lo) * 0.7, double(lo) * 0.99, double(hi) * (1 + 0x1p-5), double(hi) * 2 }) v.push_back(x);
	std::vector<double> pos;                         // positive takum16 values in increasing order
	for (std::uint64_t i = 1; i < 0x8000; ++i) pos.push_back(double(from_bits<Ref>(i)));
	pos.push_back(2.0 * pos.back() - pos[pos.size() - 2]);  // one step past maxpos
	std::vector<double> pts = { 0.0 };
	for (std::size_t i = 0; i < pos.size(); ++i) pts.push_back(pos[i]), pts.push_back(0.5 * ((i ? pos[i - 1] : 0.0) + pos[i]));
	for (double x : pts) for (double y : { x, std::nextafter(x, 0.0), std::nextafter(x, INFINITY) }) { v.push_back(y); v.push_back(-y); }
	return v;
}

// n doubles: half uniform over all bit patterns (every exponent, NaNs, subnormals),
// half with the exponent uniform over the takum16 range plus a margin
std::vector<double> RandomDoubles(std::size_t n, std::uint64_t seed) {
	std::mt19937_64 rng(seed);
	std::vector<double> v(n);
	for (std::size_t i = 0; i < n; ++i) {
		std::uint64_t u = rng();
		if (i & 1) u = (u & 0x800F'FFFF'FFFF'FFFFull) | (std::uint64_t(1023 - 270 + (u >> 52) % 540) << 52);
		std::memcpy(&v[i], &u, 8);
	}
	return v;
}

int VerifyFromIntegers() {
	Log lg;
	std::mt19937_64 rng(3);
	for (int i = -70000; i <= 70000; ++i)
		if (Fast(i).raw_bits() != Ref(i).raw_bits()) lg.fail("takum(int)", std::uint64_t(i), 0, Fast(i).raw_bits(), Ref(i).raw_bits());
	for (int k = 0; k < 1000000; ++k) {
		long long ll = static_cast<long long>(rng()) >> (rng() % 64);
		unsigned long long ull = rng() >> (rng() % 64);
		if (Fast(ll).raw_bits() != Ref(ll).raw_bits()) lg.fail("takum(long long)", std::uint64_t(ll), 0, Fast(ll).raw_bits(), Ref(ll).raw_bits());
		if (Fast(ull).raw_bits() != Ref(ull).raw_bits()) lg.fail("takum(unsigned long long)", ull, 0, Fast(ull).raw_bits(), Ref(ull).raw_bits());
	}
	return lg.result();
}

// float patterns [begin, end), stride step
int VerifyFromFloats(std::uint64_t begin, std::uint64_t end, std::uint64_t step) {
	Log lg;
	std::uint64_t n = (end - begin + step - 1) / step, chunk = 1u << 16;
	parallel_for((n + chunk - 1) / chunk, [&](std::uint64_t blk) {
		for (std::uint64_t k = blk * chunk; k < std::min(n, blk * chunk + chunk); ++k) {
			std::uint32_t u = std::uint32_t(begin + k * step); float x; std::memcpy(&x, &u, 4);
			std::uint64_t a = Fast(x).raw_bits(), b = Ref(x).raw_bits();
			if (a != b) lg.fail("takum(float)", u, 0, a, b);
		}
	});
	return lg.result();
}

// binary operators and comparisons for every a against nb operands b: all 2^16 when
// exhaustive, otherwise a fixed pseudo-random set
enum Op { ADD, SUB, MUL, DIV, ADDD, MULD, CMP };
const char* op_name[] = { "+", "-", "*", "/", "+= double", "*= double", "comparisons" };
int VerifyBinary(Op op, bool exhaustive) {
	Log lg;
	parallel_for(65536, [&](std::uint64_t i) {
		Fast fa = from_bits<Fast>(i); Ref ra = from_bits<Ref>(i);
		std::mt19937 rng(static_cast<unsigned>(i));
		for (std::uint64_t k = 0; k < (exhaustive ? 65536u : 64u); ++k) {
			std::uint64_t j = exhaustive ? k : (rng() & 0xFFFF);
			Fast fb = from_bits<Fast>(j); Ref rb = from_bits<Ref>(j);
			std::uint64_t a = 0, b = 0;
			switch (op) {
			case ADD:  a = (fa + fb).raw_bits(); b = (ra + rb).raw_bits(); break;
			case SUB:  a = (fa - fb).raw_bits(); b = (ra - rb).raw_bits(); break;
			case MUL:  a = (fa * fb).raw_bits(); b = (ra * rb).raw_bits(); break;
			case DIV:  a = (fa / fb).raw_bits(); b = (ra / rb).raw_bits(); break;
			case ADDD: { Fast x(fa); Ref y(ra); x += double(rb) * 1.3; y += double(rb) * 1.3; a = x.raw_bits(); b = y.raw_bits(); break; }
			case MULD: { Fast x(fa); Ref y(ra); x *= double(rb) * 1.3; y *= double(rb) * 1.3; a = x.raw_bits(); b = y.raw_bits(); break; }
			case CMP:
				a = (fa == fb) | (fa != fb) << 1 | (fa < fb) << 2 | (fa > fb) << 3 | (fa <= fb) << 4 | (fa >= fb) << 5;
				b = (ra == rb) | (ra != rb) << 1 | (ra < rb) << 2 | (ra > rb) << 3 | (ra <= rb) << 4 | (ra >= rb) << 5;
				break;
			}
			if (a != b) lg.fail(op_name[op], i, j, a, b);
		}
	});
	return lg.result();
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
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
// the opt-in tkm_takum_16_3_exhaustive target: minutes on a multi-core machine
#ifdef TAKUM16_EXHAUSTIVE
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fast takum<16,3,uint16_t> against the generic implementation";
	std::string test_tag    = "takum_16_3";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyUnary(), "all 2^16 unary ops and conversions", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues(), "special values", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFromDoubles(StructuredDoubles()), "values, midpoints, neighbours", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFromDoubles(RandomDoubles(1000000, 1)), "10^6 random doubles", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFromIntegers(), "conversion from integers", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFromFloats(0, 1ull << 32, 4099), "sampled floats", test_tag);
	for (Op op : { ADD, SUB, MUL, DIV, ADDD, MULD, CMP })
		nrOfFailedTestCases += ReportTestResult(VerifyBinary(op, false), std::string("sampled ") + op_name[op], test_tag);
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
	for (Op op : { ADD, SUB, MUL, DIV, CMP })
		nrOfFailedTestCases += ReportTestResult(VerifyBinary(op, true), std::string("all 2^32 pairs ") + op_name[op], test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFromFloats(0, 1ull << 32, 1), "all 2^32 floats", test_tag);
	for (std::uint64_t seed = 0; seed < 10; ++seed)
		nrOfFailedTestCases += ReportTestResult(VerifyFromDoubles(RandomDoubles(10000000, 100 + seed)), "10^7 random doubles", test_tag);
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
