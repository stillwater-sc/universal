#pragma once
// scaling.hpp: scaling functions for data preprocessing
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <universal/math/mathlib_shim.hpp>  // injection of native IEEE-754 math library functions into sw::universal namespace

namespace sw { namespace universal { namespace blas {

// range returns the minimum and maximum value of the vector
template<typename Vector>
std::pair<typename Vector::value_type, typename Vector::value_type> range(const Vector& v, unsigned incx = 1) {
	using Scalar = typename Vector::value_type;
	if (v.size() == 0) return std::pair(Scalar(0), Scalar(0));
	Scalar e = v[0];
	Scalar running_min{ e }, running_max{ e };
	for (unsigned i = 1; i < v.size(); ++i) {
		e = v[i];
		if (e < running_min) running_min = e;
		if (e > running_max) running_max = e;
	}
	return std::pair(running_min, running_max);
}

// arange returns the absolute minimum and maximum value of the vector
template<typename Vector>
std::pair<typename Vector::value_type, typename Vector::value_type> arange(const Vector& v, unsigned incx = 1) {
	using Scalar = typename Vector::value_type;
	if (v.size() == 0) return std::pair(Scalar(0), Scalar(0));
	Scalar e = abs(v[0]);
	Scalar running_min{ e }, running_max{ e };
	for (unsigned i = 1; i < v.size(); ++i) {
		e = abs(v[i]);
		if (e < running_min) running_min = e;
		if (e > running_max) running_max = e;
	}
	return std::pair(running_min, running_max);
}

// data normalization

/*
    X_std = (X - X.min) / (X.max - X.min)
	X_scaled = X_std * (ub - lb) + min

	where [lb, ub] = feature_range.

	The transformation is calculated as

		X_scaled = scale * X + min - X.min * scale
		where scale = (max - min) / (X.max - X.min)
*/

// minmaxscaler rescales the elements of a vector from their original 
// range [min, max] to a new range [lb, ub]
template<typename Scalar>
blas::vector<Scalar> minmaxscaler(const blas::vector<Scalar>& v, Scalar lb = 0, Scalar ub = 1) {
	blas::vector<Scalar> t;
	if (lb >= ub) {
		std::cerr << "target range is inconsistent\n";
		return t;
	}
	std::pair< Scalar, Scalar> mm = blas::range(v);
	Scalar min = mm.first;
	Scalar max = mm.second;
	auto scale = (ub - lb) / (max - min);
	auto offset = lb - min * scale;
	std::cout << min << ", " << max << ", " << lb << ", " << ub << ", " << scale << ", " << offset << '\n';
	for (auto e : v) t.push_back(e * scale + offset);

	return t;
}

template<typename Target>
blas::vector<Target> compress(const blas::vector<double>& v) {
	auto maxpos = double(std::numeric_limits<Target>::max());

	auto vminmax = arange(v);
//	auto minValue = vminmax.first;
	auto maxValue = vminmax.second;

	sw::universal::blas::vector<Target> t(v.size());
	auto sqrtMaxpos = sqrt(maxpos);
	//std::cout << "maxValue : " << maxValue << " sqrt(maxpos) : " << sqrtMaxpos << '\n';
	double maxScale = 1.0;
	if (abs(maxValue) >= sqrtMaxpos) maxScale = sqrtMaxpos / maxValue;
	//std::cout << "scale factor      : " << maxScale << '\n';
	t = maxScale * v;
	//std::cout << "compressed vector : " << t << '\n';

	return t;
}


/*
	Standardize features by removing the mean and scaling to unit variance.

	The standard score of a sample `x` is calculated as:

		z = (x - u) / s

	where `u` is the mean of the training samples or zero if `with_mean=False`,
	and `s` is the standard deviation of the training samples or one if
	`with_std=False`.

	Centering and scaling happen independently on each feature by computing
	the relevant statistics on the samples in the training set. Mean and
	standard deviation are then stored to be used on later data using
	:meth:`transform`.

	Standardization of a dataset is a common requirement for many
	machine learning estimators: they might behave badly if the
	individual features do not more or less look like standard normally
	distributed data (e.g. Gaussian with 0 mean and unit variance).

	For instance many elements used in the objective function of
	a learning algorithm (such as the RBF kernel of Support Vector
	Machines or the L1 and L2 regularizers of linear models) assume that
	all features are centered around 0 and have variance in the same
	order. If a feature has a variance that is orders of magnitude larger
	than others, it might dominate the objective function and make the
	estimator unable to learn from other features correctly as expected.

	This scaler can also be applied to sparse CSR or CSC matrices by passing
	`with_mean=False` to avoid breaking the sparsity structure of the data.

 */

}}} // namespace sw::universal::blas
