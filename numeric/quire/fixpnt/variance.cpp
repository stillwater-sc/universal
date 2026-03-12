// variance.cpp: Fixed-point variance computation - quire for exact statistics
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// ============================================================================
// Fixed-Point Variance: Product Truncation Bias in Statistics
// ============================================================================
//
// Computing variance in fixed-point arithmetic involves accumulating
// squared deviations:
//
//   var(x) = (1/N) · sum( (x_i − mean)² )
//
// Each (x_i − mean)² is a product of two fixpnt values.  Multiplying
// two fixpnt<N,R> values produces a 2N-bit result with 2R fractional
// bits.  Truncating back to N bits discards the lower R fractional bits
// of EVERY squared deviation.  Over N terms, these truncation errors
// accumulate as a systematic bias that distorts the computed variance.
//
// The quire (FDP) eliminates this bias: it holds every squared deviation
// at full precision and accumulates without rounding, producing a single
// correctly-rounded result.
//
// ============================================================================
// Implementation Strategy
// ============================================================================
//
// We use the CENTERED formula:  var = fdp(d, d) / N  where d_i = x_i − mean
//
// This avoids the numerically unstable E[x²] − E[x]² formula (which
// suffers catastrophic cancellation when mean >> stddev) AND avoids
// overflow in the accumulator (since deviations are small).
//
// The FDP benefit comes from exact accumulation of the d_i² products.
// Naive computation truncates each d_i² before summing; FDP does not.
//
// ============================================================================
// References
// ============================================================================
//
// [1] Welford, B. P. (1962). "Note on a Method for Calculating Corrected
//     Sums of Squares and Products." Technometrics, 4(3), 419-420.
//
// [2] Chan, T. F., Golub, G. H., and LeVeque, R. J. (1983). "Algorithms
//     for Computing the Sample Variance: Analysis and Recommendations."
//     The American Statistician, 37(3), 242-247.
//
// [3] Kulisch, U. W. (2013). "Computer Arithmetic and Validity."
//     - The super-accumulator eliminates truncation error in the
//     accumulation of products, making the centered formula exact.
//
// ============================================================================

#include <universal/utility/directives.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/fixpnt/fdp.hpp>
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_reporters.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <random>

// ============================================================================
// Variance: naive centered formula
//
// var = (1/N) * sum( (x_i - mean)² )
//
// Each (x_i - mean)² is truncated to fixpnt before accumulation.
// ============================================================================
template<typename Scalar>
double variance_naive(const std::vector<Scalar>& data) {
	size_t N = data.size();
	if (N == 0) return 0.0;

	// Compute mean in double (to isolate the accumulation error)
	double sum = 0;
	for (size_t i = 0; i < N; ++i) sum += double(data[i]);
	double mean_d = sum / double(N);
	Scalar mean_fp = Scalar(mean_d);

	// Compute deviations and sum of squared deviations naively
	Scalar sum_d2(0);
	for (size_t i = 0; i < N; ++i) {
		Scalar d = data[i] - mean_fp;
		sum_d2 = sum_d2 + d * d;  // d*d truncated before accumulation
	}

	return double(sum_d2) / double(N);
}

// ============================================================================
// Variance: FDP centered formula
//
// var = fdp(deviations, deviations) / N
//
// The quire accumulates all d_i² products at full precision.
// ============================================================================
template<typename Scalar>
double variance_fdp(const std::vector<Scalar>& data) {
	size_t N = data.size();
	if (N == 0) return 0.0;

	// Compute mean in double
	double sum = 0;
	for (size_t i = 0; i < N; ++i) sum += double(data[i]);
	double mean_d = sum / double(N);
	Scalar mean_fp = Scalar(mean_d);

	// Build deviation vector
	std::vector<Scalar> d(N);
	for (size_t i = 0; i < N; ++i)
		d[i] = data[i] - mean_fp;

	// FDP: sum(d²) = fdp(d, d)
	Scalar sum_d2 = sw::universal::fdp(d, d);

	return double(sum_d2) / double(N);
}

// ============================================================================
// Reference variance in double
// ============================================================================
template<typename Scalar>
double variance_reference(const std::vector<Scalar>& data) {
	size_t N = data.size();
	if (N == 0) return 0.0;

	double sum = 0;
	for (size_t i = 0; i < N; ++i) sum += double(data[i]);
	double mean = sum / double(N);

	double var = 0;
	for (size_t i = 0; i < N; ++i) {
		double d = double(data[i]) - mean;
		var += d * d;
	}
	return var / double(N);
}

// ============================================================================
// Case 1: Small spread near zero - truncation bias in d²
//
// Values cluster near zero with tiny spread.  Each d² is small
// (< 1 ULP of fixpnt) but the SUM is significant.  Naive truncation
// rounds each d² to zero; FDP preserves the sub-ULP contributions.
// ============================================================================
template<typename Scalar>
void Case1_SmallSpread() {
	std::cout << "\n  Case 1: Small spread (values near 0, spread ≈ few ULPs)\n";

	// fixpnt<16,8> ULP = 1/256 ≈ 0.004
	// Values: 0, ±1/256, ±2/256, ±3/256, ±4/256
	std::vector<Scalar> data;
	for (int k = -4; k <= 4; ++k) {
		data.push_back(Scalar(k / 256.0));
	}

	double ref   = variance_reference(data);
	double naive = variance_naive(data);
	double fdp   = variance_fdp(data);

	std::cout << "    Data: 9 values spanning ±4 ULPs around zero\n";
	std::cout << std::setprecision(10);
	std::cout << "    Reference (double):  " << ref << '\n';
	std::cout << "    Naive (truncated):   " << naive
	          << "  (error: " << std::abs(naive - ref) << ")\n";
	std::cout << "    FDP (exact accum):   " << fdp
	          << "  (error: " << std::abs(fdp - ref) << ")\n";
}

// ============================================================================
// Case 2: Moderate data near 1.0 - practical fixed-point range
//
// Values near 1.0 with spread ~0.5.  The deviations are moderate;
// each d² has ~16 significant bits but only 8 survive truncation.
// Over 100 terms the bias accumulates.
// ============================================================================
template<typename Scalar>
void Case2_ModerateData() {
	constexpr size_t N = 100;
	std::vector<Scalar> data(N);

	// Deterministic: x[i] = sin(i) * 0.5  (range [-0.5, +0.5], mean ≈ 0)
	for (size_t i = 0; i < N; ++i)
		data[i] = Scalar(std::sin(double(i)) * 0.5);

	double ref   = variance_reference(data);
	double naive = variance_naive(data);
	double fdp   = variance_fdp(data);

	std::cout << "\n  Case 2: Moderate data (sin(i)*0.5, N=" << N << ")\n";
	std::cout << std::setprecision(10);
	std::cout << "    Reference (double):  " << ref << '\n';
	std::cout << "    Naive (truncated):   " << naive
	          << "  (error: " << std::abs(naive - ref) << ")\n";
	std::cout << "    FDP (exact accum):   " << fdp
	          << "  (error: " << std::abs(fdp - ref) << ")\n";
	if (ref > 0) {
		std::cout << "    Naive relative err:  " << std::abs(naive - ref) / ref * 100 << "%\n";
		std::cout << "    FDP   relative err:  " << std::abs(fdp - ref) / ref * 100 << "%\n";
	}
}

// ============================================================================
// Case 3: Sensor noise - temperature fluctuations
//
// A temperature sensor reports values near 2.0°C (kept small to avoid
// overflow in fixpnt<16,8>) with millidegree noise.  The variance
// captures the noise floor.  Product truncation can bury the signal.
// ============================================================================
template<typename Scalar>
void Case3_SensorNoise() {
	constexpr size_t N = 200;
	std::vector<Scalar> data(N);

	// T = 2.0 + noise, noise ∈ {-3, -2, -1, 0, +1, +2, +3} ULPs
	std::mt19937 rng(42);
	std::uniform_int_distribution<int> noise(-3, 3);
	for (size_t i = 0; i < N; ++i)
		data[i] = Scalar(2.0 + noise(rng) / 256.0);

	double ref   = variance_reference(data);
	double naive = variance_naive(data);
	double fdp   = variance_fdp(data);

	std::cout << "\n  Case 3: Sensor noise (T ≈ 2.0, noise ±3 ULP, N=" << N << ")\n";
	std::cout << std::setprecision(10);
	std::cout << "    Expected variance:   ~" << (3.0*3.0 + 2.0*2.0 + 1.0) / (3.0 * 256.0 * 256.0) << " (approx)\n";
	std::cout << "    Reference (double):  " << ref << '\n';
	std::cout << "    Naive (truncated):   " << naive
	          << "  (error: " << std::abs(naive - ref) << ")\n";
	std::cout << "    FDP (exact accum):   " << fdp
	          << "  (error: " << std::abs(fdp - ref) << ")\n";
	if (ref > 0) {
		std::cout << "    Naive relative err:  " << std::abs(naive - ref) / ref * 100 << "%\n";
		std::cout << "    FDP   relative err:  " << std::abs(fdp - ref) / ref * 100 << "%\n";
	}
}

// ============================================================================
// Case 4: Accuracy across fixpnt widths
//
// Same dataset, different bit widths.  Wider types have more fractional
// bits, reducing truncation error.  FDP should always be at least as
// accurate as naive.
// ============================================================================
template<typename Scalar>
void RunVarianceTest(const std::string& type_name) {
	constexpr size_t N = 100;
	std::vector<Scalar> data(N);
	for (size_t i = 0; i < N; ++i)
		data[i] = Scalar(std::sin(double(i)) * 0.5);

	double ref     = variance_reference(data);
	double naive   = variance_naive(data);
	double fdp_var = variance_fdp(data);

	std::cout << "    " << std::left << std::setw(24) << type_name
	          << std::right
	          << std::setw(16) << std::scientific << std::setprecision(4) << ref
	          << std::setw(16) << std::abs(naive - ref)
	          << std::setw(16) << std::abs(fdp_var - ref)
	          << '\n';
}

// Regression testing guards
#ifndef MANUAL_TESTING
#define MANUAL_TESTING 1
#endif
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

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "Fixed-point variance - quire FDP";
	std::string test_tag   = "fixpnt_variance";
	bool reportTestCases   = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	std::cout << "============================================================\n";
	std::cout << "Fixed-Point Variance: Truncation Bias in Statistics\n";
	std::cout << "============================================================\n";
	std::cout << "\nVariance = (1/N) · sum( (x_i - mean)² ).  Each squared\n";
	std::cout << "deviation is truncated in naive fixpnt arithmetic.  The\n";
	std::cout << "quire (FDP) accumulates the exact squared deviations,\n";
	std::cout << "eliminating the systematic truncation bias.\n";

	using Fp = fixpnt<16, 8, Modulo, uint16_t>;

	Case1_SmallSpread<Fp>();
	Case2_ModerateData<Fp>();
	Case3_SensorNoise<Fp>();

	std::cout << "\n  Case 4: Accuracy across fixpnt widths (sin(i)*0.5, N=100)\n\n";
	std::cout << "    " << std::left << std::setw(24) << "Type"
	          << std::right << std::setw(16) << "True variance"
	          << std::setw(16) << "Naive error"
	          << std::setw(16) << "FDP error"
	          << '\n';
	std::cout << "    " << std::string(72, '-') << '\n';

	RunVarianceTest<fixpnt<12, 4, Modulo, uint16_t>>("fixpnt<12,4>");
	RunVarianceTest<fixpnt<16, 8, Modulo, uint16_t>>("fixpnt<16,8>");
	RunVarianceTest<fixpnt<24, 12, Modulo, uint32_t>>("fixpnt<24,12>");
	RunVarianceTest<fixpnt<32, 16, Modulo, uint32_t>>("fixpnt<32,16>");

	std::cout << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Verify FDP variance is closer to reference than naive
	{
		using Fp = fixpnt<16, 8, Modulo, uint16_t>;
		constexpr size_t N = 100;
		std::vector<Fp> data(N);
		for (size_t i = 0; i < N; ++i)
			data[i] = Fp(std::sin(double(i)) * 0.5);

		double ref = variance_reference(data);
		double naive = variance_naive(data);
		double fdp_var = variance_fdp(data);

		if (std::abs(fdp_var - ref) > std::abs(naive - ref) + 1e-10) {
			++nrOfFailedTestCases;
			std::cerr << "FAIL: FDP variance less accurate than naive\n";
		}
	}
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << '\n';
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
