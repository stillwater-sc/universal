#pragma once

// compiler_directives.hpp: 
#if defined(_MSC_VER)
#pragma warning(disable : 4514) // unreferenced function is removed
#pragma warning(disable : 4515) // unreferenced inline function has been removed
#pragma warning(disable : 4710) // function is not inlined
#pragma warning(disable : 4820) // bytes padding added after data member

// this is a good warning to catch Universal conditional compilation errors
//#pragma warning(disable : 4688)  // warning C4668: '__STDC_WANT_SECURE_LIB__' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'

#pragma warning(disable : 5045) // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified

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