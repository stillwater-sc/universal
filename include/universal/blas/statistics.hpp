#pragma once
// statistics.hpp: statistics routines
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <limits>
#include <universal/blas/vector.hpp>

namespace sw { namespace universal { namespace blas {

	template<typename Scalar>
	struct Quantiles {
		Quantiles() = default;
		Quantiles(Quantiles&) = default;
		Quantiles(Scalar q0, Scalar q1, Scalar q2, Scalar q3, Scalar q4) {
			set(q0, q1, q2, q3, q4);
		}
		void set(Scalar q0, Scalar q1, Scalar q2, Scalar q3, Scalar q4) {
			q[0] = q0;
			q[1] = q1;
			q[2] = q2;
			q[3] = q3;
			q[4] = q4;
		}
		Scalar q[5];
	};

	template<typename Scalar>
	std::ostream& operator<<(std::ostream& ostr, const Quantiles<Scalar>& quantiles) {
		ostr << "quantiles: ";
		ostr << " [ " 
			<< quantiles.q[0] << ", "
			<< quantiles.q[1] << ", "
			<< quantiles.q[2] << ", "
			<< quantiles.q[3] << ", "
			<< quantiles.q[4]
			<< "]";
		return ostr;
	}

	template<typename Scalar>
	struct SummaryStats {
		SummaryStats() = default;

		Scalar            mean;
		Scalar            stddev;
		Quantiles<Scalar> quantiles;
	};

	template<typename Scalar>
	std::ostream& operator<<(std::ostream& ostr, const SummaryStats<Scalar>& stats) {
		ostr << "mean     : " << stats.mean << '\n';
		ostr << "stddev   : " << stats.stddev << '\n';
		ostr << stats.quantiles << '\n';
		return ostr;
	}

	template<typename Vector>
	SummaryStats<typename Vector::value_type> summaryStatistics(const Vector& data) {
		using std::isnan;
		using std::sqrt;
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
		stats.stddev = sqrt(sum / Scalar(N - 1)); // use sample statistics formula
					      //
		Vector v(data); // create a copy you can sort
		std::sort(v.begin(), v.end(), 
			[](const Scalar& a, const Scalar& b) {
				// making NaN smaller than any other value
				if (isnan(a) && !isnan(b)) return true;
				if (!isnan(a) && isnan(b)) return false;
				return a < b; // this assumes a reasonable interpretation of NaN < NaN
			});
		stats.quantiles.set(v[0],       // min
							v[N/4],     // first quartile
							v[N/2],     // second quartile
							v[(3*N)/4], // third quartile
							v[N-1]);    // max
		return stats;
	}

	template<typename Vector>
	Quantiles<typename Vector::value_type> quantiles(const Vector& data) {
		using std::isnan;
		using Scalar = typename Vector::value_type;
		size_t N = size(data);
		Vector v(data); // create a copy you can sort
		std::sort(v.begin(), v.end(),
			[](const Scalar& a, const Scalar& b) {
				// making NaN smaller than any other value
				if (isnan(a) && !isnan(b)) return true;
				if (!isnan(a) && isnan(b)) return false;
				return a < b; // this assumes a reasonable interpretation of NaN < NaN
			});

		Quantiles<Scalar> quantiles(v[0], v[N/4], v[N/2], v[(3*N)/4], v[N-1]);
		return quantiles;
	}

} } }  // namespace sw::universal::blas
