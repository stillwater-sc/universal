// scaling.cpp: error measurement of data scaling to fit small and narrow representations
//
// Copyright (C) 2022-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/blas/blas.hpp>

namespace sw {
	namespace universal {
		// data normalization

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
			for (auto e : v) {
				t.push_back( e * scale + offset );
			}
			return t;
		}

		template<typename Target>
		blas::vector<Target> compress(const blas::vector<double>& v) {
			auto maxpos = double(std::numeric_limits<Target>::max());

			auto vminmax = arange(v);
			auto minValue = vminmax.first;
			auto maxValue = vminmax.second;

			sw::universal::blas::vector<Target> t(v.size());
			auto sqrtMaxpos = sqrt(maxpos);
			double maxScale = 1.0;
			if (abs(maxValue) > sqrtMaxpos) maxScale = sqrtMaxpos / maxValue;
			t = maxScale * v;

			return t;
		}

	}
}

/*
 * When we want to take arbitrary vectors and want to faithfully calculate a 
 * dot product using lower precision types, we need to 'squeeze' the values
 * of the original vector such that the computational dynamics of the dot product
 * can be emulated. 
 * 
 * When you think about very constrained types like 8-bit floating-point formats
 * the risk of overflow and underflow of the products is the first problem
 * to solve. Secondly, for long vectors overflow and catastrophic cancellation
 * are also risks.
 *                 
 */


int main()
try {
	using namespace sw::universal;

	unsigned N{ 10000 };
	double mean{ 0.0 }, stddev{ 1.0 };

	auto dv = sw::universal::blas::gaussian_random_vector<double>(N, mean, stddev);
		auto dminmax = blas::range(dv);
		std::cout << dminmax.first << ", " << dminmax.second << '\n';
	auto sv = compress<float>(dv);
		auto sminmax = blas::range(sv);
		std::cout << sminmax.first << ", " << sminmax.second << '\n';
	auto hv = compress<half>(dv);
		auto hminmax = blas::range(hv);
		std::cout << hminmax.first << ", " << hminmax.second << '\n';
	auto qv = compress<quarter>(dv);
		auto qminmax = blas::range(qv);
		std::cout << qminmax.first << ", " << qminmax.second << " : " << symmetry_range<quarter>() << '\n';

	if (N < 15) {
		std::cout << dv << '\n';
		std::cout << sv << '\n';
		std::cout << hv << '\n';
		std::cout << qv << '\n';
	}

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
