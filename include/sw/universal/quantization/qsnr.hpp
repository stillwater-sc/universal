// qsnr.hpp: Quantization Signal to Noise ratio for a sampling
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <blas/blas.hpp>
#include <blas/statistics.hpp>

namespace sw { namespace universal {

	/// <summary>
	/// calculate the Signal to Quantization Noise ratio in dB
	/// </summary>
	/// <typeparam name="Scalar"></typeparam>
	/// <param name="v">data set to quantize</param>
	/// <returns>QSNR in dB</returns>
	template<typename Scalar>
	double qsnr(const blas::vector<double>& v) {
		using std::log10;
		// std::cout << type_tag(Scalar()) << " : " << symmetry_range<Scalar>() << '\n';
		size_t N = size(v);

		blas::SummaryStats<double> stats = blas::summaryStatistics(v);
		auto stddev = stats.stddev;

		double sum = 0.0;
		for (auto number : v) {
			double quantized = double(Scalar(number)); // Quantize to Scalar
			double error = number - quantized;
			// std::cout << number << " : " << quantized << " : " << error << '\n';
			sum += error * error;
		}

		double noise_power = sum / N;
		double signal_power = stddev * stddev;
		double SNR = 10 * log10(signal_power / noise_power);

		return SNR;
	}

} } // namespace sw::universal

