#pragma once
// efloat_limbs.hpp: the significand storage of efloat (#1820)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// efloat holds its significand as 64-bit limbs, least significant first: the format of the
// limb_kernels kernels, so an operand goes to them as it is and a result comes back without being
// repacked. The limbs live in a buffer inside the value, N limbs long, and on the heap when a
// value needs more. A heap block that is freed goes to a small cache of the thread's, and the
// next value that needs one takes it from there, so a loop of c = a + b at a precision past the
// inline buffer reaches a state in which it allocates nothing: MPFR reuses the destination's
// limbs in the same way.
//
// Copies copy the limbs in use, not the buffer, and an assignment into a value that has room
// allocates nothing. The storage is usable in constant evaluation, where a value past the
// inline buffer allocates with new and frees with delete (#1819).
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace sw { namespace universal {

namespace efloat_storage {

using limb_t = std::uint64_t;

// The thread's freed heap blocks. Trivially destructible, so that it stays usable while the
// thread's other objects are destroyed; the reaper below frees what it holds.
struct block_cache {
	static constexpr unsigned      slots         = 8;
	static constexpr std::uint32_t largest_kept  = 1u << 16;   // limbs: a larger block is freed, not kept
	limb_t*       block[slots];
	std::uint32_t capacity[slots];
	unsigned      count;
	bool          closed;   // the thread is ending: blocks are freed, not kept
};
inline thread_local block_cache cache{};

struct cache_reaper {
	cache_reaper() noexcept {}
	cache_reaper(const cache_reaper&) = delete;
	cache_reaper& operator=(const cache_reaper&) = delete;
	~cache_reaper() {
		for (unsigned i = 0; i < cache.count; ++i) delete[] cache.block[i];
		cache.count  = 0;
		cache.closed = true;
	}
};
inline thread_local cache_reaper reaper;

// d[0, n) = s[0, n) for limbs that do not overlap, or overlap with d below s: a loop for the
// short runs, where a call to memmove costs more than the copy, and std::copy_n past them
constexpr void copy_limbs(limb_t* d, const limb_t* s, std::size_t n) noexcept {
	if (n <= 8 || std::is_constant_evaluated()) {
		for (std::size_t i = 0; i < n; ++i) d[i] = s[i];
	}
	else {
		std::copy_n(s, n, d);
	}
}
// d[0, n) = 0, in the same way
constexpr void zero_limbs(limb_t* d, std::size_t n) noexcept {
	if (n <= 8 || std::is_constant_evaluated()) {
		for (std::size_t i = 0; i < n; ++i) d[i] = 0u;
	}
	else {
		std::fill_n(d, n, limb_t(0));
	}
}

// heap blocks come in powers of 2, so that freed ones fit the next request
constexpr std::uint32_t block_capacity(std::size_t n) noexcept {
	return static_cast<std::uint32_t>(std::bit_ceil(std::max<std::size_t>(n, 16)));
}

inline limb_t* take_block(std::uint32_t capacity) {
	block_cache& c = cache;
	for (unsigned i = 0; i < c.count; ++i) {
		if (c.capacity[i] == capacity) {
			limb_t* b = c.block[i];
			--c.count;
			c.block[i]    = c.block[c.count];
			c.capacity[i] = c.capacity[c.count];
			return b;
		}
	}
	return new limb_t[capacity];
}

inline void give_block(limb_t* b, std::uint32_t capacity) noexcept {
	block_cache& c = cache;
	if (!c.closed && c.count < block_cache::slots && capacity <= block_cache::largest_kept) {
		(void)&reaper;   // constructs the reaper, which frees the cache when the thread ends
		c.block[c.count]    = b;
		c.capacity[c.count] = capacity;
		++c.count;
		return;
	}
	delete[] b;
}

// Scratch limbs for an operation: N on the stack, or a block from the thread's cache when the
// operation needs more, returned to it at the end of the scope
template<std::size_t N>
class scratch_limbs {
public:
	scratch_limbs() noexcept : _heap{ nullptr }, _capacity{ 0 } {}
	scratch_limbs(const scratch_limbs&) = delete;
	scratch_limbs& operator=(const scratch_limbs&) = delete;
	~scratch_limbs() { if (_heap != nullptr) give_block(_heap, _capacity); }
	// n limbs, whose values are left to the caller; a second call may move them
	limb_t* get(std::size_t n) {
		if (n <= N) return _local;
		if (n > _capacity) {
			if (_heap != nullptr) give_block(_heap, _capacity);
			_capacity = block_capacity(n);
			_heap     = take_block(_capacity);
		}
		return _heap;
	}
private:
	limb_t        _local[N];
	limb_t*       _heap;
	std::uint32_t _capacity;
};

}  // namespace efloat_storage

// N limbs inside the value, and a heap block past them
template<std::size_t N>
class efloat_limbs {
public:
	using limb_t = efloat_storage::limb_t;
	static_assert(N >= 1, "efloat_limbs needs at least one inline limb");

	// the inline limbs are left uninitialized at run time, and zeroed in constant evaluation, whose
	// results must be initialized throughout
	constexpr efloat_limbs() noexcept : _heap{ nullptr }, _size{ 0 }, _capacity{ static_cast<std::uint32_t>(N) } { constant_init(); }
	constexpr efloat_limbs(const efloat_limbs& rhs) : _heap{ nullptr }, _size{ 0 }, _capacity{ static_cast<std::uint32_t>(N) } {
		constant_init();
		assign(rhs.data(), rhs._size);
	}
	constexpr efloat_limbs(efloat_limbs&& rhs) noexcept : _heap{ nullptr }, _size{ 0 }, _capacity{ static_cast<std::uint32_t>(N) } {
		constant_init();
		steal(rhs);
	}
	constexpr efloat_limbs& operator=(const efloat_limbs& rhs) {
		if (this != &rhs) assign(rhs.data(), rhs._size);
		return *this;
	}
	constexpr efloat_limbs& operator=(efloat_limbs&& rhs) noexcept {
		if (this == &rhs) return *this;
		if (rhs._heap != nullptr || rhs._size > _capacity) {
			release();
			steal(rhs);
		}
		else {
			copy_from(rhs._local, rhs.local_size());   // fits: keep this buffer, which may be a heap block
		}
		return *this;
	}
	constexpr ~efloat_limbs() { release(); }

	constexpr std::size_t   size()     const noexcept { return _size; }
	constexpr bool          empty()    const noexcept { return _size == 0; }
	constexpr std::size_t   capacity() const noexcept { return _capacity; }
	constexpr limb_t*       data()           noexcept { return (_heap != nullptr) ? _heap : _local; }
	constexpr const limb_t* data()     const noexcept { return (_heap != nullptr) ? _heap : _local; }
	constexpr limb_t&       operator[](std::size_t i)       noexcept { return data()[i]; }
	constexpr const limb_t& operator[](std::size_t i) const noexcept { return data()[i]; }
	constexpr limb_t&       back()           noexcept { return data()[_size - 1]; }
	constexpr const limb_t& back()     const noexcept { return data()[_size - 1]; }

	constexpr void clear() noexcept { _size = 0; }
	// n limbs whose values are left to the caller: the old ones are not kept when the buffer grows
	constexpr limb_t* resize_for_overwrite(std::size_t n) {
		if (n > _capacity) grow(n, false);
		_size = static_cast<std::uint32_t>(n);
		return data();
	}
	// n limbs, the first min(n, size()) kept and any new ones zero
	constexpr void resize(std::size_t n) {
		if (n > _capacity) grow(n, true);
		limb_t* p = data();
		for (std::size_t i = _size; i < n; ++i) p[i] = 0u;
		_size = static_cast<std::uint32_t>(n);
	}
	// the n limbs of p, which must not lie in this buffer when it grows
	constexpr void assign(const limb_t* p, std::size_t n) {
		if (n > _capacity) grow(n, false);
		copy_from(p, n);
	}
	constexpr void assign(std::size_t n, limb_t v) {
		limb_t* p = resize_for_overwrite(n);
		for (std::size_t i = 0; i < n; ++i) p[i] = v;
	}

private:
	limb_t        _local[N];
	limb_t*       _heap;
	std::uint32_t _size;
	std::uint32_t _capacity;

	// the limbs of a value without a heap block, which are at most N: said so to the compiler,
	// which cannot see it and warns of a copy past the inline limbs
	constexpr std::size_t local_size() const noexcept { return (_size <= N) ? _size : N; }
	constexpr void constant_init() noexcept {
		if (std::is_constant_evaluated()) {
			for (std::size_t i = 0; i < N; ++i) _local[i] = 0u;
		}
	}
	constexpr void copy_from(const limb_t* p, std::size_t n) noexcept {
		efloat_storage::copy_limbs(data(), p, n);
		_size = static_cast<std::uint32_t>(n);
	}
	constexpr void steal(efloat_limbs& rhs) noexcept {
		if (rhs._heap != nullptr) {
			_heap     = rhs._heap;
			_size     = rhs._size;
			_capacity = rhs._capacity;
			rhs._heap     = nullptr;
			rhs._capacity = static_cast<std::uint32_t>(N);
		}
		else {
			copy_from(rhs._local, rhs.local_size());
		}
		rhs._size = 0;
	}
	constexpr void grow(std::size_t n, bool keep) {
		const std::uint32_t capacity = efloat_storage::block_capacity(n);
		limb_t* block = nullptr;
		if (std::is_constant_evaluated()) block = new limb_t[capacity];
		else                              block = efloat_storage::take_block(capacity);
		if (keep) efloat_storage::copy_limbs(block, data(), _size);
		release();
		_heap     = block;
		_capacity = capacity;
	}
	constexpr void release() noexcept {
		if (_heap == nullptr) return;
		if (std::is_constant_evaluated()) delete[] _heap;
		else                              efloat_storage::give_block(_heap, _capacity);
		_heap     = nullptr;
		_capacity = static_cast<std::uint32_t>(N);
	}
};

}}  // namespace sw::universal
