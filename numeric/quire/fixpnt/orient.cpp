// orient.cpp: Computational geometry with exact fixpnt FDP — polygon area
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// ============================================================================
// Polygon Area and Geometric Predicates via Fixed-Point FDP
// ============================================================================
//
// The shoelace formula computes the signed area of a simple polygon from
// its vertex coordinates:
//
//   2·Area = sum_{i=0}^{N-1} (x_i · y_{i+1} − x_{i+1} · y_i)
//
// This is a dot product of length 2N: each vertex contributes one positive
// and one negative cross-product term.  The sum involves massive
// cancellation — the positive and negative trapezoid areas nearly cancel,
// leaving only the polygon's area as the residual.
//
// In fixed-point arithmetic, each product x_i * y_j is truncated before
// accumulation.  Over many vertices, the truncation errors accumulate
// as a systematic bias that distorts the computed area.  The quire (FDP)
// holds every product at full precision and accumulates without rounding,
// eliminating this bias.
//
// This example also demonstrates TRANSLATION INVARIANCE: the area of a
// polygon should be independent of where it is positioned.  With naive
// fixpnt arithmetic, translating a polygon far from the origin increases
// the product magnitudes, causing more severe modular wrap and truncation.
// FDP maintains accuracy regardless of translation.
//
// ============================================================================
// References
// ============================================================================
//
// [1] Shewchuk, J. R. (1997). "Adaptive Precision Floating-Point
//     Arithmetic and Fast Robust Geometric Predicates." Discrete &
//     Computational Geometry, 18(3), 305-363.
//
// [2] Kettner, L., Mehlhorn, K., Pion, S., Schirra, S., and Yap, C.
//     (2008). "Classroom Examples of Robustness Problems in Geometric
//     Computations." Computational Geometry, 40(1), 61-78.
//
// [3] Braden, B. (1986). "The Surveyor's Area Formula."
//     The College Mathematics Journal, 17(4), 326-337.
//     — The shoelace formula and its history.
//
// [4] Kulisch, U. W. (2013). "Computer Arithmetic and Validity."
//     — The super-accumulator applied to exact geometric computation.
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
#include <string>

// ============================================================================
// 2D point
// ============================================================================
template<typename Scalar>
struct Point2D {
	Scalar x, y;
};

// ============================================================================
// Shoelace area — naive fixed-point
//
// 2*Area = sum(x_i * y_{i+1} - x_{i+1} * y_i)
// Each product is truncated before accumulation.
// ============================================================================
template<typename Scalar>
double shoelace_naive(const std::vector<Point2D<Scalar>>& poly) {
	size_t N = poly.size();
	Scalar sum(0);
	for (size_t i = 0; i < N; ++i) {
		size_t j = (i + 1) % N;
		sum = sum + poly[i].x * poly[j].y - poly[j].x * poly[i].y;
	}
	return double(sum) / 2.0;
}

// ============================================================================
// Shoelace area — FDP via quire
//
// Pack the cross-product terms as a dot product:
//   a = [x0, -x1, x1, -x2, ..., x_{N-1}, -x0]
//   b = [y1,  y0, y2,  y1, ..., y0,       y_{N-1}]
//
// so fdp(a, b) = sum(x_i * y_{i+1} - x_{i+1} * y_i) = 2*Area
// ============================================================================
template<typename Scalar>
double shoelace_fdp(const std::vector<Point2D<Scalar>>& poly) {
	size_t N = poly.size();
	std::vector<Scalar> a(2 * N), b(2 * N);
	for (size_t i = 0; i < N; ++i) {
		size_t j = (i + 1) % N;
		a[2 * i]     =  poly[i].x;   b[2 * i]     = poly[j].y;
		a[2 * i + 1] = -poly[j].x;   b[2 * i + 1] = poly[i].y;
	}
	Scalar result = sw::universal::fdp(a, b);
	return double(result) / 2.0;
}

// ============================================================================
// Reference area in double
// ============================================================================
template<typename Scalar>
double shoelace_double(const std::vector<Point2D<Scalar>>& poly) {
	size_t N = poly.size();
	double sum = 0;
	for (size_t i = 0; i < N; ++i) {
		size_t j = (i + 1) % N;
		sum += double(poly[i].x) * double(poly[j].y)
		     - double(poly[j].x) * double(poly[i].y);
	}
	return sum / 2.0;
}

// Translate polygon
template<typename Scalar>
std::vector<Point2D<Scalar>> translate(const std::vector<Point2D<Scalar>>& poly,
                                        Scalar tx, Scalar ty) {
	std::vector<Point2D<Scalar>> result(poly.size());
	for (size_t i = 0; i < poly.size(); ++i) {
		result[i].x = poly[i].x + tx;
		result[i].y = poly[i].y + ty;
	}
	return result;
}

// ============================================================================
// Case 1: Known-area polygons — triangle and rectangle
//
// Simple shapes with exact area, compared at the origin.
// ============================================================================
template<typename Scalar>
void Case1_KnownArea() {
	std::cout << "\n  Case 1: Known-area polygons\n\n";
	std::cout << "    " << std::left << std::setw(24) << "Polygon"
	          << std::setw(12) << "Exact"
	          << std::setw(14) << "Naive"
	          << std::setw(14) << "FDP"
	          << std::setw(14) << "Naive err"
	          << std::setw(14) << "FDP err"
	          << '\n';
	std::cout << "    " << std::string(92, '-') << '\n';

	// Right triangle: (0,0), (8,0), (0,6) — area = 24
	{
		std::vector<Point2D<Scalar>> tri = {
			{Scalar(0), Scalar(0)}, {Scalar(8), Scalar(0)}, {Scalar(0), Scalar(6)}
		};
		double ref   = 24.0;
		double naive = shoelace_naive(tri);
		double fdp   = shoelace_fdp(tri);

		std::cout << "    " << std::left << std::setw(24) << "Triangle (8x6)"
		          << std::setw(12) << std::fixed << std::setprecision(4) << ref
		          << std::setw(14) << naive
		          << std::setw(14) << fdp
		          << std::setw(14) << std::abs(naive - ref)
		          << std::setw(14) << std::abs(fdp - ref)
		          << '\n';
	}

	// Rectangle: (1,1), (7,1), (7,5), (1,5) — area = 24
	{
		std::vector<Point2D<Scalar>> rect = {
			{Scalar(1), Scalar(1)}, {Scalar(7), Scalar(1)},
			{Scalar(7), Scalar(5)}, {Scalar(1), Scalar(5)}
		};
		double ref   = 24.0;
		double naive = shoelace_naive(rect);
		double fdp   = shoelace_fdp(rect);

		std::cout << "    " << std::left << std::setw(24) << "Rectangle (6x4)"
		          << std::setw(12) << ref
		          << std::setw(14) << naive
		          << std::setw(14) << fdp
		          << std::setw(14) << std::abs(naive - ref)
		          << std::setw(14) << std::abs(fdp - ref)
		          << '\n';
	}
}

// ============================================================================
// Case 2: Regular polygon with many vertices
//
// A regular N-gon centered at the origin.  As N grows, the shoelace
// sum has more terms and more cancellation, amplifying truncation bias.
// Radius chosen to keep products within fixpnt range.
// ============================================================================
template<typename Scalar>
void Case2_RegularPolygon() {
	std::cout << "\n  Case 2: Regular polygons centered at origin (radius=4)\n";
	std::cout << "    Area = π·r² ≈ 50.27 as N → ∞\n\n";
	std::cout << "    " << std::left << std::setw(8) << "N"
	          << std::setw(14) << "Ref (double)"
	          << std::setw(14) << "Naive"
	          << std::setw(14) << "FDP"
	          << std::setw(14) << "Naive err"
	          << std::setw(14) << "FDP err"
	          << '\n';
	std::cout << "    " << std::string(78, '-') << '\n';

	double radius = 4.0;
	for (int N : {6, 12, 24, 48, 96}) {
		std::vector<Point2D<Scalar>> poly(N);
		for (int i = 0; i < N; ++i) {
			double angle = 2.0 * M_PI * i / N;
			poly[i].x = Scalar(radius * std::cos(angle));
			poly[i].y = Scalar(radius * std::sin(angle));
		}

		double ref   = shoelace_double(poly);
		double naive = shoelace_naive(poly);
		double fdp   = shoelace_fdp(poly);

		std::cout << "    " << std::left << std::setw(8) << N
		          << std::setw(14) << std::fixed << std::setprecision(6) << ref
		          << std::setw(14) << naive
		          << std::setw(14) << fdp
		          << std::setw(14) << std::setprecision(6) << std::abs(naive - ref)
		          << std::setw(14) << std::abs(fdp - ref)
		          << '\n';
	}
}

// ============================================================================
// Case 3: Translation invariance
//
// The area of a polygon should not depend on where it is positioned.
// With naive fixpnt arithmetic, translating the polygon far from the
// origin increases the magnitude of the cross-product terms, causing
// more truncation error (and potential modular wrap in narrow types).
//
// The FDP accumulates full-precision products, giving more consistent
// area across translations.
// ============================================================================
template<typename Scalar>
void Case3_TranslationInvariance() {
	std::cout << "\n  Case 3: Translation invariance (regular 12-gon, radius=4)\n";
	std::cout << "    The area should be constant regardless of translation.\n\n";

	// Base polygon at origin
	constexpr int N = 12;
	double radius = 4.0;
	std::vector<Point2D<Scalar>> poly(N);
	for (int i = 0; i < N; ++i) {
		double angle = 2.0 * M_PI * i / N;
		poly[i].x = Scalar(radius * std::cos(angle));
		poly[i].y = Scalar(radius * std::sin(angle));
	}

	double base_ref   = shoelace_double(poly);
	double base_naive = shoelace_naive(poly);
	double base_fdp   = shoelace_fdp(poly);

	std::cout << "    " << std::left << std::setw(20) << "Translation"
	          << std::setw(14) << "Ref (double)"
	          << std::setw(14) << "Naive"
	          << std::setw(14) << "FDP"
	          << std::setw(14) << "Naive drift"
	          << std::setw(14) << "FDP drift"
	          << '\n';
	std::cout << "    " << std::string(90, '-') << '\n';

	std::cout << "    " << std::left << std::setw(20) << "(0, 0)"
	          << std::setw(14) << std::fixed << std::setprecision(4) << base_ref
	          << std::setw(14) << base_naive
	          << std::setw(14) << base_fdp
	          << std::setw(14) << "—"
	          << std::setw(14) << "—"
	          << '\n';

	// Test translations — products grow as (r+t)² but modular wrap
	// preserves the area difference for moderate translations.
	double translations[] = { 5.0, 10.0, 20.0, 40.0 };
	for (double t : translations) {
		auto shifted = translate(poly, Scalar(t), Scalar(t));
		double ref   = shoelace_double(shifted);
		double naive = shoelace_naive(shifted);
		double fdp   = shoelace_fdp(shifted);

		std::cout << "    " << std::left << std::setw(20) << ("(" + std::to_string(int(t)) + ", " + std::to_string(int(t)) + ")")
		          << std::setw(14) << std::fixed << std::setprecision(4) << ref
		          << std::setw(14) << naive
		          << std::setw(14) << fdp
		          << std::setw(14) << std::setprecision(4) << std::abs(naive - base_naive)
		          << std::setw(14) << std::abs(fdp - base_fdp)
		          << '\n';
	}
}

// ============================================================================
// Case 4: Accuracy across fixpnt widths
// ============================================================================
template<typename Scalar>
void RunAreaTest(const std::string& type_name) {
	constexpr int N = 48;
	double radius = 4.0;
	std::vector<Point2D<Scalar>> poly(N);
	for (int i = 0; i < N; ++i) {
		double angle = 2.0 * M_PI * i / N;
		poly[i].x = Scalar(radius * std::cos(angle));
		poly[i].y = Scalar(radius * std::sin(angle));
	}

	double ref   = shoelace_double(poly);
	double naive = shoelace_naive(poly);
	double fdp   = shoelace_fdp(poly);

	std::cout << "    " << std::left << std::setw(24) << type_name
	          << std::right
	          << std::setw(14) << std::fixed << std::setprecision(6) << ref
	          << std::setw(14) << std::abs(naive - ref)
	          << std::setw(14) << std::abs(fdp - ref)
	          << '\n';
}

// Regression testing guards
#define MANUAL_TESTING 1
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

	std::string test_suite = "Polygon area — fixpnt FDP";
	std::string test_tag   = "orient";
	bool reportTestCases   = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	std::cout << "============================================================\n";
	std::cout << "Polygon Area via Shoelace Formula — Fixed-Point FDP\n";
	std::cout << "============================================================\n";
	std::cout << "\nThe shoelace formula computes polygon area as a sum of cross\n";
	std::cout << "products — a dot product with massive cancellation.  In fixed-\n";
	std::cout << "point, each product is truncated before accumulation, creating\n";
	std::cout << "a systematic bias that grows with the number of vertices.\n";
	std::cout << "The quire (FDP) eliminates this bias.\n";

	using Fp = fixpnt<16, 8, Modulo, uint16_t>;

	Case1_KnownArea<Fp>();
	Case2_RegularPolygon<Fp>();
	Case3_TranslationInvariance<Fp>();

	std::cout << "\n  Case 4: Area accuracy across fixpnt widths (48-gon, radius=4)\n\n";
	std::cout << "    " << std::left << std::setw(24) << "Type"
	          << std::right << std::setw(14) << "Ref area"
	          << std::setw(14) << "Naive error"
	          << std::setw(14) << "FDP error"
	          << '\n';
	std::cout << "    " << std::string(66, '-') << '\n';

	RunAreaTest<fixpnt<12, 4, Modulo, uint16_t>>("fixpnt<12,4>");
	RunAreaTest<fixpnt<16, 8, Modulo, uint16_t>>("fixpnt<16,8>");
	RunAreaTest<fixpnt<24, 12, Modulo, uint32_t>>("fixpnt<24,12>");
	RunAreaTest<fixpnt<32, 16, Modulo, uint32_t>>("fixpnt<32,16>");

	std::cout << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Verify FDP area of a unit square
	{
		using Fp = fixpnt<16, 8, Modulo, uint16_t>;
		std::vector<Point2D<Fp>> sq = {
			{Fp(0), Fp(0)}, {Fp(1), Fp(0)}, {Fp(1), Fp(1)}, {Fp(0), Fp(1)}
		};
		double fdp_area = shoelace_fdp(sq);
		if (std::abs(fdp_area - 1.0) > 0.01) {
			++nrOfFailedTestCases;
			std::cerr << "FAIL: FDP unit square area = " << fdp_area << ", expected 1.0\n";
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
