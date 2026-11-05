#pragma once
// exceptions.hpp: does this build have C++ exceptions, and what to do when it does not
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Firmware, flight controllers and other bare-metal targets build with -fno-exceptions. A header
// that compiles a `throw` there fails to build, even when nothing calls it (#1925). This header
// is the single place the library asks whether exceptions are available:
//
//   UNIVERSAL_HAS_EXCEPTIONS   1 when the compiler has exceptions enabled, 0 otherwise. Taken
//                              from __cpp_exceptions (the standard feature macro), __EXCEPTIONS
//                              (older gcc) or _CPPUNWIND (MSVC /EHsc). An application may define
//                              it first, to 0, to compile the library without throws even when
//                              exceptions are on.
//
//   throw_or_abort(e)          throws e when exceptions are available and calls std::abort()
//                              otherwise. It is for the errors that have no non-throwing result:
//                              a bad argument to a container accessor, an exhausted iteration
//                              budget, an operand a quire cannot hold. It is constexpr, so
//                              reaching it during constant evaluation is still a compile error.
//
// The arithmetic errors of each number system are gated by that system's own
// *_THROW_ARITHMETIC_EXCEPTION and *_THROW_DIVIDE_BY_ZERO macros instead. Those default to 0
// without exceptions, and each system's header stops the build with an #error when one of them
// is set to 1 in a build that cannot throw.

#include <cstdlib>
#include <utility>

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
#define UNIVERSAL_COMPILER_HAS_EXCEPTIONS 1
#else
#define UNIVERSAL_COMPILER_HAS_EXCEPTIONS 0
#endif

#ifndef UNIVERSAL_HAS_EXCEPTIONS
#define UNIVERSAL_HAS_EXCEPTIONS UNIVERSAL_COMPILER_HAS_EXCEPTIONS
#endif

#if UNIVERSAL_HAS_EXCEPTIONS && !UNIVERSAL_COMPILER_HAS_EXCEPTIONS
#error "UNIVERSAL_HAS_EXCEPTIONS is set to 1, but this build has exceptions disabled (-fno-exceptions)"
#endif

// A number system asked to throw in a build that cannot. Without this check the build fails
// with a compiler diagnostic deep inside that system's arithmetic instead. These macros are set
// by the application, before it includes the library, so they are visible here.
#if !UNIVERSAL_HAS_EXCEPTIONS
#if defined(AREAL_THROW_ARITHMETIC_EXCEPTION) && (AREAL_THROW_ARITHMETIC_EXCEPTION + 0)
#error "AREAL_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(BFLOAT_THROW_ARITHMETIC_EXCEPTION) && (BFLOAT_THROW_ARITHMETIC_EXCEPTION + 0)
#error "BFLOAT_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(BISECTION_THROW_ARITHMETIC_EXCEPTION) && (BISECTION_THROW_ARITHMETIC_EXCEPTION + 0)
#error "BISECTION_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(BITBLOCK_THROW_ARITHMETIC_EXCEPTION) && (BITBLOCK_THROW_ARITHMETIC_EXCEPTION + 0)
#error "BITBLOCK_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION) && (BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION + 0)
#error "BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(BLOCKDECIMAL_THROW_ARITHMETIC_EXCEPTION) && (BLOCKDECIMAL_THROW_ARITHMETIC_EXCEPTION + 0)
#error "BLOCKDECIMAL_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(BLOCKTRIPLE_THROW_ARITHMETIC_EXCEPTION) && (BLOCKTRIPLE_THROW_ARITHMETIC_EXCEPTION + 0)
#error "BLOCKTRIPLE_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(BPOSIT_THROW_ARITHMETIC_EXCEPTION) && (BPOSIT_THROW_ARITHMETIC_EXCEPTION + 0)
#error "BPOSIT_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(CFLOAT_THROW_ARITHMETIC_EXCEPTION) && (CFLOAT_THROW_ARITHMETIC_EXCEPTION + 0)
#error "CFLOAT_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(DBNS_THROW_ARITHMETIC_EXCEPTION) && (DBNS_THROW_ARITHMETIC_EXCEPTION + 0)
#error "DBNS_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(DD_CASCADE_THROW_ARITHMETIC_EXCEPTION) && (DD_CASCADE_THROW_ARITHMETIC_EXCEPTION + 0)
#error "DD_CASCADE_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(DFIXPNT_THROW_ARITHMETIC_EXCEPTION) && (DFIXPNT_THROW_ARITHMETIC_EXCEPTION + 0)
#error "DFIXPNT_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(DFLOAT_THROW_ARITHMETIC_EXCEPTION) && (DFLOAT_THROW_ARITHMETIC_EXCEPTION + 0)
#error "DFLOAT_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION) && (DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION + 0)
#error "DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(E8M0_THROW_ARITHMETIC_EXCEPTION) && (E8M0_THROW_ARITHMETIC_EXCEPTION + 0)
#error "E8M0_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(EDECIMAL_THROW_ARITHMETIC_EXCEPTION) && (EDECIMAL_THROW_ARITHMETIC_EXCEPTION + 0)
#error "EDECIMAL_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(EDECIMAL_THROW_DIVIDE_BY_ZERO) && (EDECIMAL_THROW_DIVIDE_BY_ZERO + 0)
#error "EDECIMAL_THROW_DIVIDE_BY_ZERO is set to 1, but this build has exceptions disabled"
#endif
#if defined(EFLOAT_THROW_ARITHMETIC_EXCEPTION) && (EFLOAT_THROW_ARITHMETIC_EXCEPTION + 0)
#error "EFLOAT_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(EINTEGER_THROW_ARITHMETIC_EXCEPTION) && (EINTEGER_THROW_ARITHMETIC_EXCEPTION + 0)
#error "EINTEGER_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(EINTEGER_THROW_DIVIDE_BY_ZERO) && (EINTEGER_THROW_DIVIDE_BY_ZERO + 0)
#error "EINTEGER_THROW_DIVIDE_BY_ZERO is set to 1, but this build has exceptions disabled"
#endif
#if defined(ELREAL_THROW_ARITHMETIC_EXCEPTION) && (ELREAL_THROW_ARITHMETIC_EXCEPTION + 0)
#error "ELREAL_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(ERATIONAL_THROW_ARITHMETIC_EXCEPTION) && (ERATIONAL_THROW_ARITHMETIC_EXCEPTION + 0)
#error "ERATIONAL_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(ERATIONAL_THROW_DIVIDE_BY_ZERO) && (ERATIONAL_THROW_DIVIDE_BY_ZERO + 0)
#error "ERATIONAL_THROW_DIVIDE_BY_ZERO is set to 1, but this build has exceptions disabled"
#endif
#if defined(EREAL_THROW_ARITHMETIC_EXCEPTION) && (EREAL_THROW_ARITHMETIC_EXCEPTION + 0)
#error "EREAL_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(FAITHFUL_THROW_ARITHMETIC_EXCEPTION) && (FAITHFUL_THROW_ARITHMETIC_EXCEPTION + 0)
#error "FAITHFUL_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(FIXPNT_THROW_ARITHMETIC_EXCEPTION) && (FIXPNT_THROW_ARITHMETIC_EXCEPTION + 0)
#error "FIXPNT_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(FIXPNT_THROW_DIVIDE_BY_ZERO) && (FIXPNT_THROW_DIVIDE_BY_ZERO + 0)
#error "FIXPNT_THROW_DIVIDE_BY_ZERO is set to 1, but this build has exceptions disabled"
#endif
#if defined(HFLOAT_THROW_ARITHMETIC_EXCEPTION) && (HFLOAT_THROW_ARITHMETIC_EXCEPTION + 0)
#error "HFLOAT_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(INTEGER_THROW_ARITHMETIC_EXCEPTION) && (INTEGER_THROW_ARITHMETIC_EXCEPTION + 0)
#error "INTEGER_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(INTEGER_THROW_DIVIDE_BY_ZERO) && (INTEGER_THROW_DIVIDE_BY_ZERO + 0)
#error "INTEGER_THROW_DIVIDE_BY_ZERO is set to 1, but this build has exceptions disabled"
#endif
#if defined(INTERVAL_THROW_ARITHMETIC_EXCEPTION) && (INTERVAL_THROW_ARITHMETIC_EXCEPTION + 0)
#error "INTERVAL_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(LINDEX_THROW_ARITHMETIC_EXCEPTION) && (LINDEX_THROW_ARITHMETIC_EXCEPTION + 0)
#error "LINDEX_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(LNS_THROW_ARITHMETIC_EXCEPTION) && (LNS_THROW_ARITHMETIC_EXCEPTION + 0)
#error "LNS_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(MICROFLOAT_THROW_ARITHMETIC_EXCEPTION) && (MICROFLOAT_THROW_ARITHMETIC_EXCEPTION + 0)
#error "MICROFLOAT_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(MXFLOAT_THROW_ARITHMETIC_EXCEPTION) && (MXFLOAT_THROW_ARITHMETIC_EXCEPTION + 0)
#error "MXFLOAT_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(NVBLOCK_THROW_ARITHMETIC_EXCEPTION) && (NVBLOCK_THROW_ARITHMETIC_EXCEPTION + 0)
#error "NVBLOCK_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(POSITIONAL_THROW_ARITHMETIC_EXCEPTION) && (POSITIONAL_THROW_ARITHMETIC_EXCEPTION + 0)
#error "POSITIONAL_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(POSIT_THROW_ARITHMETIC_EXCEPTION) && (POSIT_THROW_ARITHMETIC_EXCEPTION + 0)
#error "POSIT_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(POXEL_THROW_ARITHMETIC_EXCEPTION) && (POXEL_THROW_ARITHMETIC_EXCEPTION + 0)
#error "POXEL_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(QD_CASCADE_THROW_ARITHMETIC_EXCEPTION) && (QD_CASCADE_THROW_ARITHMETIC_EXCEPTION + 0)
#error "QD_CASCADE_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION) && (QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION + 0)
#error "QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(QUIRE_THROW_ARITHMETIC_EXCEPTION) && (QUIRE_THROW_ARITHMETIC_EXCEPTION + 0)
#error "QUIRE_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(RATIONAL_THROW_ARITHMETIC_EXCEPTION) && (RATIONAL_THROW_ARITHMETIC_EXCEPTION + 0)
#error "RATIONAL_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(SORN_THROW_ARITHMETIC_EXCEPTION) && (SORN_THROW_ARITHMETIC_EXCEPTION + 0)
#error "SORN_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(TAKUM_THROW_ARITHMETIC_EXCEPTION) && (TAKUM_THROW_ARITHMETIC_EXCEPTION + 0)
#error "TAKUM_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(TD_CASCADE_THROW_ARITHMETIC_EXCEPTION) && (TD_CASCADE_THROW_ARITHMETIC_EXCEPTION + 0)
#error "TD_CASCADE_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(UNUM_THROW_ARITHMETIC_EXCEPTION) && (UNUM_THROW_ARITHMETIC_EXCEPTION + 0)
#error "UNUM_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(VALID_THROW_ARITHMETIC_EXCEPTION) && (VALID_THROW_ARITHMETIC_EXCEPTION + 0)
#error "VALID_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(VALUE_THROW_ARITHMETIC_EXCEPTION) && (VALUE_THROW_ARITHMETIC_EXCEPTION + 0)
#error "VALUE_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#if defined(ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION) && (ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION + 0)
#error "ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION is set to 1, but this build has exceptions disabled"
#endif
#endif // !UNIVERSAL_HAS_EXCEPTIONS

namespace sw { namespace universal {

// throw e, or abort when this build cannot throw
template<typename Exception>
[[noreturn]] constexpr void throw_or_abort(Exception&& e) {
#if UNIVERSAL_HAS_EXCEPTIONS
	throw std::forward<Exception>(e);
#else
	(void)e;
	std::abort();
#endif
}

}} // namespace sw::universal
