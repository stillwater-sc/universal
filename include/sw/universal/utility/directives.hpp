#pragma once
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

// compiler directives
#if defined(_MSC_VER)

// MSVC does not define M_PI and friends by default
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

// this is a good warning to catch conditional compilation errors
//#pragma warning(disable : 4688)  warning C4668: 'LONG_DOUBLE_SUPPORT' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
// but this one warning is making life difficult
// warning C4668: '__STDC_WANT_SECURE_LIB__' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
// TODO: figure out what compiler flag configuration is causing this,
// but side step it in the mean time
#ifndef __STDC_WANT_SECURE_LIB__
#define __STDC_WANT_SECURE_LIB__ 1
#endif

#endif // _MSC_VER

// ========== LONG_DOUBLE_SUPPORT detection ==========
// Many platforms alias `long double` to `double` (53-bit mantissa) rather than
// providing extended precision. Tests and APIs that rely on long double having
// extra headroom over double must guard with LONG_DOUBLE_SUPPORT.
//
//   1 -- long double has more precision than double (e.g., x86_64 Linux/macOS
//        80-bit x87 extended, PowerPC double-double, sparc64 IEEE quadruple).
//   0 -- long double is the same as double (MSVC always, plus most ARM,
//        RISC-V, Android NDK, and 32-bit x86 with -mlong-double-64).
//
// Detection uses <cfloat>'s LDBL_MANT_DIG / DBL_MANT_DIG which are required
// by the C/C++ standard and reflect the ABI choice the compiler made.
#ifndef LONG_DOUBLE_SUPPORT
#include <cfloat>
#if defined(_MSC_VER)
// MSVC always aliases long double to double regardless of /arch flags.
#define LONG_DOUBLE_SUPPORT 0
#elif defined(LDBL_MANT_DIG) && defined(DBL_MANT_DIG) && (LDBL_MANT_DIG > DBL_MANT_DIG)
#define LONG_DOUBLE_SUPPORT 1
#else
#define LONG_DOUBLE_SUPPORT 0
#endif
#endif // LONG_DOUBLE_SUPPORT

// ========== Compiler Configuration Messages ==========
// Macro for consistent compile-time messages across compilers
// Usage: UNIVERSAL_COMPILER_MESSAGE("Fast specialization of posit<8,0>")
//
// To enable these messages, define UNIVERSAL_VERBOSE_BUILD before including headers
// or add -DUNIVERSAL_VERBOSE_BUILD to compiler flags
//
// MSVC: Uses #pragma message for clean output
// GCC/Clang: Uses #warning directive for warning output
// Other: No-op

#ifdef UNIVERSAL_VERBOSE_BUILD
	#if defined(_MSC_VER)
		// MSVC: Use pragma message
		#define UNIVERSAL_COMPILER_MESSAGE(msg) __pragma(message("Universal: " msg))
	#elif defined(__GNUC__) || defined(__clang__)
		// GCC/Clang: Use #warning directive for clean output
		// The _Pragma trick allows us to use #warning from within a macro
		#define STRINGIZE_IMPL(x) #x
		#define STRINGIZE(x) STRINGIZE_IMPL(x)
		#define UNIVERSAL_COMPILER_MESSAGE(msg) _Pragma(STRINGIZE(GCC warning msg))
	#else
		// Other compilers: no-op
		#define UNIVERSAL_COMPILER_MESSAGE(msg)
	#endif
#else
	// Verbose build not enabled: no-op
	#define UNIVERSAL_COMPILER_MESSAGE(msg)
#endif

// ========== End Compiler Configuration Messages ==========