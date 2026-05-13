#pragma once
// zfparray_impl.hpp: implementation of zfparray compressed array container
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// zfparray wraps the single-block ZFP codec into a multi-block compressed
// array with random access. All blocks use fixed-rate mode so block N
// starts at a computable byte offset. A single-block write-back cache
// provides efficient sequential access patterns.
//
// All public entry points are usable in constant-evaluated contexts in C++20
// after PR #816, building on the codec promotion from PR #815.  The supported
// pattern is a constexpr lambda that constructs a zfparray, exercises the
// API, and returns -- the vector storage is freed at lambda exit, satisfying
// C++20's rule that constexpr allocations not persist beyond the constant
// expression.  Static-storage `constexpr zfparray` instances are not
// supported (because their vector would persist).

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include <algorithm>
#include <type_traits>

#include <universal/number/zfpblock/zfparray_fwd.hpp>
#include <universal/number/zfpblock/zfp_codec_traits.hpp>
#include <universal/number/zfpblock/zfp_codec.hpp>

namespace sw { namespace universal {

template<typename Real, unsigned Dim>
class zfparray {
	static_assert(std::is_same_v<Real, float> || std::is_same_v<Real, double>,
		"zfparray requires float or double");
	static_assert(Dim >= 1 && Dim <= 3, "zfparray requires Dim in {1, 2, 3}");

public:
	static constexpr size_t BLOCK_SIZE = zfp_block_size<Dim>::value;
	static constexpr size_t MAX_BYTES  = zfp_max_bytes<Real, Dim>::value;

	// default constructor -- empty array.  In-class member initializers
	// give us a clean constexpr default state with no memset.
	constexpr zfparray() = default;

	// construct with n elements at given rate (bits per value)
	constexpr zfparray(size_t n, double rate)
		: _n(n), _rate(rate) {
		_store.resize(num_blocks() * bytes_per_block(), 0);
	}

	// construct from raw data
	constexpr zfparray(size_t n, double rate, const Real* src)
		: _n(n), _rate(rate) {
		_store.resize(num_blocks() * bytes_per_block(), 0);
		compress(src);
	}

	// copy constructor -- flush source cache first so its compressed
	// store is up-to-date, then copy.
	constexpr zfparray(const zfparray& other)
		: _n(other._n), _rate(other._rate) {
		if (other._dirty) other.flush();
		_store = other._store;
		// _cache, _cached_block, _dirty are intentionally fresh
		// (left at their in-class defaults) so the copy starts cold.
	}

	// move constructor
	constexpr zfparray(zfparray&& other) noexcept
		: _store(std::move(other._store)), _n(other._n), _rate(other._rate),
		  _cached_block(other._cached_block), _dirty(other._dirty) {
		for (size_t i = 0; i < BLOCK_SIZE; ++i) _cache[i] = other._cache[i];
		other._n = 0;
		other._cached_block = SIZE_MAX;
		other._dirty = false;
	}

	// copy assignment
	constexpr zfparray& operator=(const zfparray& other) {
		if (this != &other) {
			flush();  // flush our dirty cache
			if (other._dirty) other.flush();
			_store = other._store;
			_n = other._n;
			_rate = other._rate;
			_cached_block = SIZE_MAX;
			_dirty = false;
			for (size_t i = 0; i < BLOCK_SIZE; ++i) _cache[i] = Real(0);
		}
		return *this;
	}

	// move assignment
	constexpr zfparray& operator=(zfparray&& other) noexcept {
		if (this != &other) {
			flush();
			_store = std::move(other._store);
			_n = other._n;
			_rate = other._rate;
			_cached_block = other._cached_block;
			_dirty = other._dirty;
			for (size_t i = 0; i < BLOCK_SIZE; ++i) _cache[i] = other._cache[i];
			other._n = 0;
			other._cached_block = SIZE_MAX;
			other._dirty = false;
		}
		return *this;
	}

	constexpr ~zfparray() {
		if (_dirty) flush();
	}

	// read element at index i (logically const, may load cache)
	constexpr Real operator()(size_t i) const {
		size_t block_idx = i / BLOCK_SIZE;
		size_t offset    = i % BLOCK_SIZE;
		load_block(block_idx);
		return _cache[offset];
	}

	// write element at index i
	constexpr void set(size_t i, Real val) {
		size_t block_idx = i / BLOCK_SIZE;
		size_t offset    = i % BLOCK_SIZE;
		load_block(block_idx);
		_cache[offset] = val;
		_dirty = true;
	}

	// compress entire array from raw data
	constexpr void compress(const Real* src) {
		_cached_block = SIZE_MAX;
		_dirty = false;

		size_t nblk = num_blocks();
		size_t bpb = bytes_per_block();
		size_t maxbits = static_cast<size_t>(_rate * BLOCK_SIZE);

		for (size_t b = 0; b < nblk; ++b) {
			Real block_data[BLOCK_SIZE]{};

			// copy valid elements (handles partial last block); remaining
			// elements remain value-initialized to Real(0).
			size_t start = b * BLOCK_SIZE;
			size_t count = std::min(BLOCK_SIZE, _n - start);
			for (size_t i = 0; i < count; ++i) block_data[i] = src[start + i];

			uint8_t temp[MAX_BYTES]{};
			unsigned maxprec = zfp_type_traits<Real>::precision_bits;
			encode_block<Real, Dim>(block_data, temp, MAX_BYTES, maxprec, maxbits);

			// copy truncated compressed data into store
			for (size_t i = 0; i < bpb; ++i) _store[b * bpb + i] = temp[i];
		}
	}

	// decompress entire array to raw data
	constexpr void decompress(Real* dst) const {
		if (_dirty) flush();  // ensure _store is up-to-date

		size_t nblk = num_blocks();
		size_t bpb = bytes_per_block();
		size_t maxbits = static_cast<size_t>(_rate * BLOCK_SIZE);

		for (size_t b = 0; b < nblk; ++b) {
			Real block_data[BLOCK_SIZE]{};
			unsigned maxprec = zfp_type_traits<Real>::precision_bits;

			// decode from a temporary buffer padded to MAX_BYTES
			uint8_t temp[MAX_BYTES]{};
			for (size_t i = 0; i < bpb; ++i) temp[i] = _store[b * bpb + i];
			decode_block<Real, Dim>(temp, MAX_BYTES, block_data, maxprec, maxbits);

			// copy only valid elements
			size_t start = b * BLOCK_SIZE;
			size_t count = std::min(BLOCK_SIZE, _n - start);
			for (size_t i = 0; i < count; ++i) dst[start + i] = block_data[i];
		}
	}

	// number of elements
	constexpr size_t size() const { return _n; }

	// number of compressed blocks
	constexpr size_t num_blocks() const {
		return (_n + BLOCK_SIZE - 1) / BLOCK_SIZE;
	}

	// bits per value
	constexpr double rate() const { return _rate; }

	// compressed bytes per block
	constexpr size_t bytes_per_block() const {
		size_t bits = static_cast<size_t>(_rate * BLOCK_SIZE);
		return (bits + 7) / 8;
	}

	// total compressed storage in bytes
	constexpr size_t compressed_bytes() const { return _store.size(); }

	// compression ratio: uncompressed / compressed
	constexpr double compression_ratio() const {
		if (_store.empty()) return 0.0;
		return static_cast<double>(_n * sizeof(Real)) / static_cast<double>(_store.size());
	}

	// write-back dirty cache without evicting.  const-callable because
	// it only mutates mutable members (_store, _dirty, the cache).
	constexpr void flush() const {
		if (_dirty && _cached_block != SIZE_MAX) {
			write_back_cache();
			_dirty = false;
		}
	}

	// invalidate cache (flushes first if dirty)
	constexpr void clear_cache() const {
		if (_dirty) flush();
		_cached_block = SIZE_MAX;
		_dirty = false;
	}

	// resize, preserving rate (data is lost)
	constexpr void resize(size_t n) {
		flush();
		_n = n;
		_store.assign(num_blocks() * bytes_per_block(), 0);
		_cached_block = SIZE_MAX;
		_dirty = false;
		for (size_t i = 0; i < BLOCK_SIZE; ++i) _cache[i] = Real(0);
	}

	// change rate, recompress (requires full decompression/recompression)
	constexpr void set_rate(double rate) {
		if (_n == 0) {
			_rate = rate;
			return;
		}
		// decompress all data at old rate
		std::vector<Real> raw(_n);
		decompress(raw.data());

		// update rate and storage
		_rate = rate;
		_store.assign(num_blocks() * bytes_per_block(), 0);
		_cached_block = SIZE_MAX;
		_dirty = false;

		// recompress at new rate
		compress(raw.data());
	}

	// raw compressed data pointer
	constexpr const uint8_t* data() const { return _store.data(); }

	// raw compressed data size
	constexpr size_t data_size() const { return _store.size(); }

private:
	// _store is mutable because the const lazy-cache-load methods
	// (operator(), decompress, flush, load_block, write_back_cache)
	// need to update the compressed buffer when evicting a dirty cache
	// line.  Conceptually the visible array contents are unchanged --
	// the dirty cache is an internal implementation detail -- so const
	// methods are entitled to flush.
	mutable std::vector<uint8_t> _store{};      // compressed blocks
	size_t                       _n     = 0;    // total element count
	double                       _rate  = 0.0;  // bits per value

	mutable Real         _cache[BLOCK_SIZE] = {};      // decompressed block
	mutable size_t       _cached_block      = SIZE_MAX;// cached block index
	mutable bool         _dirty             = false;   // cache modified flag

	// load block into cache, evicting current block if necessary.
	// const because cache fields and _store are mutable.
	constexpr void load_block(size_t block_idx) const {
		if (_cached_block == block_idx) return;  // already cached

		// evict current block (write-back if dirty)
		if (_dirty && _cached_block != SIZE_MAX) {
			write_back_cache();
		}

		// decompress requested block into cache
		size_t bpb = bytes_per_block();
		size_t maxbits = static_cast<size_t>(_rate * BLOCK_SIZE);
		unsigned maxprec = zfp_type_traits<Real>::precision_bits;

		uint8_t temp[MAX_BYTES]{};
		for (size_t i = 0; i < bpb; ++i) temp[i] = _store[block_idx * bpb + i];
		decode_block<Real, Dim>(temp, MAX_BYTES, _cache, maxprec, maxbits);

		_cached_block = block_idx;
		_dirty = false;
	}

	// write dirty cache back to compressed store.  const because _store
	// is mutable.
	constexpr void write_back_cache() const {
		size_t bpb = bytes_per_block();
		size_t maxbits = static_cast<size_t>(_rate * BLOCK_SIZE);
		unsigned maxprec = zfp_type_traits<Real>::precision_bits;

		// for partial last block, ensure padding is zero
		Real padded[BLOCK_SIZE]{};
		for (size_t i = 0; i < BLOCK_SIZE; ++i) padded[i] = _cache[i];
		size_t start = _cached_block * BLOCK_SIZE;
		if (start + BLOCK_SIZE > _n) {
			// zero-pad beyond valid elements
			size_t valid = _n - start;
			for (size_t i = valid; i < BLOCK_SIZE; ++i) {
				padded[i] = Real(0);
			}
		}

		uint8_t temp[MAX_BYTES]{};
		encode_block<Real, Dim>(padded, temp, MAX_BYTES, maxprec, maxbits);

		for (size_t i = 0; i < bpb; ++i) _store[_cached_block * bpb + i] = temp[i];
	}
};

}} // namespace sw::universal
