#pragma once
// error.hpp: utility functions to calculate relative and absolute error
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

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
	return (ReturnType(actual) - ReturnType(reference)) / ReturnType(reference);
}

}} // namespace sw::universal

