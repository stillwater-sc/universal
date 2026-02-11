// api.cpp: application programming interface tests for zfpblock (ZFP compressed block float) codec
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the zfpblock template environment
#define ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/zfpblock/zfpblock.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "zfpblock API tests";
	int nrOfFailedTestCases = 0;

	// demonstrate all 6 type aliases
	std::cout << "+---------    zfpblock type aliases   --------+\n";
	{
		zfp1f a; std::cout << "zfp1f : " << type_tag(a) << '\n';
		zfp2f b; std::cout << "zfp2f : " << type_tag(b) << '\n';
		zfp3f c; std::cout << "zfp3f : " << type_tag(c) << '\n';
		zfp1d d; std::cout << "zfp1d : " << type_tag(d) << '\n';
		zfp2d e; std::cout << "zfp2d : " << type_tag(e) << '\n';
		zfp3d f; std::cout << "zfp3d : " << type_tag(f) << '\n';
	}

	// display block info for each configuration
	std::cout << "+---------    zfpblock info   --------+\n";
	{
		std::cout << zfp_block_info<float, 1>() << '\n';
		std::cout << zfp_block_info<float, 2>() << '\n';
		std::cout << zfp_block_info<float, 3>() << '\n';
		std::cout << zfp_block_info<double, 1>() << '\n';
		std::cout << zfp_block_info<double, 2>() << '\n';
		std::cout << zfp_block_info<double, 3>() << '\n';
	}

	// 1D float reversible round-trip
	std::cout << "+---------    1D float reversible round-trip   --------+\n";
	{
		float input[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
		float output[4] = {};

		zfp1f blk;
		size_t nbits = blk.compress_reversible(input);
		blk.decompress(output);

		std::cout << "Compressed to " << nbits << " bits"
		          << " (" << blk.compressed_bytes() << " bytes)"
		          << ", ratio: " << blk.compression_ratio() << "x\n";
		for (int i = 0; i < 4; ++i) {
			float err = std::abs(output[i] - input[i]);
			std::cout << "  [" << i << "] in=" << input[i] << " out=" << output[i];
			if (err > 0.0f) std::cout << " err=" << err;
			std::cout << '\n';
		}
	}

	// 2D float fixed-rate compression
	std::cout << "+---------    2D float fixed-rate compression   --------+\n";
	{
		float input[16];
		for (int i = 0; i < 16; ++i) input[i] = static_cast<float>(i) * 0.5f;

		zfp2f blk;
		size_t nbits = blk.compress_fixed_rate(input, 8.0);  // 8 bits per value
		std::cout << "Fixed-rate(8): " << nbits << " bits"
		          << ", expected " << (8 * 16) << " bits\n";

		float output[16];
		blk.decompress(output);
		double max_err = 0.0;
		for (int i = 0; i < 16; ++i) {
			double err = std::abs(static_cast<double>(output[i]) - static_cast<double>(input[i]));
			if (err > max_err) max_err = err;
		}
		std::cout << "Max error: " << max_err << '\n';
		std::cout << zfp_compression_stats(blk) << '\n';
	}

	// 1D double reversible
	std::cout << "+---------    1D double reversible round-trip   --------+\n";
	{
		double input[4] = { 3.14159265358979, -2.71828182845905, 1.41421356237310, 0.0 };
		double output[4] = {};

		zfp1d blk;
		blk.compress_reversible(input);
		blk.decompress(output);

		for (int i = 0; i < 4; ++i) {
			double err = std::abs(output[i] - input[i]);
			std::cout << "  [" << i << "] in=" << std::setprecision(15) << input[i]
			          << " out=" << output[i];
			if (err > 0.0) std::cout << " err=" << err;
			std::cout << '\n';
		}
	}

	// all 4 modes with 1D float
	std::cout << "+---------    all modes with 1D float   --------+\n";
	{
		float input[4] = { 1.5f, -2.5f, 3.5f, -4.5f };
		float output[4];

		zfp1f blk;
		// fixed-rate
		blk.compress(input, zfp_mode::fixed_rate, 16.0);
		blk.decompress(output);
		std::cout << "fixed_rate(16): " << blk.compressed_bits() << " bits\n";

		// fixed-precision
		blk.compress(input, zfp_mode::fixed_precision, 16.0);
		blk.decompress(output);
		std::cout << "fixed_precision(16): " << blk.compressed_bits() << " bits\n";

		// fixed-accuracy
		blk.compress(input, zfp_mode::fixed_accuracy, 0.01);
		blk.decompress(output);
		std::cout << "fixed_accuracy(0.01): " << blk.compressed_bits() << " bits\n";

		// reversible
		blk.compress(input, zfp_mode::reversible, 0.0);
		blk.decompress(output);
		std::cout << "reversible: " << blk.compressed_bits() << " bits\n";
	}

	// display binary representation
	std::cout << "+---------    binary representation   --------+\n";
	{
		float input[4] = { 1.0f, 0.5f, 0.25f, 0.125f };
		zfp1f blk;
		blk.compress_reversible(input);
		std::cout << to_binary(blk) << '\n';
	}

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
