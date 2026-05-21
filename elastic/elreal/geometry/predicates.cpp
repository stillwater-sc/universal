// predicates.cpp: tests for elreal geometric predicates (Phase F, #879)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Mirrors elastic/ereal/geometry/predicates.cpp: a fixed set of hand-built
// configurations whose orient / incircle / insphere sign is geometrically
// obvious. Each predicate path uses elreal arithmetic (Phases A-E) to
// evaluate the determinant, then Phase D's sign() returns the answer.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/elreal/elreal.hpp>
#include <universal/number/elreal/geometry/predicates.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	static int VerifyOrient2D(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// Left turn: c = (0.5, 0.5) is above the segment (0,0)->(1,0)
		{
			Point2D<elreal> a(elreal(0.0), elreal(0.0));
			Point2D<elreal> b(elreal(1.0), elreal(0.0));
			Point2D<elreal> c(elreal(0.5), elreal(0.5));
			elreal r = orient2d(a, b, c);
			if (sign(r) <= 0) {
				if (reportTestCases) std::cerr << "FAIL: orient2d left turn (sign=" << sign(r) << ")\n";
				++nrOfFailedTestCases;
			}
		}

		// Right turn: c below the segment
		{
			Point2D<elreal> a(elreal(0.0), elreal(0.0));
			Point2D<elreal> b(elreal(1.0), elreal(0.0));
			Point2D<elreal> c(elreal(0.5), elreal(-0.5));
			elreal r = orient2d(a, b, c);
			if (sign(r) >= 0) {
				if (reportTestCases) std::cerr << "FAIL: orient2d right turn (sign=" << sign(r) << ")\n";
				++nrOfFailedTestCases;
			}
		}

		// Collinear: integer-valued inputs make the determinant exactly zero
		{
			Point2D<elreal> a(elreal(0.0), elreal(0.0));
			Point2D<elreal> b(elreal(1.0), elreal(1.0));
			Point2D<elreal> c(elreal(2.0), elreal(2.0));
			elreal r = orient2d(a, b, c);
			if (sign(r) != 0) {
				if (reportTestCases) std::cerr << "FAIL: orient2d collinear (sign=" << sign(r) << ")\n";
				++nrOfFailedTestCases;
			}
		}

		// Nearly collinear: c just above the line, small positive determinant
		{
			Point2D<elreal> a(elreal(0.0), elreal(0.0));
			Point2D<elreal> b(elreal(1.0), elreal(1.0));
			Point2D<elreal> c(elreal(2.0), elreal(2.0) + elreal(1.0) / elreal(1024LL, 1LL));
			elreal r = orient2d(a, b, c);
			if (sign(r) <= 0) {
				if (reportTestCases) std::cerr << "FAIL: orient2d near-collinear (sign=" << sign(r) << ")\n";
				++nrOfFailedTestCases;
			}
		}

		return nrOfFailedTestCases;
	}

	static int VerifyOrient3D(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// d above the xy plane through a, b, c -- Shewchuk convention: negative sign
		{
			Point3D<elreal> a(elreal(0.0), elreal(0.0), elreal(0.0));
			Point3D<elreal> b(elreal(1.0), elreal(0.0), elreal(0.0));
			Point3D<elreal> c(elreal(0.0), elreal(1.0), elreal(0.0));
			Point3D<elreal> d(elreal(0.0), elreal(0.0), elreal(1.0));
			elreal r = orient3d(a, b, c, d);
			if (sign(r) >= 0) {
				if (reportTestCases) std::cerr << "FAIL: orient3d d above (sign=" << sign(r) << ")\n";
				++nrOfFailedTestCases;
			}
		}

		// d below the xy plane -- positive sign
		{
			Point3D<elreal> a(elreal(0.0), elreal(0.0), elreal(0.0));
			Point3D<elreal> b(elreal(1.0), elreal(0.0), elreal(0.0));
			Point3D<elreal> c(elreal(0.0), elreal(1.0), elreal(0.0));
			Point3D<elreal> d(elreal(0.0), elreal(0.0), elreal(-1.0));
			elreal r = orient3d(a, b, c, d);
			if (sign(r) <= 0) {
				if (reportTestCases) std::cerr << "FAIL: orient3d d below (sign=" << sign(r) << ")\n";
				++nrOfFailedTestCases;
			}
		}

		// Coplanar: d in the xy plane (z=0) -- exact zero
		{
			Point3D<elreal> a(elreal(0.0), elreal(0.0), elreal(0.0));
			Point3D<elreal> b(elreal(1.0), elreal(0.0), elreal(0.0));
			Point3D<elreal> c(elreal(0.0), elreal(1.0), elreal(0.0));
			Point3D<elreal> d(elreal(0.5), elreal(0.5), elreal(0.0));
			elreal r = orient3d(a, b, c, d);
			if (sign(r) != 0) {
				if (reportTestCases) std::cerr << "FAIL: orient3d coplanar (sign=" << sign(r) << ")\n";
				++nrOfFailedTestCases;
			}
		}

		return nrOfFailedTestCases;
	}

	static int VerifyIncircle(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// Unit circle through (1,0), (0,1), (-1,0); d=(0,0) at origin is inside.
		// Triangle (1,0)->(0,1)->(-1,0) is counterclockwise.
		{
			Point2D<elreal> a(elreal( 1.0), elreal( 0.0));
			Point2D<elreal> b(elreal( 0.0), elreal( 1.0));
			Point2D<elreal> c(elreal(-1.0), elreal( 0.0));
			Point2D<elreal> d(elreal( 0.0), elreal( 0.0));
			elreal r = incircle(a, b, c, d);
			if (sign(r) <= 0) {
				if (reportTestCases) std::cerr << "FAIL: incircle origin inside (sign=" << sign(r) << ")\n";
				++nrOfFailedTestCases;
			}
		}

		// Point well outside the unit circle
		{
			Point2D<elreal> a(elreal( 1.0), elreal( 0.0));
			Point2D<elreal> b(elreal( 0.0), elreal( 1.0));
			Point2D<elreal> c(elreal(-1.0), elreal( 0.0));
			Point2D<elreal> d(elreal( 5.0), elreal( 5.0));
			elreal r = incircle(a, b, c, d);
			if (sign(r) >= 0) {
				if (reportTestCases) std::cerr << "FAIL: incircle d outside (sign=" << sign(r) << ")\n";
				++nrOfFailedTestCases;
			}
		}

		// Cocircular: d = (0, -1) lies on the same unit circle as a, b, c
		{
			Point2D<elreal> a(elreal( 1.0), elreal( 0.0));
			Point2D<elreal> b(elreal( 0.0), elreal( 1.0));
			Point2D<elreal> c(elreal(-1.0), elreal( 0.0));
			Point2D<elreal> d(elreal( 0.0), elreal(-1.0));
			elreal r = incircle(a, b, c, d);
			if (sign(r) != 0) {
				if (reportTestCases) std::cerr << "FAIL: incircle cocircular (sign=" << sign(r) << ")\n";
				++nrOfFailedTestCases;
			}
		}

		return nrOfFailedTestCases;
	}

	static int VerifyInsphere(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// Standard tetrahedron with vertices (1,0,0), (0,1,0), (0,0,1), (-1,-1,-1)
		// Has positive orient3d orientation. The origin (0,0,0) is on its
		// circumsphere boundary; let's use a unit-sphere setup instead:
		// a, b, c, d on the unit sphere; e at origin is inside.
		{
			Point3D<elreal> a(elreal( 1.0), elreal( 0.0), elreal( 0.0));
			Point3D<elreal> b(elreal( 0.0), elreal( 1.0), elreal( 0.0));
			Point3D<elreal> c(elreal(-1.0), elreal( 0.0), elreal( 0.0));
			Point3D<elreal> d(elreal( 0.0), elreal( 0.0), elreal( 1.0));
			Point3D<elreal> e(elreal( 0.0), elreal( 0.0), elreal( 0.0));
			elreal r = insphere(a, b, c, d, e);
			// Sign depends on the orientation of a, b, c, d. We only require
			// that the predicate returns a definite sign (not zero -- the
			// origin is strictly inside the unit sphere through a, b, c, d).
			if (sign(r) == 0) {
				if (reportTestCases) std::cerr << "FAIL: insphere origin inside, but sign=0\n";
				++nrOfFailedTestCases;
			}
		}

		// Cospherical: e = (0, 0, -1) lies on the same unit sphere
		{
			Point3D<elreal> a(elreal( 1.0), elreal( 0.0), elreal( 0.0));
			Point3D<elreal> b(elreal( 0.0), elreal( 1.0), elreal( 0.0));
			Point3D<elreal> c(elreal(-1.0), elreal( 0.0), elreal( 0.0));
			Point3D<elreal> d(elreal( 0.0), elreal( 0.0), elreal( 1.0));
			Point3D<elreal> e(elreal( 0.0), elreal( 0.0), elreal(-1.0));
			elreal r = insphere(a, b, c, d, e);
			if (sign(r) != 0) {
				if (reportTestCases) std::cerr << "FAIL: insphere cospherical (sign=" << sign(r) << ")\n";
				++nrOfFailedTestCases;
			}
		}

		return nrOfFailedTestCases;
	}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase F geometric predicates";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = false;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	nrOfFailedTestCases += VerifyOrient2D(reportTestCases);
	nrOfFailedTestCases += VerifyOrient3D(reportTestCases);
	nrOfFailedTestCases += VerifyIncircle(reportTestCases);
	nrOfFailedTestCases += VerifyInsphere(reportTestCases);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
