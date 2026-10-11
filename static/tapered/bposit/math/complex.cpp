// complex.cpp: sw::universal::complex<bposit>, arithmetic and functions (#1653)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// complex<bposit> carries its arithmetic in bposit arithmetic, so + - * are checked
// against the same expressions written out in bposits.  The functions go through
// std::complex<double> and round each component once; they are checked against
// std::complex<long double> to within one encoding per component.  The last check is
// the reason the issue exists: an FFT in the DSP-fitted bposit<16,4,2>, with its
// twiddles from exp(-2 pi i k / N) in the bposit, recovers its input.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <complex>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>
#include <universal/number/bposit/bposit.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using sw::universal::SpecificValue;

template<typename B>
std::int64_t ordinal(const B& v) {
	const std::uint64_t r = v.raw();
	const std::uint64_t top = 1ull << (B::nbits - 1);
	return (r & top) ? static_cast<std::int64_t>(r) - static_cast<std::int64_t>(top << 1) : static_cast<std::int64_t>(r);
}

template<typename B>
bool near(const B& got, long double want) {
	const B w(want);
	if (got.isnar() || w.isnar()) return got.isnar() && w.isnar();
	const std::int64_t d = ordinal(got) - ordinal(w);
	return d >= -1 && d <= 1;
}

std::uint64_t lcg(std::uint64_t& state) {
	state = state * 6364136223846793005ull + 1442695040888963407ull;
	return state;
}

// random values of moderate magnitude, where the products stay well inside the range
template<typename B>
B moderate(std::uint64_t& state) {
	return B(static_cast<double>(static_cast<std::int64_t>(lcg(state) >> 40) - (1ll << 23)) / static_cast<double>(1 << 20));
}

template<typename B>
int VerifyArithmetic(unsigned count, bool report) {
	using C = sw::universal::complex<B>;
	int fails = 0;
	std::uint64_t state = 0x2545F4914F6CDD1Dull;
	for (unsigned k = 0; k < count; ++k) {
		const B a = moderate<B>(state), b = moderate<B>(state), c = moderate<B>(state), d = moderate<B>(state);
		const C x(a, b), y(c, d);
		const C s = x + y, m = x - y, p = x * y;
		const bool ok = s.real() == a + c && s.imag() == b + d
		             && m.real() == a - c && m.imag() == b - d
		             && p.real() == a * c - b * d && p.imag() == a * d + b * c
		             && real(x) == a && imag(x) == b && conj(x).imag() == -b;
		if (!ok) {
			++fails;
			if (report && fails < 10) std::cerr << "FAIL complex arithmetic at (" << a << ", " << b << ") and (" << c << ", " << d << ")\n";
		}
	}
	return fails;
}

template<typename B>
int VerifyFunctions(unsigned count, bool report) {
	using C = sw::universal::complex<B>;
	using L = std::complex<long double>;
	int fails = 0;
	auto expect = [&](bool ok, const char* what, const C& z) {
		if (ok) return;
		++fails;
		if (report && fails < 10) std::cerr << "FAIL complex " << what << " at (" << z.real() << ", " << z.imag() << ")\n";
	};
	std::uint64_t state = 0x9E3779B97F4A7C15ull;
	for (unsigned k = 0; k < count; ++k) {
		const C z(moderate<B>(state), moderate<B>(state));
		const L w(static_cast<long double>(z.real()), static_cast<long double>(z.imag()));
		expect(near(abs(z), std::abs(w)), "abs", z);
		expect(near(arg(z), std::arg(w)), "arg", z);
		const C e = exp(z);
		const L ew = std::exp(w);
		expect(near(e.real(), ew.real()) && near(e.imag(), ew.imag()), "exp", z);
		if (!(z.real().iszero() && z.imag().iszero())) {
			const C l = log(z);
			const L lw = std::log(w);
			expect(near(l.real(), lw.real()) && near(l.imag(), lw.imag()), "log", z);
		}
		const C r = sqrt(z);
		const L rw = std::sqrt(w);
		expect(near(r.real(), rw.real()) && near(r.imag(), rw.imag()), "sqrt", z);
	}
	// exact cases
	expect(abs(C(B(3), B(4))) == B(5), "abs(3 + 4i)", C(B(3), B(4)));
	expect(norm(C(B(3), B(4))) == B(25), "norm(3 + 4i)", C(B(3), B(4)));
	expect(exp(C(B(0), B(0))) == C(B(1), B(0)), "exp(0)", C());
	return fails;
}

// a radix-2 FFT with twiddles generated in the bposit, then its inverse: the round trip
// must recover a unit-amplitude input to within a few bposit ulps
template<typename B>
int VerifyFFTRoundTrip(unsigned N, bool report) {
	using C = sw::universal::complex<B>;
	auto fft = [](std::vector<C>& a, bool inverse) {
		const std::size_t n = a.size();
		for (std::size_t i = 1, j = 0; i < n; ++i) {
			std::size_t bit = n >> 1;
			for (; j & bit; bit >>= 1) j ^= bit;
			j ^= bit;
			if (i < j) std::swap(a[i], a[j]);
		}
		const B pi(3.14159265358979323846);
		for (std::size_t len = 2; len <= n; len <<= 1) {
			const B angle = (inverse ? B(2) : B(-2)) * pi / B(static_cast<int>(len));
			for (std::size_t i = 0; i < n; i += len) {
				for (std::size_t k = 0; k < len / 2; ++k) {
					// the twiddle from the bposit's own trigonometry
					const B theta = angle * B(static_cast<int>(k));
					const C w(cos(theta), sin(theta));
					const C u = a[i + k], v = a[i + k + len / 2] * w;
					a[i + k] = u + v;
					a[i + k + len / 2] = u - v;
				}
			}
		}
		if (inverse) for (auto& x : a) x = C(x.real() / B(static_cast<int>(n)), x.imag() / B(static_cast<int>(n)));
	};
	std::vector<C> x(N);
	for (unsigned k = 0; k < N; ++k) x[k] = C(B(std::sin(0.3 * k)), B(std::cos(0.7 * k) * 0.5));
	std::vector<C> y = x;
	fft(y, false);
	fft(y, true);
	double worst = 0;
	for (unsigned k = 0; k < N; ++k) {
		worst = std::fmax(worst, std::fabs(double(y[k].real()) - double(x[k].real())));
		worst = std::fmax(worst, std::fabs(double(y[k].imag()) - double(x[k].imag())));
	}
	// log2(N) stages each way, each adding about an ulp of the unit-magnitude data
	const double ulp = double(std::numeric_limits<B>::epsilon());
	const double bound = 2.0 * std::log2(static_cast<double>(N)) * ulp;
	if (report) std::cout << "  " << sw::universal::type_tag(B()) << " FFT(" << N << ") round trip: worst error " << worst
	                      << " (" << worst / ulp << " ulp at 1.0), bound " << bound << '\n';
	return (worst <= bound) ? 0 : 1;
}

}  // anonymous namespace

#define MANUAL_TESTING 0
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
	std::string test_suite  = "bposit complex";
	std::string test_tag    = "complex";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	using bposit16_42 = bposit<16, 4, 2, std::uint16_t>;
	using bposit32_42 = bposit<32, 4, 2, std::uint32_t>;

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyFFTRoundTrip<bposit16_42>(64, true), "bposit<16,4,2>", "FFT round trip");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<bposit16_42>(10000, reportTestCases), "bposit<16,4,2>", "complex + - *");
	nrOfFailedTestCases += ReportTestResult(VerifyFunctions<bposit16_42>(10000, reportTestCases), "bposit<16,4,2>", "complex functions");
	nrOfFailedTestCases += ReportTestResult(VerifyFFTRoundTrip<bposit16_42>(64, reportTestCases), "bposit<16,4,2>", "FFT round trip");
#endif
#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<bposit16>(10000, reportTestCases), "bposit<16,6,5>", "complex + - *");
	nrOfFailedTestCases += ReportTestResult(VerifyFunctions<bposit16>(10000, reportTestCases), "bposit<16,6,5>", "complex functions");
	nrOfFailedTestCases += ReportTestResult(VerifyFFTRoundTrip<bposit32_42>(256, reportTestCases), "bposit<32,4,2>", "FFT round trip");
#endif
#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<bposit32_42>(10000, reportTestCases), "bposit<32,4,2>", "complex + - *");
	nrOfFailedTestCases += ReportTestResult(VerifyFunctions<bposit32>(10000, reportTestCases), "bposit<32,6,5>", "complex functions");
#endif
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
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
