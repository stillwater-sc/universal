// linspace.cpp: linspace/logspace/geomspace implementations
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
// Configure the posit library with arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
//#include <universal/posit/posit>
#include <universal/blas/blas>

namespace sw { namespace unum { namespace blas {

// vector power function
template<typename Scalar1, typename Scalar2>
vector<Scalar1> power(const Scalar1& x, const vector<Scalar2>& y) {
	using std::pow;
	using namespace sw::unum;
	vector<Scalar1> v(y.size());
	for (size_t i = 0; i < y.size(); ++i) {
		v[i] = pow(x, Scalar1(y[i]));
	}
	return v;
}


template<typename Scalar>
sw::unum::blas::vector<Scalar> arange(int64_t start, int64_t stop, int64_t step = 1) {
	if (start > stop) return sw::unum::blas::vector<Scalar>(0);
	sw::unum::blas::vector<Scalar> v;
	int64_t sample = start;
	while (sample <= stop) {
		v.push_back(sample);
		sample += step;
	}
	return v;
}

/*
	linspace: return evenly spaced samples over a specified interval.

	Returns `steps` evenly spaced samples, calculated over the
	interval [`start`, `stop`].

	The endpoint of the interval can optionally be excluded.
*/
template<typename Scalar>
sw::unum::blas::vector<Scalar> linspace(const Scalar& start, const Scalar& stop, size_t steps, bool endpoint = true) {
	if (steps == 0) return sw::unum::blas::vector<Scalar>(0);
	if (steps == 1) return sw::unum::blas::vector<Scalar>(1) = start;
	sw::unum::blas::vector<Scalar> v(steps);
	steps = (endpoint ? steps - 1 : steps); // if endpoint is inclusive, we have one less segment
	Scalar step = (stop - start) / steps;
	Scalar x = start;
	for (size_t i = 0; i < steps; ++i) {
		v[i] = i * step;
	}
	v += start; // this can vectorize
	if (endpoint) v[steps] = stop;
	return v;
}


/*
	logspace: return evenly spaced samples over a log scale interval.

	Returns `steps` evenly spaced samples, calculated over the
	interval [`base ^ start`, `base ^ stop`].

	The endpoint of the interval can optionally be excluded.
*/
template<typename Scalar>
sw::unum::blas::vector<Scalar> logspace(const Scalar& start, const Scalar& stop, size_t steps, bool endpoint = true, const Scalar& base = Scalar(10.0)) {
	using std::pow;
	if (steps == 0) return sw::unum::blas::vector<Scalar>(0);
	if (steps == 1) return sw::unum::blas::vector<Scalar>(1) = pow(base, start);
	auto exponents = linspace(start, stop, steps, endpoint);
	return power(base, exponents);
}

/*
	geomspace: return evenly spaced samples over a geometric progression.

	Returns `steps` evenly spaced samples, calculated over the
	interval [`base ^ start`, `base ^ stop`].

	The endpoint of the interval can optionally be excluded.
		Examples
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
template<typename Scalar>
sw::unum::blas::vector<Scalar> geomspace(const Scalar& start, const Scalar& stop, size_t steps, bool endpoint = true, const Scalar& base = Scalar(10.0)) {
	using std::pow;
	if (steps == 0) return sw::unum::blas::vector<Scalar>(0);
	if (steps == 1) return sw::unum::blas::vector<Scalar>(1) = pow(base, start);
	auto samples = logspace(start, stop, steps, endpoint);
	return samples;
}

} } }  // namespace sw::unum::blas
