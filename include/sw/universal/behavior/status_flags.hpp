// status_flags.hpp: generalized IEEE-754 exception/status flags for the Universal Numbers Library
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cstdint>

namespace sw { namespace universal {

// Standard IEEE-754 exception flags
struct ExceptionFlag {
	static constexpr uint32_t Inexact          = 1u << 0;
	static constexpr uint32_t Underflow        = 1u << 1;
	static constexpr uint32_t Overflow         = 1u << 2;
	static constexpr uint32_t DivisionByZero   = 1u << 3;
	static constexpr uint32_t InvalidOperation = 1u << 4;
};

// Thread-local sticky exception flags container
class ExceptionFlags {
public:
	constexpr ExceptionFlags() noexcept : _flags(0) {}

	constexpr void set(uint32_t flag) noexcept { _flags |= flag; }
	constexpr void clear(uint32_t flag) noexcept { _flags &= ~flag; }
	constexpr void clear_all() noexcept { _flags = 0; }
	constexpr bool has(uint32_t flag) const noexcept { return (_flags & flag) != 0; }

	constexpr uint32_t get() const noexcept { return _flags; }
	constexpr void set_state(uint32_t state) noexcept { _flags = state; }

private:
	uint32_t _flags;
};

// Shared thread-local exception flags for efloat operations
inline thread_local ExceptionFlags efloat_exception_flags;

// Global helper wrappers for user interaction
inline void clear_efloat_exceptions() noexcept { efloat_exception_flags.clear_all(); }
inline uint32_t get_efloat_exceptions() noexcept { return efloat_exception_flags.get(); }
inline bool has_efloat_exception(uint32_t flag) noexcept { return efloat_exception_flags.has(flag); }
inline void set_efloat_exceptions(uint32_t state) noexcept { efloat_exception_flags.set_state(state); }

}} // namespace sw::universal
