// CHEBPTS(n,kind) - returns the n Chebyshev nodes of the kind.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Author: James Quinlan
// See Readme for further details

#pragma once
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <numeric/containers.hpp>

namespace sw {
	namespace chebyshev {
		using namespace sw::universal;
		using namespace sw::numeric::containers;

		template<typename Scalar>
		vector<Scalar> chebpts(int n, size_t kind = 2)
		{
			if (n < 0) {
				std::cerr << "Parameter must be a positive integer. Provided n == " << n << '\n';
				return blas::vector<Scalar>(1);
			}
			constexpr double _PI = 3.14159265358979323846;
			blas::vector<Scalar>x(n);
			int m = n - 1;
			switch (kind) {
			case 1: // Chebyshev 1st Kind 
				for (int k = n; k > 0; --k) {
					x(n - k) = sin(_PI * (n - 2 * k + 1) / (2 * n));
				}
				break;
			case 2: // Chebyshev 2nd Kind (default)
				for (int k = m; k >= 0; --k) {
					x(m - k) = sin(_PI * (m - 2 * k) / (2 * m));
				}
				break;
			default: // Chebyshev 2nd Kind
				for (int k = m; k >= 0; --k) {
					x(m - k) = sin(_PI * (m - 2 * k) / (2 * m));
				}
				break;
			}
			return x;
		}
	}
}
