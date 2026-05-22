#pragma once
// lazy_component_buffer.hpp: small-buffer-optimised storage for elreal's lazy stream
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase K.1 of the elreal follow-up epic (#905).
//
// Why
// ===
// Each elreal value carries a `_components` field that records the
// materialised prefix of its lazy stream. The Phase I baseline
// (docs/algorithmic-details/elreal-performance-baseline.md) identified
// `std::vector<double>` allocation as the dominant per-operation cost
// for elreal arithmetic operators: each `+ - *` allocates a fresh
// vector for the result and another via the lambda capture in the
// generator. For the common case (depth 1 = 1 component, depth 2 =
// 2 components) the small-buffer optimisation here eliminates both
// allocations.
//
// Design
// ======
// - `inline_capacity = 4` doubles held inline (32 bytes). Covers
//   depth 1 through depth 4 with no heap allocation. The Phase G
//   default refinement budget is 8 components, but as of Phase G the
//   per-operator generators clamp at depth 1, so the typical actually-
//   materialised depth is 1-2.
// - When the logical size exceeds `inline_capacity`, the additional
//   components spill into a heap-allocated `std::vector<double>`. The
//   first `inline_capacity` slots remain in the inline array; spilled
//   entries live at logical indices `[inline_capacity, _size)`.
// - The spill vector's *empty* footprint is still 24 bytes (three
//   pointers), so the full buffer occupies 32 + 24 + 8 = 64 bytes:
//   exactly one cache line on x86_64.
//
// API surface
// ===========
// Only the operations elreal actually needs:
//   - default ctor (empty)
//   - push_back(double)
//   - clear()
//   - operator[](size_t) const
//   - size(), empty()
//   - reserve(size_t)  -- a hint; no-op when n <= inline_capacity
// No iterator support (use indexed iteration). No range-for.
//
// Copy / move semantics
// =====================
// Default copy and move are correct: copying copies the inline buffer
// element-by-element and copies the spill vector (which heap-allocates
// only if it was non-empty); moving moves the spill vector and copies
// the inline buffer (the inline elements are trivially-copyable).
//
// Thread safety
// =============
// The same as std::vector<double>: not internally synchronised. elreal
// already documents that mutation through the materialisation path is
// const-but-not-thread-safe (see the `mutable` member declaration in
// elreal_impl.hpp); this storage upholds the same contract.

#include <array>
#include <cassert>
#include <cstddef>
#include <vector>

namespace sw { namespace universal {

class lazy_component_buffer {
public:
	static constexpr std::size_t inline_capacity = 4;

	lazy_component_buffer() noexcept : _inl_buf{}, _spill{}, _size{0} {}

	// Convenience: construct with a single leading component. Used by
	// elreal's double-valued constructor to avoid going through push_back.
	explicit lazy_component_buffer(double v) noexcept : _inl_buf{}, _spill{}, _size{1} {
		_inl_buf[0] = v;
	}

	void push_back(double v) {
		if (_size < inline_capacity) {
			_inl_buf[_size] = v;
		}
		else {
			_spill.push_back(v);
		}
		++_size;
	}

	void clear() noexcept {
		_spill.clear();
		_size = 0;
	}

	double operator[](std::size_t i) const noexcept {
		assert(i < _size && "lazy_component_buffer index out of range");
		return (i < inline_capacity) ? _inl_buf[i] : _spill[i - inline_capacity];
	}

	std::size_t size() const noexcept { return _size; }
	bool empty() const noexcept { return _size == 0; }

	// Hint for upcoming inserts. n <= inline_capacity is a no-op; larger
	// n pre-reserves the spill vector to avoid incremental growth.
	void reserve(std::size_t n) {
		if (n > inline_capacity) {
			_spill.reserve(n - inline_capacity);
		}
	}

private:
	std::array<double, inline_capacity> _inl_buf;
	std::vector<double>                 _spill;
	std::size_t                         _size;
};

}} // namespace sw::universal
