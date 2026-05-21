#pragma once
// predicates.hpp: Exact geometric predicates using elreal (McCleeary lazy reals)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// =============================================================================
// Geometric predicates via lazy real arithmetic (Phase F of epic #873, #879)
// =============================================================================
//
// The canonical "showcase application" for exact-real arithmetic:
//   orient2d  -- 2D orientation (left/right/collinear)
//   orient3d  -- 3D orientation (above/below/coplanar)
//   incircle  -- 2D inside-circumcircle (inside/outside/cocircular)
//   insphere  -- 3D inside-circumsphere (inside/outside/cospherical)
//
// Each predicate reduces to the sign of a small determinant; computing that
// sign correctly for any input -- including near-degenerate configurations
// where a double-precision evaluation would be ambiguous -- is what makes
// Delaunay triangulation and other mesh-generation algorithms robust.
//
// The elreal path: evaluate the determinant with lazy-real arithmetic
// (Phases A-E), then ask Phase D's sign(...) for the answer. The refinement
// budget controls how many bits the comparison is allowed to pull when the
// determinant is small enough that the answer hinges on bits past double
// precision.
//
// Compared to the ereal (Shewchuk-style) path:
//   - ereal eagerly builds an expansion of all components needed to resolve
//     sign exactly. The expansion grows worst-case quadratically with the
//     determinant order; ereal<16> is the recommended upper end for insphere.
//   - elreal lazily refines just enough to settle sign. The refinement budget
//     bounds the worst case; the common case (general-position inputs)
//     resolves at depth 0 or 1.
//
// See docs/algorithmic-details/multi-component-arithmetic.md section 9 for
// the comparison study.
//
// References:
//   - Shewchuk, J. R. (1997). "Adaptive Precision Floating-Point Arithmetic
//     and Fast Robust Geometric Predicates." DCG 18(3), 305-363.
//   - McCleeary, R. (2019). "Lazy Exact Real Arithmetic Using Floating Point
//     Operations." Ph.D. dissertation, University of Iowa.

#include <universal/number/elreal/elreal.hpp>

namespace sw { namespace universal {

// Point types used by elreal predicates. Templated on Real so the same
// struct can be reused with elreal, ereal, dd, qd, double, ... -- whichever
// arithmetic substrate the caller picks. Mirrors the ereal predicates'
// Point2D / Point3D layout.
//
// Note: if a translation unit also includes ereal/geometry/predicates.hpp,
// these definitions collide. Each predicates header is intended to be used
// independently; a future refactor may extract Point2D / Point3D to a
// shared `include/sw/universal/geometry/point.hpp`.
#ifndef SW_UNIVERSAL_GEOMETRY_POINT_TYPES
#define SW_UNIVERSAL_GEOMETRY_POINT_TYPES

template<typename Real>
struct Point2D {
	Real x, y;
	Point2D(const Real& _x, const Real& _y) : x(_x), y(_y) {}
};

template<typename Real>
struct Point3D {
	Real x, y, z;
	Point3D(const Real& _x, const Real& _y, const Real& _z) : x(_x), y(_y), z(_z) {}
};

#endif

// orient2d: 2D orientation predicate.
// Returns the signed area of the parallelogram (a-c, b-c) times 2.
// Sign:  > 0 if c -> a -> b is counterclockwise (left turn)
//        = 0 if a, b, c are collinear
//        < 0 if c -> a -> b is clockwise (right turn)
//
//   | a.x - c.x   a.y - c.y |
//   | b.x - c.x   b.y - c.y |  =  (a.x - c.x)*(b.y - c.y) - (a.y - c.y)*(b.x - c.x)
inline elreal orient2d(
	const Point2D<elreal>& a,
	const Point2D<elreal>& b,
	const Point2D<elreal>& c)
{
	elreal acx = a.x - c.x;
	elreal acy = a.y - c.y;
	elreal bcx = b.x - c.x;
	elreal bcy = b.y - c.y;
	return acx * bcy - acy * bcx;
}

// orient3d: 3D orientation predicate.
// Sign of the 4x4 determinant
//   | a.x  a.y  a.z  1 |
//   | b.x  b.y  b.z  1 |
//   | c.x  c.y  c.z  1 |
//   | d.x  d.y  d.z  1 |
// expressed via cofactor expansion along the last row of the 3x3 matrix
// (a-d, b-d, c-d).
// Sign:  > 0 if d is below the plane through a, b, c (right-hand rule)
//        = 0 if a, b, c, d are coplanar
//        < 0 if d is above the plane.
inline elreal orient3d(
	const Point3D<elreal>& a,
	const Point3D<elreal>& b,
	const Point3D<elreal>& c,
	const Point3D<elreal>& d)
{
	elreal adx = a.x - d.x;
	elreal ady = a.y - d.y;
	elreal adz = a.z - d.z;
	elreal bdx = b.x - d.x;
	elreal bdy = b.y - d.y;
	elreal bdz = b.z - d.z;
	elreal cdx = c.x - d.x;
	elreal cdy = c.y - d.y;
	elreal cdz = c.z - d.z;

	elreal bdxcdy = bdx * cdy;
	elreal cdxbdy = cdx * bdy;

	elreal cdxady = cdx * ady;
	elreal adxcdy = adx * cdy;

	elreal adxbdy = adx * bdy;
	elreal bdxady = bdx * ady;

	return adz * (bdxcdy - cdxbdy)
	     + bdz * (cdxady - adxcdy)
	     + cdz * (adxbdy - bdxady);
}

// incircle: 2D in-circumcircle predicate.
// Assumes a, b, c are in counterclockwise order.
// Sign:  > 0 if d is strictly inside the circumcircle of triangle abc
//        = 0 if a, b, c, d are cocircular
//        < 0 if d is strictly outside.
//
// Determinant form:
//   | a.x  a.y  a.x^2 + a.y^2  1 |
//   | b.x  b.y  b.x^2 + b.y^2  1 |
//   | c.x  c.y  c.x^2 + c.y^2  1 |
//   | d.x  d.y  d.x^2 + d.y^2  1 |
inline elreal incircle(
	const Point2D<elreal>& a,
	const Point2D<elreal>& b,
	const Point2D<elreal>& c,
	const Point2D<elreal>& d)
{
	elreal adx = a.x - d.x;
	elreal ady = a.y - d.y;
	elreal bdx = b.x - d.x;
	elreal bdy = b.y - d.y;
	elreal cdx = c.x - d.x;
	elreal cdy = c.y - d.y;

	elreal bdxcdy = bdx * cdy;
	elreal cdxbdy = cdx * bdy;
	elreal alift = adx * adx + ady * ady;

	elreal cdxady = cdx * ady;
	elreal adxcdy = adx * cdy;
	elreal blift = bdx * bdx + bdy * bdy;

	elreal adxbdy = adx * bdy;
	elreal bdxady = bdx * ady;
	elreal clift = cdx * cdx + cdy * cdy;

	return alift * (bdxcdy - cdxbdy)
	     + blift * (cdxady - adxcdy)
	     + clift * (adxbdy - bdxady);
}

// insphere: 3D in-circumsphere predicate.
// Assumes a, b, c, d have positive orient3d orientation.
// Sign:  > 0 if e is strictly inside the circumsphere of tetrahedron abcd
//        = 0 if a, b, c, d, e are cospherical
//        < 0 if e is strictly outside.
//
// The most demanding of the four shipped predicates -- the determinant is
// 5x5 and an exact ereal evaluation may need 16+ limbs of expansion. The
// lazy-elreal path resolves the sign by refining only as deep as the
// configuration demands; near-cospherical inputs may consume the full
// refinement budget.
inline elreal insphere(
	const Point3D<elreal>& a,
	const Point3D<elreal>& b,
	const Point3D<elreal>& c,
	const Point3D<elreal>& d,
	const Point3D<elreal>& e)
{
	elreal aex = a.x - e.x;
	elreal aey = a.y - e.y;
	elreal aez = a.z - e.z;
	elreal bex = b.x - e.x;
	elreal bey = b.y - e.y;
	elreal bez = b.z - e.z;
	elreal cex = c.x - e.x;
	elreal cey = c.y - e.y;
	elreal cez = c.z - e.z;
	elreal dex = d.x - e.x;
	elreal dey = d.y - e.y;
	elreal dez = d.z - e.z;

	elreal ab = aex * bey - bex * aey;
	elreal bc = bex * cey - cex * bey;
	elreal cd = cex * dey - dex * cey;
	elreal da = dex * aey - aex * dey;

	elreal ac = aex * cey - cex * aey;
	elreal bd = bex * dey - dex * bey;

	elreal abc = aez * bc - bez * ac + cez * ab;
	elreal bcd = bez * cd - cez * bd + dez * bc;
	elreal cda = cez * da + dez * ac + aez * cd;
	elreal dab = dez * ab + aez * bd + bez * da;

	elreal alift = aex * aex + aey * aey + aez * aez;
	elreal blift = bex * bex + bey * bey + bez * bez;
	elreal clift = cex * cex + cey * cey + cez * cez;
	elreal dlift = dex * dex + dey * dey + dez * dez;

	return (dlift * abc - clift * dab) + (blift * cda - alift * bcd);
}

}} // namespace sw::universal
