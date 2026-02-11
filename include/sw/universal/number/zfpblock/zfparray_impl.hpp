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

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include <algorithm>

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

	// default constructor — empty array
	zfparray() : _n(0), _rate(0), _cached_block(SIZE_MAX), _dirty(false) {
		std::memset(_cache, 0, sizeof(_cache));
	}

	// construct with n elements at given rate (bits per value)
	zfparray(size_t n, double rate)
		: _n(n), _rate(rate), _cached_block(SIZE_MAX), _dirty(false) {
		std::memset(_cache, 0, sizeof(_cache));
		_store.resize(num_blocks() * bytes_per_block(), 0);
	}

	// construct from raw data
	zfparray(size_t n, double rate, const Real* src)
		: _n(n), _rate(rate), _cached_block(SIZE_MAX), _dirty(false) {
		std::memset(_cache, 0, sizeof(_cache));
		_store.resize(num_blocks() * bytes_per_block(), 0);
		compress(src);
	}

	// copy constructor — flush source cache, then copy
	zfparray(const zfparray& other)
		: _store(other._store), _n(other._n), _rate(other._rate),
		  _cached_block(SIZE_MAX), _dirty(false) {
		// flush other's dirty cache so compressed store is up-to-date
		if (other._dirty) {
			const_cast<zfparray&>(other).flush();
		}
		_store = other._store;  // re-copy after flush
		std::memset(_cache, 0, sizeof(_cache));
	}

	// move constructor
	zfparray(zfparray&& other) noexcept
		: _store(std::move(other._store)), _n(other._n), _rate(other._rate),
		  _cached_block(other._cached_block), _dirty(other._dirty) {
		std::memcpy(_cache, other._cache, sizeof(_cache));
		other._n = 0;
		other._cached_block = SIZE_MAX;
		other._dirty = false;
	}

	// copy assignment
	zfparray& operator=(const zfparray& other) {
		if (this != &other) {
			flush();  // flush our dirty cache
			if (other._dirty) {
				const_cast<zfparray&>(other).flush();
			}
			_store = other._store;
			_n = other._n;
			_rate = other._rate;
			_cached_block = SIZE_MAX;
			_dirty = false;
			std::memset(_cache, 0, sizeof(_cache));
		}
		return *this;
	}

	// move assignment
	zfparray& operator=(zfparray&& other) noexcept {
		if (this != &other) {
			flush();
			_store = std::move(other._store);
			_n = other._n;
			_rate = other._rate;
			_cached_block = other._cached_block;
			_dirty = other._dirty;
			std::memcpy(_cache, other._cache, sizeof(_cache));
			other._n = 0;
			other._cached_block = SIZE_MAX;
			other._dirty = false;
		}
		return *this;
	}

	~zfparray() {
		if (_dirty) flush();
	}

	// read element at index i (logically const, may load cache)
	Real operator()(size_t i) const {
		size_t block_idx = i / BLOCK_SIZE;
		size_t offset    = i % BLOCK_SIZE;
		load_block(block_idx);
		return _cache[offset];
	}

	// write element at index i
	void set(size_t i, Real val) {
		size_t block_idx = i / BLOCK_SIZE;
		size_t offset    = i % BLOCK_SIZE;
		load_block(block_idx);
		_cache[offset] = val;
		_dirty = true;
	}

	// compress entire array from raw data
	void compress(const Real* src) {
		_cached_block = SIZE_MAX;
		_dirty = false;

		size_t nblk = num_blocks();
		size_t bpb = bytes_per_block();
		size_t maxbits = static_cast<size_t>(_rate * BLOCK_SIZE);

		for (size_t b = 0; b < nblk; ++b) {
			Real block_data[BLOCK_SIZE];
			std::memset(block_data, 0, sizeof(block_data));

			// copy valid elements (handles partial last block)
			size_t start = b * BLOCK_SIZE;
			size_t count = std::min(BLOCK_SIZE, _n - start);
			std::memcpy(block_data, src + start, count * sizeof(Real));

			uint8_t temp[MAX_BYTES];
			std::memset(temp, 0, sizeof(temp));
			unsigned maxprec = zfp_type_traits<Real>::precision_bits;
			encode_block<Real, Dim>(block_data, temp, MAX_BYTES, maxprec, maxbits);

			// copy truncated compressed data into store
			std::memcpy(_store.data() + b * bpb, temp, bpb);
		}
	}

	// decompress entire array to raw data
	void decompress(Real* dst) const {
		// flush dirty cache first so store is up-to-date
		if (_dirty) {
			const_cast<zfparray*>(this)->flush();
		}

		size_t nblk = num_blocks();
		size_t bpb = bytes_per_block();
		size_t maxbits = static_cast<size_t>(_rate * BLOCK_SIZE);

		for (size_t b = 0; b < nblk; ++b) {
			Real block_data[BLOCK_SIZE];
			unsigned maxprec = zfp_type_traits<Real>::precision_bits;

			// decode from a temporary buffer padded to MAX_BYTES
			uint8_t temp[MAX_BYTES];
			std::memset(temp, 0, sizeof(temp));
			std::memcpy(temp, _store.data() + b * bpb, bpb);
			decode_block<Real, Dim>(temp, MAX_BYTES, block_data, maxprec, maxbits);

			// copy only valid elements
			size_t start = b * BLOCK_SIZE;
			size_t count = std::min(BLOCK_SIZE, _n - start);
			std::memcpy(dst + start, block_data, count * sizeof(Real));
		}
	}

	// number of elements
	size_t size() const { return _n; }

	// number of compressed blocks
	size_t num_blocks() const {
		return (_n + BLOCK_SIZE - 1) / BLOCK_SIZE;
	}

	// bits per value
	double rate() const { return _rate; }

	// compressed bytes per block
	size_t bytes_per_block() const {
		size_t bits = static_cast<size_t>(_rate * BLOCK_SIZE);
		return (bits + 7) / 8;
	}

	// total compressed storage in bytes
	size_t compressed_bytes() const { return _store.size(); }

	// compression ratio: uncompressed / compressed
	double compression_ratio() const {
		if (_store.empty()) return 0.0;
		return static_cast<double>(_n * sizeof(Real)) / static_cast<double>(_store.size());
	}

	// write-back dirty cache without evicting
	void flush() {
		if (_dirty && _cached_block != SIZE_MAX) {
			write_back_cache();
			_dirty = false;
		}
	}

	// invalidate cache (must flush first if dirty)
	void clear_cache() const {
		if (_dirty) {
			const_cast<zfparray*>(this)->flush();
		}
		_cached_block = SIZE_MAX;
		_dirty = false;
	}

	// resize, preserving rate (data is lost)
	void resize(size_t n) {
		flush();
		_n = n;
		_store.assign(num_blocks() * bytes_per_block(), 0);
		_cached_block = SIZE_MAX;
		_dirty = false;
		std::memset(_cache, 0, sizeof(_cache));
	}

	// change rate, recompress (requires full decompression/recompression)
	void set_rate(double rate) {
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
	const uint8_t* data() const { return _store.data(); }

	// raw compressed data size
	size_t data_size() const { return _store.size(); }

private:
	std::vector<uint8_t> _store;            // compressed blocks
	size_t               _n;                // total element count
	double               _rate;             // bits per value

	mutable Real         _cache[BLOCK_SIZE];// decompressed block
	mutable size_t       _cached_block;     // cached block index (SIZE_MAX = none)
	mutable bool         _dirty;            // cache modified flag

	// load block into cache, evicting current block if necessary
	void load_block(size_t block_idx) const {
		if (_cached_block == block_idx) return;  // already cached

		// evict current block (write-back if dirty)
		if (_dirty && _cached_block != SIZE_MAX) {
			const_cast<zfparray*>(this)->write_back_cache();
		}

		// decompress requested block into cache
		size_t bpb = bytes_per_block();
		size_t maxbits = static_cast<size_t>(_rate * BLOCK_SIZE);
		unsigned maxprec = zfp_type_traits<Real>::precision_bits;

		uint8_t temp[MAX_BYTES];
		std::memset(temp, 0, sizeof(temp));
		std::memcpy(temp, _store.data() + block_idx * bpb, bpb);
		decode_block<Real, Dim>(temp, MAX_BYTES, _cache, maxprec, maxbits);

		_cached_block = block_idx;
		_dirty = false;
	}

	// write dirty cache back to compressed store
	void write_back_cache() {
		size_t bpb = bytes_per_block();
		size_t maxbits = static_cast<size_t>(_rate * BLOCK_SIZE);
		unsigned maxprec = zfp_type_traits<Real>::precision_bits;

		// for partial last block, ensure padding is zero
		Real padded[BLOCK_SIZE];
		std::memcpy(padded, _cache, sizeof(padded));
		size_t start = _cached_block * BLOCK_SIZE;
		if (start + BLOCK_SIZE > _n) {
			// zero-pad beyond valid elements
			size_t valid = _n - start;
			for (size_t i = valid; i < BLOCK_SIZE; ++i) {
				padded[i] = Real(0);
			}
		}

		uint8_t temp[MAX_BYTES];
		std::memset(temp, 0, sizeof(temp));
		encode_block<Real, Dim>(padded, temp, MAX_BYTES, maxprec, maxbits);

		std::memcpy(_store.data() + _cached_block * bpb, temp, bpb);
	}
};

}} // namespace sw::universal
