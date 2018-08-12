// common.hpp : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef WINDOWS
// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#include <SDKDDKVer.h>
#endif // WINDOWS

#include <cassert>
#include <cstdint>	// uint8_t, etc.
#include <cfloat>   // for FLT_MIN, etc. constants
// enable the mathematical constants in cmath
#define _USE_MATH_DEFINES
#include <cmath>        // for frexp/frexpf

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#if defined(__GNUC__)
#if __GNUC__ < 5
#define hexfloat     scientific
#define defaultfloat scientific
#endif
#endif
