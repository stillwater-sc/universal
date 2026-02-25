// common.hpp : include file for common system include files,
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
#include <stdio.h>
//#include <tchar.h>
#include <cstdint>	// uint8_t, etc.
#include <cmath>	// for frexp/frexpf and std::fma
#include <cfenv>	// feclearexcept/fetestexcept

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#if defined(__GNUC__)
#if __GNUC__ < 5
#define hexfloat     scientific
#define defaultfloat scientific
#endif
#endif

