// trigonometric.hpp: trigonometric functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <math/constants/double_constants.hpp>  // for d_pi_2

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> sin(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::sin(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> cos(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::cos(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> tan(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::tan(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> atan(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::atan(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> atan2(const takum<nbits, rbits, bt>& y, const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::atan2(double(y), double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> asin(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::asin(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> acos(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::acos(double(x)));
}

// cot(x) = tan(pi/2 - x)
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> cot(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::tan(sw::universal::d_pi_2 - double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> sec(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(1.0 / std::cos(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> csc(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(1.0 / std::sin(double(x)));
}

// ---------------------------------------------------------------------------
// takum_log: the circular functions have no logarithmic-domain form; they are
// evaluated as reals.  Kept here so takum_log is a complete plug-in type.
// ---------------------------------------------------------------------------
template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> sin(const takum_log<nbits, rbits, bt>& x) {
	return takum_log<nbits, rbits, bt>(std::sin(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> cos(const takum_log<nbits, rbits, bt>& x) {
	return takum_log<nbits, rbits, bt>(std::cos(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> tan(const takum_log<nbits, rbits, bt>& x) {
	return takum_log<nbits, rbits, bt>(std::tan(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> asin(const takum_log<nbits, rbits, bt>& x) {
	return takum_log<nbits, rbits, bt>(std::asin(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> acos(const takum_log<nbits, rbits, bt>& x) {
	return takum_log<nbits, rbits, bt>(std::acos(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> atan(const takum_log<nbits, rbits, bt>& x) {
	return takum_log<nbits, rbits, bt>(std::atan(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> atan2(const takum_log<nbits, rbits, bt>& y,
                                         const takum_log<nbits, rbits, bt>& x) {
	return takum_log<nbits, rbits, bt>(std::atan2(double(y), double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> sec(const takum_log<nbits, rbits, bt>& x) {
	return takum_log<nbits, rbits, bt>(1.0 / std::cos(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> csc(const takum_log<nbits, rbits, bt>& x) {
	return takum_log<nbits, rbits, bt>(1.0 / std::sin(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> cot(const takum_log<nbits, rbits, bt>& x) {
	return takum_log<nbits, rbits, bt>(1.0 / std::tan(double(x)));
}

}} // namespace sw::universal
