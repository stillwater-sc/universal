#pragma once
// blas_utils.hpp :  include file containing templated utilities to work with vectors and matrices
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace blas {

		// These functions print matrices and vectors in a nice format
		template<typename Ty>
		void printMatrix(std::ostream& ostr, const std::string& name, const std::vector<Ty>& m) {
			size_t d = size_t(std::sqrt(m.size()));
			ostr << "Matrix: " << name << " is " << d << "x" << d << std::endl;
			ostr << std::setprecision(17);
			for (size_t i = 0; i<d; ++i) {
				for (size_t j = 0; j<d; ++j) std::cout << std::setw(20) << m[i*d + j] << " ";
				ostr << std::endl;
			}
			ostr << std::setprecision(5);
		}

		template<typename Ty>
		void printVector(std::ostream& ostr, const std::string& name, const std::vector<Ty>& v) {
			size_t d = v.size();
			ostr << "Vector: " << name << " is of size " << d << " elements" << std::endl;
			ostr << std::setprecision(17);
			for (size_t j = 0; j<d; ++j) std::cout << std::setw(20) << v[j] << " ";
			ostr << std::setprecision(5) << std::endl;
		}

		// generate random data vector
		template<typename element_T>
		void randomVectorFill(size_t n, std::vector<element_T>& vec) {
			for (size_t i = 0; i < n; i++) {
				int rnd1 = rand();
				int rnd2 = rand();
				double rnd = rnd1 / (double)rnd2;
				vec[i] = (element_T)rnd;
			}
		}

		// generate a vector of random permutations around 1.0
		// contraction is a right shift of the random variable causing smaller fluctuations
		// RAND_MAX is typically a 16bit number so can't contract more than 15 bits
		template<typename element_T>
		void randomVectorFillAroundOneEPS(size_t n, std::vector<element_T>& vec, size_t contraction = 6) {
			for (size_t i = 0; i < n; i++) {
				int rnd1 = (rand() - (RAND_MAX >> 1)) >> contraction;
				double eps = rnd1 / (double)RAND_MAX;
				double v = 1.0 + eps;
				vec[i] = (element_T)v;
			}
		}

		// print a sampling of the provided vector
		// if samples is set to 0, all elements of the vector are printed
		template<typename element_T>
		void sampleVector(std::string vec_name, std::vector<element_T>& vec, uint32_t start = 0, uint32_t incr = 1, uint32_t nrSamples = 0) {
			std::cout << "Vector sample is: " << '\n';
			if (nrSamples) {
				uint32_t printed = 0;
				for (uint32_t i = start; i < vec.size(); i += incr) {
					if (printed < nrSamples) {
						printed++;
						cout << vec_name << "[" << setw(3) << i << "] = " << setprecision(15) << vec[i] << '\n';
					}
				}
			}
			else {
				for (uint32_t i = start; i < vec.size(); i += incr) {
					cout << vec_name << "[" << setw(3) << i << "] = " << setprecision(15) << vec[i] << '\n';
				}
			}
			std::cout << std::endl;
		}

	} // namespace blas

} // namespace sw
