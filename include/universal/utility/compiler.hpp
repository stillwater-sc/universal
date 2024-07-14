#pragma once
// identify compiler
// 
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>




namespace sw { namespace universal {

	inline void report_compiler() {

#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */
std::string compiler_identifier("clang");

#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */

std::string compiler_identifier("icc");

#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */
std::string compiler_identifier("gcc");

#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */
std::string compiler_identifier("hpcc");

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */
std::string compiler_identifier("ibmcpp");

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */
std::string compiler_identifier("msvc");

#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */
std::string compiler_identifier("pgcpp");

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */
std::string compiler_identifier("suncc");

#endif
        
        std::cout << "compiler architecture: " << compiler_identifier << '\n';
        std::cout << "compiler language cfg: " << __cplusplus << '\n';
        
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
		else if constexpr (_MSC_VER == 1930) std::cout << "(Visual Studio 2022 Version 17.0 RTW)\n";
		else if constexpr (_MSC_VER == 1931) std::cout << "(Visual Studio 2022 Version 17.1)\n";
		else if constexpr (_MSC_VER == 1932) std::cout << "(Visual Studio 2022 Version 17.2)\n";
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
		std::cout << "_MSVC_LANG: " << _MSVC_LANG << '\n';
		if constexpr (_MSVC_LANG == 202002l)      std::cout << "/std:c++20\n";
		else if constexpr (_MSVC_LANG == 202004l) std::cout << "/std:c++20\n";
		else if constexpr (_MSVC_LANG == 201703l) std::cout << "/std:c++17\n";
		else if constexpr (_MSVC_LANG == 201402l) std::cout << "/std:c++14\n";
		else if constexpr (_MSVC_LANG == 202004)  std::cout << "/std:c++latest\n";
		else  std::cout << "_MSVC_LANG: " << _MSVC_LANG << '\n';

#else
		if constexpr (__cplusplus == 202002L) std::cout << "C++20\n";
		else if constexpr (__cplusplus == 201703L) std::cout << "C++17\n";
		else if constexpr (__cplusplus == 201402L) std::cout << "C++14\n";
		else if constexpr (__cplusplus == 201103L) std::cout << "C++11\n";
		else if constexpr (__cplusplus == 199711L) std::cout << "C++98\n";
		else std::cout << __cplusplus << " pre-standard C++\n";

#endif
	}

}} // namespace sw::universal
