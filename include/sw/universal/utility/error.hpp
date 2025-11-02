#pragma once
// error.hpp: utility functions to calculate relative and absolute error
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <limits>

namespace sw { namespace universal {


/// <summary>
/// Absolute error is the difference between the computed value and the reference value of a quantity.
/// </summary>
/// <typeparam name="Scalar">type representation of the value and reference</typeparam>
/// <param name="actual">measured or computed value</param>
/// <param name="reference">reference value of the quantity</param>
/// <returns>absolute error between actual and reference values</returns>
template<typename Scalar>
Scalar AbsoluteError(const Scalar& actual, const Scalar& reference) {
	return (actual - reference);
}

/// <summary>
/// The relative error is defined as the ratio of the absolute error of the computed value to the reference value.
/// Using this method we can determine the magnitude of the absolute error in terms of the reference quantity.
/// The relative error gives an indication of how good the answer is relative to the value being approximated.
/// </summary>
/// <typeparam name="ArgumentType">type representation of the value and reference</typeparam>
/// <typeparam name="ReturnType">type representation of relative error. defaults to double</typeparam>
/// <param name="actual">measured or computed value</param>
/// <param name="reference">reference value of the quantity</param>
/// <returns>relative error between actual and reference values</returns>
template<typename ArgumentType, typename ReturnType = double>
ReturnType RelativeError(const ArgumentType& actual, const ArgumentType& reference) {
	return static_cast<ReturnType>((actual - reference) / reference);
}

/// <summary>
/// The logarithmic relative error is defined as the difference of the computed value to the reference value.
/// Using this method we can determine error on a logarithmic scale.
/// The logarithmic relative error gives an indication of how good the answer is relative to the value being approximated on a log scale.
/// </summary>
/// <typeparam name="ArgumentType">type representation of the value and reference</typeparam>
/// <typeparam name="ReturnType">type representation of relative error. defaults to double</typeparam>
/// <param name="actual">measured or computed value</param>
/// <param name="reference">reference value of the quantity</param>
/// <returns>Log error between actual and reference values</returns>
template<typename ArgumentType, typename ReturnType = double>
ReturnType LogRelativeError(const ArgumentType& actual, const ArgumentType& reference) {
    using std::log10, std::abs;

    ReturnType a = static_cast<ReturnType>(abs(actual));
    ReturnType r = static_cast<ReturnType>(abs(reference)); 

    return static_cast<ReturnType>(log10(a) - log10(r));
}

/// <summary>
/// Normalize error on a logarithmic scale between a minimum and maximum positive range.
/// </summary>
/// <typeparam name="ArgumentType">type representation of the value and reference</typeparam>
/// <typeparam name="ReturnType">type representation of relative error. defaults to double</typeparam>
/// <param name="logRelativeError">measured or computed error value: needs to be in log-base10</param>
/// <returns>relative error between actual and reference values</returns>
template<typename ArgumentType, typename ReturnType = double>
ReturnType MinMaxLogNormalization(const ArgumentType& logRelativeError, const ArgumentType& maxpos, const ArgumentType& minpos){ 
    using std::log10, std::abs;
    ReturnType range = static_cast<ReturnType>(log10(maxpos) - log10(minpos));
	return static_cast<ReturnType>(abs(logRelativeError) / range);
}

/// <summary>
/// Estimate the number of valid bits in the computed value given the expected value.
/// </summary>
/// <typeparam name="Real"></typeparam>
/// <param name="computed"></param>
/// <param name="expected"></param>
/// <returns></returns>
template<typename Real>
int calculateNrOfValidBits(const Real& computed, const Real& expected) {
	constexpr double LOG2E = 1.44269504088896340736;

	Real delta = computed - expected;
	if (delta == 0.0) {
		return std::numeric_limits<Real>::digits;
	} else {
		if (expected == 0.0) {
			return static_cast<int>(-std::log(std::fabs(double(computed))) * LOG2E);
		} else {
			delta /= expected;
			double logOfDelta = std::log(std::fabs(double(delta))) * LOG2E;
			return static_cast<int>(-logOfDelta);
		}
	}
}

}} // namespace sw::universal