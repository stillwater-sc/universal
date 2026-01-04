#pragma once
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

// compiler directives  
#if defined(_MSC_VER)

#ifndef LONG_DOUBLE_SUPPORT
// this is too chatty
// #pragma message("MSVC does not have LONG_DOUBLE_SUPPORT")

// set the default to off
#define LONG_DOUBLE_SUPPORT 0
#endif // LONG_DOUBLE_SUPPORT

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