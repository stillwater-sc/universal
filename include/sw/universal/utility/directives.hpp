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
// The single definition of LONG_DOUBLE_SUPPORT (#1534). utility/long_double.hpp used to define it
// as well, from a hand-maintained per-compiler table, and neither header could see the other's
// answer: whichever a translation unit included first won. Where they disagreed the compiler said
// so -- every RISC-V TU on main warned "LONG_DOUBLE_SUPPORT redefined", the table calling the type
// unsupported while the format reported IEEE binary128 (#1399).
//
// The question this answers is "does this build have a long double whose fields this library can
// read?", and the answer comes from the format the compiler reports, which is the ABI choice it
// made. The compiler only selects WHICH formats the layer implements for it:
//
//   MSVC      long double is double, and the long double paths do not compile there
//   RISC-V    IEEE binary128 only; no other format has a shape in the GNU layer (#1399)
//   GNU       x87, binary128 and double-double; with long double == double, ieee754_components
//             static_asserts on sizeof(long double) == 16, so that build has no shape either
//   clang     as GNU, plus the long double == double shape its own config carries
//
// "Has more precision than double" is a DIFFERENT question, and the sites that mean that one
// should ask it directly, as LDBL_MANT_DIG > DBL_MANT_DIG. Sorting the call sites by which
// question they ask is the next step of #1534.
#ifndef LONG_DOUBLE_SUPPORT
#include <cfloat>
#if defined(_MSC_VER)
#define LONG_DOUBLE_SUPPORT 0
#elif defined(__riscv) && (LDBL_MANT_DIG != 113)
#define LONG_DOUBLE_SUPPORT 0
#elif (defined(__GNUC__) || defined(__GNUG__)) && !defined(__clang__) && (LDBL_MANT_DIG == DBL_MANT_DIG)
#define LONG_DOUBLE_SUPPORT 0
#else
#define LONG_DOUBLE_SUPPORT 1
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