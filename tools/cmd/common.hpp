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

#include <stdio.h>
//#include <tchar.h>
#include <cstdint>	// uint8_t, etc.
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>        // for frexp/frexpf
