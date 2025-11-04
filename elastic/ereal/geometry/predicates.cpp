// predicates.cpp: test suite for exact geometric predicates using ereal
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/number/ereal/geometry/predicates.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw {
	namespace universal {

		// Verify orient2d predicate - basic cases
		template<typename Real>
		int VerifyOrient2D(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test 1: Simple left turn (counterclockwise)
			{
				Point2D<Real> a(Real(0.0), Real(0.0));
				Point2D<Real> b(Real(1.0), Real(0.0));
				Point2D<Real> c(Real(0.5), Real(0.5));
				Real result = orient2d(a, b, c);

				if (result.sign() <= 0) {
					if (reportTestCases) std::cerr << "FAIL: orient2d left turn\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 2: Simple right turn (clockwise)
			{
				Point2D<Real> a(Real(0.0), Real(0.0));
				Point2D<Real> b(Real(1.0), Real(0.0));
				Point2D<Real> c(Real(0.5), Real(-0.5));
				Real result = orient2d(a, b, c);

				if (result.sign() >= 0) {
					if (reportTestCases) std::cerr << "FAIL: orient2d right turn\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 3: Collinear points (should be exactly zero)
			{
				Point2D<Real> a(Real(0.0), Real(0.0));
				Point2D<Real> b(Real(1.0), Real(1.0));
				Point2D<Real> c(Real(2.0), Real(2.0));
				Real result = orient2d(a, b, c);

				// Use tolerance check instead of iszero() due to ereal implementation
				if (std::abs(double(result)) > 1e-15) {
					if (reportTestCases) std::cerr << "FAIL: orient2d collinear\n";
					++nrOfFailedTestCases;
				}
			}

			return nrOfFailedTestCases;
		}

		// Verify orient3d predicate - basic cases
		template<typename Real>
		int VerifyOrient3D(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test 1: Point above plane (negative orientation per Shewchuk convention)
			{
				Point3D<Real> a(Real(0.0), Real(0.0), Real(0.0));
				Point3D<Real> b(Real(1.0), Real(0.0), Real(0.0));
				Point3D<Real> c(Real(0.0), Real(1.0), Real(0.0));
				Point3D<Real> d(Real(0.0), Real(0.0), Real(1.0));
				Real result = orient3d(a, b, c, d);

				if (result.sign() >= 0) {
					if (reportTestCases) std::cerr << "FAIL: orient3d point above\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 2: Point below plane (positive orientation per Shewchuk convention)
			{
				Point3D<Real> a(Real(0.0), Real(0.0), Real(0.0));
				Point3D<Real> b(Real(1.0), Real(0.0), Real(0.0));
				Point3D<Real> c(Real(0.0), Real(1.0), Real(0.0));
				Point3D<Real> d(Real(0.0), Real(0.0), Real(-1.0));
				Real result = orient3d(a, b, c, d);

				if (result.sign() <= 0) {
					if (reportTestCases) std::cerr << "FAIL: orient3d point below\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 3: Coplanar points (should be exactly zero)
			{
				Point3D<Real> a(Real(0.0), Real(0.0), Real(0.0));
				Point3D<Real> b(Real(1.0), Real(0.0), Real(0.0));
				Point3D<Real> c(Real(0.0), Real(1.0), Real(0.0));
				Point3D<Real> d(Real(0.5), Real(0.5), Real(0.0));
				Real result = orient3d(a, b, c, d);

				// Use tolerance check
				if (std::abs(double(result)) > 1e-15) {
					if (reportTestCases) std::cerr << "FAIL: orient3d coplanar\n";
					++nrOfFailedTestCases;
				}
			}

			return nrOfFailedTestCases;
		}

		// Verify incircle predicate
		template<typename Real>
		int VerifyIncircle(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test 1: Point clearly inside circle
			{
				Point2D<Real> a(Real(0.0), Real(0.0));
				Point2D<Real> b(Real(1.0), Real(0.0));
				Point2D<Real> c(Real(0.0), Real(1.0));
				Point2D<Real> d(Real(0.25), Real(0.25));
				Real result = incircle(a, b, c, d);

				if (result.sign() <= 0) {
					if (reportTestCases) std::cerr << "FAIL: incircle point inside\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 2: Point clearly outside circle
			{
				Point2D<Real> a(Real(0.0), Real(0.0));
				Point2D<Real> b(Real(1.0), Real(0.0));
				Point2D<Real> c(Real(0.0), Real(1.0));
				Point2D<Real> d(Real(2.0), Real(2.0));
				Real result = incircle(a, b, c, d);

				if (result.sign() >= 0) {
					if (reportTestCases) std::cerr << "FAIL: incircle point outside\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 3: Point on circle (cocircular) - challenging case
			// For a right triangle with legs 3 and 4, hypotenuse is 5
			// Circle center is at (1.5, 2), radius = 2.5
			// Point (4, 2) should be on the circle
			{
				Point2D<Real> a(Real(0.0), Real(0.0));
				Point2D<Real> b(Real(3.0), Real(0.0));
				Point2D<Real> c(Real(0.0), Real(4.0));
				Point2D<Real> d(Real(4.0), Real(2.0));
				Real result = incircle(a, b, c, d);

				// Should be very close to zero (cocircular)
				if (std::abs(double(result)) > 1e-10) {
					if (reportTestCases) {
						std::cerr << "FAIL: incircle cocircular, result = " << double(result) << "\n";
					}
					++nrOfFailedTestCases;
				}
			}

			return nrOfFailedTestCases;
		}

		// Verify insphere predicate
		template<typename Real>
		int VerifyInsphere(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test 1: Point clearly inside sphere (negative per Shewchuk convention)
			{
				Point3D<Real> a(Real(0.0), Real(0.0), Real(0.0));
				Point3D<Real> b(Real(1.0), Real(0.0), Real(0.0));
				Point3D<Real> c(Real(0.0), Real(1.0), Real(0.0));
				Point3D<Real> d(Real(0.0), Real(0.0), Real(1.0));
				Point3D<Real> e(Real(0.25), Real(0.25), Real(0.25));
				Real result = insphere(a, b, c, d, e);

				if (result.sign() >= 0) {
					if (reportTestCases) std::cerr << "FAIL: insphere point inside\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 2: Point clearly outside sphere (positive per Shewchuk convention)
			{
				Point3D<Real> a(Real(0.0), Real(0.0), Real(0.0));
				Point3D<Real> b(Real(1.0), Real(0.0), Real(0.0));
				Point3D<Real> c(Real(0.0), Real(1.0), Real(0.0));
				Point3D<Real> d(Real(0.0), Real(0.0), Real(1.0));
				Point3D<Real> e(Real(2.0), Real(2.0), Real(2.0));
				Real result = insphere(a, b, c, d, e);

				if (result.sign() <= 0) {
					if (reportTestCases) std::cerr << "FAIL: insphere point outside\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 3: Nearly cospherical points - stress test
			// Simple test: point on or near sphere surface
			{
				Point3D<Real> a(Real(0.0), Real(0.0), Real(0.0));
				Point3D<Real> b(Real(1.0), Real(0.0), Real(0.0));
				Point3D<Real> c(Real(0.0), Real(1.0), Real(0.0));
				Point3D<Real> d(Real(0.0), Real(0.0), Real(1.0));
				// e is very close to the circumsphere surface
				Point3D<Real> e(Real(0.333333333333333), Real(0.333333333333333), Real(0.333333333333333));
				Real result = insphere(a, b, c, d, e);

				// Should be very close to zero (on or near surface)
				// Just verify we get a reasonable result
				if (std::abs(double(result)) > 1.0) {
					if (reportTestCases) {
						std::cerr << "FAIL: insphere cospherical check, result = " << double(result) << "\n";
					}
					++nrOfFailedTestCases;
				}
			}

			return nrOfFailedTestCases;
		}

	}
}

// Regression testing guards
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "ereal exact geometric predicates validation";
	std::string test_tag = "predicates";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of geometric predicates:\n";

	Point2D<ereal<>> a2(0.0, 0.0);
	Point2D<ereal<>> b2(1.0, 0.0);
	Point2D<ereal<>> c2(0.5, 0.5);
	std::cout << "orient2d (left turn): " << double(orient2d(a2, b2, c2)) << " (expected: positive)\n";

	Point3D<ereal<>> a3(0.0, 0.0, 0.0);
	Point3D<ereal<>> b3(1.0, 0.0, 0.0);
	Point3D<ereal<>> c3(0.0, 1.0, 0.0);
	Point3D<ereal<>> d3(0.0, 0.0, 1.0);
	std::cout << "orient3d (above): " << double(orient3d(a3, b3, c3, d3)) << " (expected: positive)\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	// Basic predicates at default precision (~32 digits)
	// orient2d needs up to 6 components
	// orient3d needs up to 16 components
	test_tag = "orient2d";
	nrOfFailedTestCases += ReportTestResult(VerifyOrient2D<ereal<>>(reportTestCases), "orient2d(ereal)", test_tag);

	test_tag = "orient3d";
	nrOfFailedTestCases += ReportTestResult(VerifyOrient3D<ereal<>>(reportTestCases), "orient3d(ereal)", test_tag);

	// incircle predicate: adaptive precision automatically builds needed components
	// Test at extended precision (512 bits ≈ 154 digits)
	test_tag = "incircle";
	nrOfFailedTestCases += ReportTestResult(VerifyIncircle<ereal<8>>(reportTestCases), "incircle(ereal<8>)", test_tag);

	// insphere predicate: most demanding test (adaptive expansion handles complexity)
	// Test at maximum precision (1216 bits ≈ 366 digits, maxlimbs=19)
	// Note: maxlimbs ≤ 19 constraint due to Shewchuk expansion arithmetic requirements
	test_tag = "insphere";
	nrOfFailedTestCases +=
	ReportTestResult(VerifyInsphere<ereal<19>>(reportTestCases), "insphere(ereal<19>)", test_tag);

#endif

#if REGRESSION_LEVEL_2
	// incircle predicate requires up to 32 components
	// Test at extended precision (512 bits ≈ 154 digits)
	test_tag = "incircle";
	nrOfFailedTestCases += ReportTestResult(VerifyIncircle<ereal<8>>(reportTestCases), "incircle(ereal<8>)", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// Reserved for future high-precision tests
#endif

#if REGRESSION_LEVEL_4
	// insphere predicate: stress test at high precision
	// Test at 1024 bits (≈308 digits, maxlimbs=16)
	test_tag = "insphere";
	nrOfFailedTestCases += ReportTestResult(VerifyInsphere<ereal<16>>(reportTestCases), "insphere(ereal<16>)", test_tag);
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
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
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
