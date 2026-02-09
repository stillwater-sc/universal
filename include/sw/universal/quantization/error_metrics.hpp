#pragma once
// error_metrics.hpp: quantization error metrics (RMSE, SNR, QSNR)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Two API styles:
//
//   1. Scalar-type quantization — matches the qsnr<NumberType>() pattern in qsnr.hpp.
//      Takes a reference vector and a template NumberType that performs the quantization:
//        rmse<NumberType>(data), snr<NumberType>(data), qsnr<NumberType>(data)
//
//   2. Pre-quantized pair — for block formats (mxblock, nvblock, zfparray) or any
//      external quantization pipeline where src and dst are already available:
//        rmse(src, dst), snr(src, dst), qsnr(src, dst)

#include <cassert>
#include <cmath>
#include <limits>
#include <vector>

namespace sw { namespace universal {

// -------------------------------------------------------------------------
// Pre-quantized pair API:  metrics from (original, quantized) vector pairs
// -------------------------------------------------------------------------

/// Root Mean Square Error between original and quantized vectors
template<typename Real>
double rmse(const std::vector<Real>& src, const std::vector<Real>& dst) {
	assert(src.size() == dst.size() && "rmse: vectors must be the same length");
	size_t n = src.size();
	double sum_sq = 0.0;
	for (size_t i = 0; i < n; ++i) {
		double d = static_cast<double>(src[i]) - static_cast<double>(dst[i]);
		sum_sq += d * d;
	}
	return std::sqrt(sum_sq / static_cast<double>(n));
}

/// Signal-to-Noise Ratio in dB: 10 * log10( E[x^2] / E[(x - Q(x))^2] )
template<typename Real>
double snr(const std::vector<Real>& src, const std::vector<Real>& dst) {
	assert(src.size() == dst.size() && "snr: vectors must be the same length");
	size_t n = src.size();
	double signal = 0.0;
	double noise  = 0.0;
	for (size_t i = 0; i < n; ++i) {
		double s = static_cast<double>(src[i]);
		double d = s - static_cast<double>(dst[i]);
		signal += s * s;
		noise  += d * d;
	}
	if (noise == 0.0) noise = std::numeric_limits<double>::min();
	return 10.0 * std::log10(signal / noise);
}

/// Quantization Signal-to-Noise Ratio in dB
///   QSNR = 10 * log10( signal_variance / noise_power )
/// where signal_variance = stddev^2 of the source and
///       noise_power     = E[(Q(x) - x)^2].
template<typename Real>
double qsnr(const std::vector<Real>& src, const std::vector<Real>& dst) {
	assert(src.size() == dst.size() && "qsnr: vectors must be the same length");
	size_t n = src.size();

	// signal mean
	double mean = 0.0;
	for (size_t i = 0; i < n; ++i) {
		mean += static_cast<double>(src[i]);
	}
	mean /= static_cast<double>(n);

	// signal variance and noise power
	double variance = 0.0;
	double noise    = 0.0;
	for (size_t i = 0; i < n; ++i) {
		double s = static_cast<double>(src[i]);
		double diff = s - mean;
		variance += diff * diff;
		double err = s - static_cast<double>(dst[i]);
		noise += err * err;
	}
	variance /= static_cast<double>(n);
	noise    /= static_cast<double>(n);
	if (noise == 0.0) noise = std::numeric_limits<double>::epsilon();
	return 10.0 * std::log10(variance / noise);
}

// -------------------------------------------------------------------------
// Scalar-type quantization API:  quantize through NumberType, then measure
//
// Mirrors the qsnr<NumberType>(data) pattern in qsnr.hpp, adding rmse and
// snr variants.  Uses std::vector<double> to avoid a blas dependency.
// -------------------------------------------------------------------------

/// RMSE of quantizing data through NumberType
template<typename NumberType>
double rmse(const std::vector<double>& data) {
	size_t n = data.size();
	double sum_sq = 0.0;
	for (size_t i = 0; i < n; ++i) {
		double q = static_cast<double>(NumberType(data[i]));
		double d = data[i] - q;
		sum_sq += d * d;
	}
	return std::sqrt(sum_sq / static_cast<double>(n));
}

/// SNR (dB) of quantizing data through NumberType
///   SNR = 10 * log10( E[x^2] / E[(x - Q(x))^2] )
template<typename NumberType>
double snr(const std::vector<double>& data) {
	size_t n = data.size();
	double signal = 0.0;
	double noise  = 0.0;
	for (size_t i = 0; i < n; ++i) {
		double q = static_cast<double>(NumberType(data[i]));
		double d = data[i] - q;
		signal += data[i] * data[i];
		noise  += d * d;
	}
	if (noise == 0.0) noise = std::numeric_limits<double>::min();
	return 10.0 * std::log10(signal / noise);
}

/// QSNR (dB) of quantizing data through NumberType
///   QSNR = 10 * log10( signal_variance / noise_power )
template<typename NumberType>
double qsnr(const std::vector<double>& data) {
	size_t n = data.size();

	// signal mean
	double mean = 0.0;
	for (size_t i = 0; i < n; ++i) {
		mean += data[i];
	}
	mean /= static_cast<double>(n);

	// signal variance and noise power
	double variance = 0.0;
	double noise    = 0.0;
	for (size_t i = 0; i < n; ++i) {
		double diff = data[i] - mean;
		variance += diff * diff;
		double q = static_cast<double>(NumberType(data[i]));
		double err = data[i] - q;
		noise += err * err;
	}
	variance /= static_cast<double>(n);
	noise    /= static_cast<double>(n);
	if (noise == 0.0) noise = std::numeric_limits<double>::epsilon();
	return 10.0 * std::log10(variance / noise);
}

}} // namespace sw::universal

/*

Notice the key differences between QSNR vs SNR:

  - Sinusoidal data: QSNR = SNR because the sinusoid has zero mean, so variance == E[x^2]
  - Linear ramp data: QSNR < SNR by ~6 dB because the ramp has a non-zero mean, so variance < E[x^2]
  — the QSNR correctly measures noise relative to signal variation, not signal power
  - zfp rate=16 ramp: QSNR shows a finite 145.74 dB instead of inf because the epsilon guard fires

*/
