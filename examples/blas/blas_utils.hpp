#pragma once
// blas_utils.hpp :  include file containing templated utilities to work with vectors and matrices
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <random>

// These functions print matrices and vectors in a nice format
template<typename Ty>
void printMatrix(std::ostream& ostr, const std::string& name, const std::vector<Ty>& m) {
	size_t d = size_t(std::sqrt(m.size()));
	ostr << "Matrix: " << name << " is " << d << "x" << d << std::endl;
	std::streamsize prec = ostr.precision();
	ostr << std::setprecision(17);
	for (size_t i = 0; i<d; ++i) {
		for (size_t j = 0; j<d; ++j) std::cout << std::setw(20) << m[i*d + j] << " ";
		ostr << std::endl;
	}
	ostr << std::setprecision(prec);
}

template<typename Ty>
void printVector(std::ostream& ostr, const std::string& name, const std::vector<Ty>& v) {
	size_t d = v.size();
	ostr << "Vector: " << name << " is of size " << d << " elements" << std::endl;
	std::streamsize prec = ostr.precision();
	ostr << std::setprecision(17);
	for (size_t j = 0; j<d; ++j) std::cout << std::setw(20) << v[j] << " ";
	ostr << std::setprecision(prec) << std::endl;
}

// print a vector
template<typename vector_T>
void print(std::ostream& ostr, size_t n, vector_T& x, size_t incx = 1) {
	size_t cnt, ix;
	for (cnt = 0, ix = 0; cnt < n && ix < x.size(); ++cnt, ix += incx) {
		cnt == 0 ? ostr << "[" << x[ix] : ostr << ", " << x[ix];
	}
	ostr << "]";
}

// generate a vector of random permutations around 1.0
// contraction is a right shift of the mantissa causing smaller fluctuations
template<typename element_T>
void randomVectorFillAroundOneEPS(size_t n, std::vector<element_T>& vec, size_t contraction = 6) {
	// Use random_device to generate a seed for Mersenne twister engine.
	std::random_device rd{};
	// Use Mersenne twister engine to generate pseudo-random numbers.
	std::mt19937 engine{ rd() };
	// "Filter" MT engine's output to generate pseudo-random double values,
	// **uniformly distributed** on the closed interval [0, 1].
	// (Note that the range is [inclusive, inclusive].)
	std::uniform_real_distribution<double> dist{ 0.0, 1.0 };
	// Pattern to generate pseudo-random number.
	// double rnd_value = dist(engine);

	double scale = std::pow(2, double(0.0-contraction));
	for (size_t i = 0; i < n; ++i) {
		// generate random value between [-0.5, 0.5], and contract
		double eps = (dist(engine) - 0.5) * scale;
		double v = 1.0 + eps;
		vec[i] = (element_T)v;
	}
}

// generate a vector of random permutations around 1.0
// contraction is a right shift of the mantissa causing smaller fluctuations
template<typename element_T>
void randomVectorFillAroundZeroEPS(size_t n, std::vector<element_T>& vec, size_t contraction = 6) {
	// Use random_device to generate a seed for Mersenne twister engine.
	std::random_device rd{};
	// Use Mersenne twister engine to generate pseudo-random numbers.
	std::mt19937 engine{ rd() };
	// "Filter" MT engine's output to generate pseudo-random double values,
	// **uniformly distributed** on the closed interval [0, 1].
	// (Note that the range is [inclusive, inclusive].)
	std::uniform_real_distribution<double> dist{ 0.0, 1.0 };
	// Pattern to generate pseudo-random number.
	// double rnd_value = dist(engine);

	double scale = std::pow(2, double(0.0 - contraction));
	for (size_t i = 0; i < n; ++i) {
		// generate random value between [-0.5, 0.5], and contract
		double eps = (dist(engine) - 0.5) * scale;
		double v = 0.0 + eps;
		vec[i] = (element_T)v;
	}
}

// print a sampling of the provided vector
// if samples is set to 0, all elements of the vector are printed
template<typename element_T>
void sampleVector(std::string vec_name, std::vector<element_T>& vec, uint32_t start = 0, uint32_t incr = 1, uint32_t nrSamples = 0) {
	std::cout << "Vector sample is: " << '\n';
	std::streamsize prec = std::cout.precision();
	if (nrSamples) {
		uint32_t printed = 0;
		for (uint32_t i = start; i < vec.size(); i += incr) {
			if (printed < nrSamples) {
				printed++;
				std::cout << vec_name << "[" << std::setw(3) << i << "] = " << std::setprecision(15) << vec[i] << '\n';
			}
		}
	}
	else {
		for (uint32_t i = start; i < vec.size(); i += incr) {
			std::cout << vec_name << "[" << std::setw(3) << i << "] = " << std::setprecision(15) << vec[i] << '\n';
		}
	}
	std::cout << std::setprecision(prec) << std::endl;
}