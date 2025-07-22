#pragma once
// linspace.hpp: linspace/logspace/geomspace implementations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <numeric/containers.hpp>
#include <blas/vmath/power.hpp>

namespace sw { namespace universal { namespace blas {

/*
 * arange generates an integer sequence between start and stop with stride
 * Use template argument to project the sequence into a target data type
 */
template<typename Scalar>
sw::universal::blas::vector<Scalar> arange(int64_t start, int64_t stop, int64_t step = 1) {
	if (start > stop) return sw::universal::blas::vector<Scalar>(0);
	sw::universal::blas::vector<Scalar> v;
	int64_t sample = start;
	while (sample <= stop) {
		v.push_back(Scalar(sample));
		sample += step;
	}
	return v;
}

/*
 * linspace: generate evenly spaced samples over a specified interval.
 * 
 * Returns `steps` evenly spaced samples, calculated over the interval [`start`, `stop`].
 * 
 * The endpoint of the interval can optionally be excluded.
*/
template<typename Scalar>
sw::universal::blas::vector<Scalar> linspace(const Scalar& start, const Scalar& stop, size_t steps, bool endpoint = true) {
	if (steps == 0) return sw::universal::blas::vector<Scalar>(0);
	if (steps == 1) return sw::universal::blas::vector<Scalar>(1) = start;
	sw::universal::blas::vector<Scalar> v(steps);
	steps = (endpoint ? steps - 1 : steps); // if endpoint is inclusive, we have one less segment
	Scalar step = (stop - start) / Scalar(steps);
	for (size_t i = 0; i < steps; ++i) {
		v[i] = Scalar(i) * step;
	}
	v += start; // this can vectorize
	if (endpoint) v[steps] = stop;
	return v;
}


/*
 * logspace: return evenly spaced samples over a log scale interval.
 * 
 * Returns `steps` evenly spaced samples, calculated over the interval [`base ^ start`, `base ^ stop`].
 * 
 * The endpoint of the interval can optionally be excluded.
*/
template<typename Scalar>
sw::universal::blas::vector<Scalar> logspace(const Scalar& start, const Scalar& stop, size_t steps, bool endpoint = true, const Scalar& base = Scalar(10.0)) {
	using std::pow;
	if (steps == 0) return sw::universal::blas::vector<Scalar>(0);
	if (steps == 1) return sw::universal::blas::vector<Scalar>(1) = pow(base, start);
	auto exponents = linspace(start, stop, steps, endpoint);
	return power(base, exponents);
}

/*
 * geomspace: return evenly spaced samples over a geometric progression.
 * 
 * Returns `steps` evenly spaced samples, calculated over the interval [`base ^ start`, `base ^ stop`].
 * 
 * The endpoint of the interval can optionally be excluded.
*/
template<typename Scalar>
sw::universal::blas::vector<Scalar> geomspace(const Scalar& start, const Scalar& stop, size_t steps, bool endpoint = true, const Scalar& base = Scalar(10.0)) {
	using std::pow;
	if (steps == 0) return sw::universal::blas::vector<Scalar>(0);
	if (steps == 1) return sw::universal::blas::vector<Scalar>(1) = pow(base, start);
	auto samples = logspace(start, stop, steps, endpoint);
	return samples;
}

/*
geomspace examples
--------
>>> np.geomspace(1, 1000, num=4)
array([    1.,    10.,   100.,  1000.])
>>> np.geomspace(1, 1000, num=3, endpoint=False)
array([   1.,   10.,  100.])
>>> np.geomspace(1, 1000, num=4, endpoint=False)
array([   1.        ,    5.62341325,   31.6227766 ,  177.827941  ])
>>> np.geomspace(1, 256, num=9)
array([   1.,    2.,    4.,    8.,   16.,   32.,   64.,  128.,  256.])

Note that the above may not produce exact integers:

>>> np.geomspace(1, 256, num=9, dtype=int)
array([  1,   2,   4,   7,  16,  32,  63, 127, 256])
>>> np.around(np.geomspace(1, 256, num=9)).astype(int)
array([  1,   2,   4,   8,  16,  32,  64, 128, 256])

Negative, decreasing, and complex inputs are allowed:

>>> np.geomspace(1000, 1, num=4)
array([1000.,  100.,   10.,    1.])
>>> np.geomspace(-1000, -1, num=4)
array([-1000.,  -100.,   -10.,    -1.])
>>> np.geomspace(1j, 1000j, num=4)  # Straight line
array([0.   +1.j, 0.  +10.j, 0. +100.j, 0.+1000.j])
>>> np.geomspace(-1+0j, 1+0j, num=5)  # Circle
array([-1.00000000e+00+1.22464680e-16j, -7.07106781e-01+7.07106781e-01j,
6.12323400e-17+1.00000000e+00j,  7.07106781e-01+7.07106781e-01j,
1.00000000e+00+0.00000000e+00j])
*/

}}} // namespace sw::universal::blas
