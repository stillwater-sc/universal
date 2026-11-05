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
// without exceptions, and utility/exceptions_check.hpp stops the build with an #error when one of
// them is set to 1 in a build that cannot throw.

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

// Reject a throw switch set to 1 in a build that cannot throw. The check is re-included by each
// number system after it defines its switches, since an application may define them after its
// first Universal include.
#include <universal/utility/exceptions_check.hpp>

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
