#pragma once
// statistics.hpp: statistics routines
//
// Copyright (C) 2023-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <limits>
#include <universal/blas/vector.hpp>

namespace sw { namespace universal { namespace blas {

	template<typename Scalar>
	struct SummaryStats {
		SummaryStats() = default;

		Scalar mean;
		Scalar stddev;
		Scalar quartiles[5];
	};

	template<typename Scalar>
	std::ostream& operator<<(std::ostream& ostr, const SummaryStats<Scalar>& stats) {
		ostr << "mean     : " << stats.mean << '\n';
		ostr << "stddev   : " << stats.stddev << '\n';
		ostr << "quartiles\n";
		ostr << " [ " << stats.quartiles[0] << ", "
	 	     << stats.quartiles[1] << ", "
	 	     << stats.quartiles[2] << ", "
	 	     << stats.quartiles[3] << ", "
	 	     << stats.quartiles[4] << "]\n";
		return ostr;
	}

	template<typename Vector>
	SummaryStats<typename Vector::value_type> summaryStatistics(const Vector& data) {
		using std::isnan;
		using Scalar = typename Vector::value_type;
		SummaryStats<Scalar> stats;
		size_t N = size(data);
		Scalar sum{0};
		for (auto e : data) {
			sum += e;
		}
		stats.mean = sum / Scalar(N);
		sum = 0.0;
		for (auto e : data) {
			Scalar s = (e - stats.mean);
			sum += s*s;
		}
		stats.stddev = sum / Scalar(N - 1); // use sample statistics formula
					      //
		Vector v(data); // create a copy you can sort
		std::sort(v.begin(), v.end(), 
			[](const Scalar& a, const Scalar& b) {
				// making NaN smaller than any other value
				if (isnan(a) && !isnan(b)) return true;
				if (!isnan(a) && isnan(b)) return false;
				return a < b; // this assumes a reasonable interpretation of NaN < NaN
			});
		stats.quartiles[0] = v[0];       // min
		stats.quartiles[1] = v[N/4];     // first quartile
		stats.quartiles[2] = v[N/2];     // second quartile
		stats.quartiles[3] = v[(3*N)/4]; // third quartile
		stats.quartiles[4] = v[N-1];     // max
		return stats;
	}

} } }  // namespace sw::universal::blas
