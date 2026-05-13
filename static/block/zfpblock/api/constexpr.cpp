// constexpr.cpp: compile-time tests for zfpblock (accessors + full codec roundtrip)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Scope of this constexpr coverage:
//
// zfpblock combines:
//   * a static block-buffer container with simple integer accessors -- all
//     constexpr-promotable (this file exercises that subset);
//   * a 564-line ZFP transform codec (zfp_codec.hpp) using std::frexp /
//     std::ldexp / std::memset / __builtin_ctzll and bit-stream packing.
//     After PR #815 all of the codec is constexpr via std::is_constant_evaluated()
//     dispatch onto cx_ldexp / cx_frexp_exp / std::countr_zero / explicit loops.
//   * a std::vector-backed multi-block array (zfparray) -- NOT in scope.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <universal/number/zfpblock/zfpblock.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "zfpblock constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// zfpblock<float, 1>: 4-element 1D block of floats.
	// zfpblock<double, 2>: 4x4 = 16-element 2D block of doubles.
	// zfpblock<double, 3>: 4x4x4 = 64-element 3D block of doubles.
	using zfp_f1 = zfpblock<float, 1>;
	using zfp_d2 = zfpblock<double, 2>;
	using zfp_d3 = zfpblock<double, 3>;

	// ----------------------------------------------------------------------------
	// Static accessors: BLOCK_SIZE, MAX_BYTES, block_size(), dim().
	// ----------------------------------------------------------------------------
	{
		// zfp_block_size<Dim>::value = 4^Dim (4, 16, 64 for Dim 1/2/3)
		static_assert(zfp_f1::BLOCK_SIZE == 4u,  "constexpr zfpblock<float, 1>::BLOCK_SIZE == 4");
		static_assert(zfp_d2::BLOCK_SIZE == 16u, "constexpr zfpblock<double, 2>::BLOCK_SIZE == 16");
		static_assert(zfp_d3::BLOCK_SIZE == 64u, "constexpr zfpblock<double, 3>::BLOCK_SIZE == 64");

		static_assert(zfp_f1::block_size() == 4u,  "constexpr zfp_f1::block_size() == 4");
		static_assert(zfp_d3::block_size() == 64u, "constexpr zfp_d3::block_size() == 64");

		static_assert(zfp_f1::dim() == 1u, "constexpr zfp_f1::dim() == 1");
		static_assert(zfp_d2::dim() == 2u, "constexpr zfp_d2::dim() == 2");
		static_assert(zfp_d3::dim() == 3u, "constexpr zfp_d3::dim() == 3");

		// MAX_BYTES = upper bound on compressed buffer size; non-zero per Dim.
		static_assert(zfp_f1::MAX_BYTES > 0u, "constexpr zfp_f1::MAX_BYTES > 0");
		static_assert(zfp_d3::MAX_BYTES > 0u, "constexpr zfp_d3::MAX_BYTES > 0");
	}

	// ----------------------------------------------------------------------------
	// Empty-state contract: a value-initialized zfpblock has _nbits = 0,
	// _mode = 0 (first enum value), _param = 0.0.  All three accessors are
	// constexpr-deterministic at value-init.  (Note: trivial default
	// construction `zfp_f1 b;` leaves storage indeterminate; this contract
	// applies only to `zfp_f1 b{};` value-init, which we use throughout.)
	// ----------------------------------------------------------------------------
	{
		constexpr size_t cx_nbits = []() {
			zfp_f1 b{};
			return b.compressed_bits();
		}();
		static_assert(cx_nbits == 0u, "constexpr compressed_bits for empty block == 0");

		constexpr size_t cx_bytes = []() {
			zfp_f1 b{};
			return b.compressed_bytes();
		}();
		static_assert(cx_bytes == 0u, "constexpr compressed_bytes for empty block == 0");

		constexpr double cx_ratio = []() {
			zfp_f1 b{};
			return b.compression_ratio();
		}();
		// With _nbits == 0, compression_ratio returns 0.0 by contract
		// (the early-out branch in compression_ratio()).
		static_assert(cx_ratio == 0.0, "constexpr compression_ratio for empty block == 0.0");
	}

	// ----------------------------------------------------------------------------
	// data() pointer is constexpr (returns &_buffer[0]).  Useful for
	// constexpr inspection of pre-populated blocks.  We just verify the
	// expression is well-formed at constant evaluation.
	// ----------------------------------------------------------------------------
	{
		// data() returns const uint8_t* pointing into the block's buffer.
		// In a constant expression, the pointer's value isn't directly
		// comparable across translation units, but the call must compile.
		constexpr bool has_data = []() {
			zfp_f1 b{};
			return b.data() != nullptr;
		}();
		static_assert(has_data, "constexpr data() returns valid pointer");
	}

	// ----------------------------------------------------------------------------
	// Mode and parameter accessors.  Default-constructed zfpblock has
	// _mode and _param value-initialized (zfp_mode is an enum, defaulted to
	// 0 = zfp_mode::fixed_rate; _param defaulted to 0.0).
	// ----------------------------------------------------------------------------
	{
		constexpr zfp_mode cx_mode = []() {
			zfp_f1 b{};
			return b.mode();
		}();
		(void)cx_mode;  // value depends on zfp_mode enum's first entry; not pinned

		constexpr double cx_param = []() {
			zfp_f1 b{};
			return b.param();
		}();
		static_assert(cx_param == 0.0, "constexpr default param() == 0.0");
	}

	// ----------------------------------------------------------------------------
	// Codec roundtrip: full compress/decompress pipeline at constant evaluation.
	// Exercises zfp_bitstream, fwd_cast (cx_frexp_exp + cx_ldexp), fwd/inv_xform,
	// negabinary conversion, and bit-plane encode/decode -- the whole codec.
	// ----------------------------------------------------------------------------
	{
		// All-zero block: encoder takes the zero-flag fast path; decoder returns
		// all zeros.  This exercises zfp_bitstream's write_bit/read_bit and the
		// all-zero memset replacement on the constexpr branch.
		constexpr bool cx_zero_roundtrip = []() {
			zfp_f1 b{};
			float src[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			b.compress(src, zfp_mode::fixed_rate, 32.0);
			float dst[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			b.decompress(dst);
			return dst[0] == 0.0f && dst[1] == 0.0f
			    && dst[2] == 0.0f && dst[3] == 0.0f;
		}();
		static_assert(cx_zero_roundtrip,
			"constexpr zfpblock<float,1> all-zero compress/decompress roundtrip preserves zero");

		// Nonzero block of powers of two: emax search runs through cx_frexp_exp;
		// quantization runs through cx_ldexp; lifting + bit-plane + decoding
		// run end-to-end.  Verifies the compressed size is nonzero and the
		// decoded values are within a reasonable tolerance of the originals.
		// (Tolerance: the codec is faithful, not bit-exact, for arbitrary inputs;
		// fixed-rate 32 bpv with prec=30 gives recovery within a few ULPs for
		// power-of-two inputs.)
		constexpr bool cx_nonzero_roundtrip = []() {
			zfp_f1 b{};
			float src[4] = { 1.0f, 2.0f, 4.0f, 8.0f };
			b.compress(src, zfp_mode::fixed_rate, 32.0);
			if (b.compressed_bits() == 0) return false;
			float dst[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			b.decompress(dst);
			// Each dst[i] must round-trip to within a small relative tolerance
			// of src[i].  Using max relative error 2^-20 -- a loose bound that
			// catches gross codec failures but tolerates the codec's natural
			// quantization noise.
			auto abs_diff = [](float a, float ref) {
				float d = a - ref;
				return d < 0.0f ? -d : d;
			};
			constexpr float tol_rel = 1.0f / (1u << 20);
			for (unsigned i = 0; i < 4; ++i) {
				float ref = src[i];
				float ref_abs = ref < 0.0f ? -ref : ref;
				if (abs_diff(dst[i], ref) > tol_rel * ref_abs) return false;
			}
			return true;
		}();
		static_assert(cx_nonzero_roundtrip,
			"constexpr zfpblock<float,1> power-of-two compress/decompress roundtrip recovers within tolerance");

		// Negabinary identity at constant evaluation: int2uint and uint2int are
		// mutual inverses for any 32- or 64-bit value.
		static_assert(uint2int<int32_t, uint32_t>(int2uint<int32_t, uint32_t>(int32_t(0))) == 0,
			"constexpr negabinary roundtrip preserves zero");
		static_assert(uint2int<int32_t, uint32_t>(int2uint<int32_t, uint32_t>(int32_t(42))) == 42,
			"constexpr negabinary roundtrip preserves positive int32");
		static_assert(uint2int<int32_t, uint32_t>(int2uint<int32_t, uint32_t>(int32_t(-42))) == -42,
			"constexpr negabinary roundtrip preserves negative int32");
		static_assert(uint2int<int64_t, uint64_t>(int2uint<int64_t, uint64_t>(int64_t(123456789012345LL))) == 123456789012345LL,
			"constexpr negabinary roundtrip preserves wide int64");
	}

	std::cout << "zfpblock constexpr verification: "
	          << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception\n";
	return EXIT_FAILURE;
}
