#pragma once

// compiler_directives.hpp: 
#if defined(_MSC_VER)
#pragma warning(disable : 4514) // unreferenced function is removed
#pragma warning(disable : 4515) // unreferenced inline function has been removed
#pragma warning(disable : 4710) // function is not inlined
#pragma warning(disable : 4820) // bytes padding added after data member

// this is a good warning to catch Universal library conditional compilation errors
//#pragma warning(disable : 4688)  warning C4668: 'LONG_DOUBLE_SUPPORT' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
// but this one warning is making life difficult
// warning C4668: '__STDC_WANT_SECURE_LIB__' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
// TODO: figure out what compiler flag configuration is causing this,
// but side step it in the mean time
#ifndef __STDC_WANT_SECURE_LIB__
#define __STDC_WANT_SECURE_LIB__ 1
#endif

#pragma warning(disable : 5045) // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified

#ifndef LONG_DOUBLE_SUPPORT
// this is too chatty
// #pragma message("MSVC does not have LONG_DOUBLE_SUPPORT")
#define LONG_DOUBLE_SUPPORT 0
#endif
#endif

#include <iostream>
#include <iomanip>
#include <string>
#include <typeinfo>

namespace sw::universal {

	/// <summary>
	/// print the cmd line if there is one or more parameters provided
	/// </summary>
	/// <param name="argc">number of arguments</param>
	/// <param name="argv">array of char* representing the arguments</param>
	void print_cmd_line(int argc, char* argv[]) {
		if (argc > 1) {
			for (int i = 0; i < argc; ++i) {
				std::cout << argv[i] << ' ';
			}
			std::cout << std::endl;
		}
	}
}