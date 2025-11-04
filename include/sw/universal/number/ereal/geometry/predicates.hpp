#pragma once
// predicates.hpp: Exact geometric predicates using ereal adaptive-precision
//
// Based on Jonathan Richard Shewchuk's "Adaptive Precision Floating-Point
// Arithmetic and Fast Robust Geometric Predicates" (1997)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Point structures for 2D and 3D
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

	// orient2d: 2D orientation test
	// Returns: positive if c is to the left of line ab
	//          zero if a, b, c are collinear
	//          negative if c is to the right of line ab
	//
	// Computes determinant:
	// | ax  ay  1 |
	// | bx  by  1 | = (ax - cx)(by - cy) - (ay - cy)(bx - cx)
	// | cx  cy  1 |
	//
	// Note: Shewchuk's expansion arithmetic may generate up to 6 components
	// during intermediate calculations. ereal adapts precision automatically.
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> orient2d(
		const Point2D<ereal<maxlimbs>>& a,
		const Point2D<ereal<maxlimbs>>& b,
		const Point2D<ereal<maxlimbs>>& c)
	{
		using Real = ereal<maxlimbs>;

		Real acx = a.x - c.x;
		Real acy = a.y - c.y;
		Real bcx = b.x - c.x;
		Real bcy = b.y - c.y;

		return acx * bcy - acy * bcx;
	}

	// orient3d: 3D orientation test
	// Returns: positive if d is below plane abc (right-hand rule)
	//          zero if a, b, c, d are coplanar
	//          negative if d is above plane abc
	//
	// Computes determinant:
	// | ax  ay  az  1 |
	// | bx  by  bz  1 |
	// | cx  cy  cz  1 |
	// | dx  dy  dz  1 |
	//
	// Note: Shewchuk's expansion arithmetic may generate up to 16 components
	// during intermediate calculations. ereal adapts precision automatically.
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> orient3d(
		const Point3D<ereal<maxlimbs>>& a,
		const Point3D<ereal<maxlimbs>>& b,
		const Point3D<ereal<maxlimbs>>& c,
		const Point3D<ereal<maxlimbs>>& d)
	{
		using Real = ereal<maxlimbs>;

		Real adx = a.x - d.x;
		Real ady = a.y - d.y;
		Real adz = a.z - d.z;
		Real bdx = b.x - d.x;
		Real bdy = b.y - d.y;
		Real bdz = b.z - d.z;
		Real cdx = c.x - d.x;
		Real cdy = c.y - d.y;
		Real cdz = c.z - d.z;

		// Compute 3x3 determinant using rule of Sarrus
		Real bdxcdy = bdx * cdy;
		Real cdxbdy = cdx * bdy;

		Real cdxady = cdx * ady;
		Real adxcdy = adx * cdy;

		Real adxbdy = adx * bdy;
		Real bdxady = bdx * ady;

		return adz * (bdxcdy - cdxbdy)
		     + bdz * (cdxady - adxcdy)
		     + cdz * (adxbdy - bdxady);
	}

	// incircle: 2D incircle test
	// Returns: positive if d is inside circumcircle of triangle abc
	//          zero if a, b, c, d are cocircular
	//          negative if d is outside circumcircle of triangle abc
	//
	// Assumes a, b, c are in counterclockwise order
	//
	// Computes determinant:
	// | ax  ay  ax²+ay²  1 |
	// | bx  by  bx²+by²  1 |
	// | cx  cy  cx²+cy²  1 |
	// | dx  dy  dx²+dy²  1 |
	//
	// Note: More complex predicate requiring higher precision.
	// ereal adapts precision automatically to maintain accuracy.
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> incircle(
		const Point2D<ereal<maxlimbs>>& a,
		const Point2D<ereal<maxlimbs>>& b,
		const Point2D<ereal<maxlimbs>>& c,
		const Point2D<ereal<maxlimbs>>& d)
	{
		using Real = ereal<maxlimbs>;

		Real adx = a.x - d.x;
		Real ady = a.y - d.y;
		Real bdx = b.x - d.x;
		Real bdy = b.y - d.y;
		Real cdx = c.x - d.x;
		Real cdy = c.y - d.y;

		Real bdxcdy = bdx * cdy;
		Real cdxbdy = cdx * bdy;
		Real alift = adx * adx + ady * ady;

		Real cdxady = cdx * ady;
		Real adxcdy = adx * cdy;
		Real blift = bdx * bdx + bdy * bdy;

		Real adxbdy = adx * bdy;
		Real bdxady = bdx * ady;
		Real clift = cdx * cdx + cdy * cdy;

		return alift * (bdxcdy - cdxbdy)
		     + blift * (cdxady - adxcdy)
		     + clift * (adxbdy - bdxady);
	}

	// insphere: 3D insphere test
	// Returns: positive if e is inside circumsphere of tetrahedron abcd
	//          zero if a, b, c, d, e are cospherical
	//          negative if e is outside circumsphere
	//
	// Assumes a, b, c, d have positive orientation
	//
	// Computes determinant:
	// | ax  ay  az  ax²+ay²+az²  1 |
	// | bx  by  bz  bx²+by²+bz²  1 |
	// | cx  cy  cz  cx²+cy²+cz²  1 |
	// | dx  dy  dz  dx²+dy²+dz²  1 |
	// | ex  ey  ez  ex²+ey²+ez²  1 |
	//
	// Note: Most demanding geometric predicate - requires highest precision.
	// ereal adapts precision automatically. Use maxlimbs ≥ 16 for reliability.
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> insphere(
		const Point3D<ereal<maxlimbs>>& a,
		const Point3D<ereal<maxlimbs>>& b,
		const Point3D<ereal<maxlimbs>>& c,
		const Point3D<ereal<maxlimbs>>& d,
		const Point3D<ereal<maxlimbs>>& e)
	{
		using Real = ereal<maxlimbs>;

		Real aex = a.x - e.x;
		Real aey = a.y - e.y;
		Real aez = a.z - e.z;
		Real bex = b.x - e.x;
		Real bey = b.y - e.y;
		Real bez = b.z - e.z;
		Real cex = c.x - e.x;
		Real cey = c.y - e.y;
		Real cez = c.z - e.z;
		Real dex = d.x - e.x;
		Real dey = d.y - e.y;
		Real dez = d.z - e.z;

		Real ab = aex * bey - bex * aey;
		Real bc = bex * cey - cex * bey;
		Real cd = cex * dey - dex * cey;
		Real da = dex * aey - aex * dey;

		Real ac = aex * cey - cex * aey;
		Real bd = bex * dey - dex * bey;

		Real abc = aez * bc - bez * ac + cez * ab;
		Real bcd = bez * cd - cez * bd + dez * bc;
		Real cda = cez * da + dez * ac + aez * cd;
		Real dab = dez * ab + aez * bd + bez * da;

		Real alift = aex * aex + aey * aey + aez * aez;
		Real blift = bex * bex + bey * bey + bez * bez;
		Real clift = cex * cex + cey * cey + cez * cez;
		Real dlift = dex * dex + dey * dey + dez * dez;

		return (dlift * abc - clift * dab) + (blift * cda - alift * bcd);
	}

}} // namespace sw::universal
