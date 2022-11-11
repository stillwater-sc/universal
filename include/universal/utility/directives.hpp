#pragma once

// compiler_directives.hpp: 
#if defined(_MSC_VER)
#pragma warning(disable : 4514) // unreferenced function is removed
#pragma warning(disable : 4515) // unreferenced inline function has been removed
#pragma warning(disable : 4710) // function is not inlined
#pragma warning(disable : 4820) // bytes padding added after data member
#pragma warning(disable : 5262) // implicit fall-through occurs here
#pragma warning(disable : 5264) // 'const' variable is not used

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

namespace sw { namespace universal {

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

	void report_compiler_environment() {
#if defined(_MSC_VER) && (_MSC_VER >= 1300)
		std::cout << "Microsoft Visual C++: " << _MSC_VER << '\n';
		if constexpr (_MSC_VER == 1600) std::cout << "(Visual Studio 2010 version 10.0)\n";
		else if constexpr (_MSC_VER == 1700) std::cout << "(Visual Studio 2012 version 11.0)\n";
		else if constexpr (_MSC_VER == 1800) std::cout << "(Visual Studio 2013 version 12.0)\n";
		else if constexpr (_MSC_VER == 1900) std::cout << "(Visual Studio 2015 version 14.0)\n";
		else if constexpr (_MSC_VER == 1910) std::cout << "(Visual Studio 2017 version 15.0)\n";
		else if constexpr (_MSC_VER == 1911) std::cout << "(Visual Studio 2017 version 15.3)\n";
		else if constexpr (_MSC_VER == 1912) std::cout << "(Visual Studio 2017 version 15.5)\n";
		else if constexpr (_MSC_VER == 1913) std::cout << "(Visual Studio 2017 version 15.6)\n";
		else if constexpr (_MSC_VER == 1914) std::cout << "(Visual Studio 2017 version 15.7)\n";
		else if constexpr (_MSC_VER == 1915) std::cout << "(Visual Studio 2017 version 15.8)\n";
		else if constexpr (_MSC_VER == 1916) std::cout << "(Visual Studio 2017 version 15.9)\n";
		else if constexpr (_MSC_VER == 1920) std::cout << "(Visual Studio 2019 Version 16.0)\n";
		else if constexpr (_MSC_VER == 1921) std::cout << "(Visual Studio 2019 Version 16.1)\n";
		else if constexpr (_MSC_VER == 1922) std::cout << "(Visual Studio 2019 Version 16.2)\n";
		else if constexpr (_MSC_VER == 1923) std::cout << "(Visual Studio 2019 Version 16.3)\n";
		else if constexpr (_MSC_VER == 1924) std::cout << "(Visual Studio 2019 Version 16.4)\n";
		else if constexpr (_MSC_VER == 1925) std::cout << "(Visual Studio 2019 Version 16.5)\n";
		else if constexpr (_MSC_VER == 1926) std::cout << "(Visual Studio 2019 Version 16.6)\n";
		else if constexpr (_MSC_VER == 1927) std::cout << "(Visual Studio 2019 Version 16.7)\n";
		else if constexpr (_MSC_VER == 1928) std::cout << "(Visual Studio 2019 Version 16.9)\n";
		else if constexpr (_MSC_VER == 1929) std::cout << "(Visual Studio 2019 Version 16.11)\n";
		else std::cout << "unknown Microsoft Visual C++: " << _MSC_VER << '\n';

		std::cout << "__cplusplus: " << __cplusplus << '\n';

		/*
		_MSVC_LANG Defined as an integer literal that specifies the C++ language standard targeted by the compiler.
		 It's set only in code compiled as C++. The macro is the integer literal value 201402L by default,
		 or when the /std:c++14 compiler option is specified. The macro is set to 201703L if the /std:c++17
		 compiler option is specified. It's set to a higher, unspecified value when the /std:c++latest option
		 is specified. Otherwise, the macro is undefined.
		 The _MSVC_LANG macro and /std(Specify Language Standard Version) compiler options are available
		 beginning in Visual Studio 2015 Update 3.
		 */
		if constexpr (_MSVC_LANG == 201402l)      std::cout << "/std:c++14\n";
		else if constexpr (_MSVC_LANG == 201703l) std::cout << "/std:c++17\n";
		else if constexpr (_MSVC_LANG == 202004) std::cout << "/std:c++20\n";
		else if constexpr (_MSVC_LANG == 202004) std::cout << "/std:c++latest\n";
		else  std::cout << "_MSVC_LANG: " << _MSVC_LANG << '\n';

#else
		if constexpr (__cplusplus == 202003L) std::cout << "C++20\n";
		else if constexpr (__cplusplus == 201703L) std::cout << "C++17\n";
		else if constexpr (__cplusplus == 201402L) std::cout << "C++14\n";
		else if constexpr (__cplusplus == 201103L) std::cout << "C++11\n";
		else if constexpr (__cplusplus == 199711L) std::cout << "C++98\n";
		else std::cout << __cplusplus << " pre-standard C++\n";
#endif
	}

}} // namespace sw::universal