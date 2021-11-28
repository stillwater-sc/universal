// midpoint.cpp: example program to use C++20 <cmath> lerp and midpoint functions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>
#include <cmath>   // lerp and midpoint
#if (__cplusplus == 202003L) || (_MSVC_LANG == 202003L)
#include <numbers>    // high-precision numbers
#endif


// select the number systems we would like to compare
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

#include <cstddef>
#include <stdexcept>
#include <cstring>
#include <ostream>

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::cout << "high-precision constants\n";

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
	std::cout << "Microsoft Visual C++: " << _MSC_VER << '\n';
	/*
	MSVC++ 10.0  _MSC_VER == 1600 (Visual Studio 2010 version 10.0)
	MSVC++ 11.0  _MSC_VER == 1700 (Visual Studio 2012 version 11.0)
	MSVC++ 12.0  _MSC_VER == 1800 (Visual Studio 2013 version 12.0)
	MSVC++ 14.0  _MSC_VER == 1900 (Visual Studio 2015 version 14.0)
	MSVC++ 14.1  _MSC_VER == 1910 (Visual Studio 2017 version 15.0)
	MSVC++ 14.11 _MSC_VER == 1911 (Visual Studio 2017 version 15.3)
	MSVC++ 14.12 _MSC_VER == 1912 (Visual Studio 2017 version 15.5)
	MSVC++ 14.13 _MSC_VER == 1913 (Visual Studio 2017 version 15.6)
	MSVC++ 14.14 _MSC_VER == 1914 (Visual Studio 2017 version 15.7)
	MSVC++ 14.15 _MSC_VER == 1915 (Visual Studio 2017 version 15.8)
	MSVC++ 14.16 _MSC_VER == 1916 (Visual Studio 2017 version 15.9)
	MSVC++ 14.2  _MSC_VER == 1920 (Visual Studio 2019 Version 16.0)
	MSVC++ 14.21 _MSC_VER == 1921 (Visual Studio 2019 Version 16.1)
	MSVC++ 14.22 _MSC_VER == 1922 (Visual Studio 2019 Version 16.2)
	MSVC++ 14.23 _MSC_VER == 1923 (Visual Studio 2019 Version 16.3)
	MSVC++ 14.24 _MSC_VER == 1924 (Visual Studio 2019 Version 16.4)
	MSVC++ 14.25 _MSC_VER == 1925 (Visual Studio 2019 Version 16.5)
	MSVC++ 14.26 _MSC_VER == 1926 (Visual Studio 2019 Version 16.6)
	MSVC++ 14.27 _MSC_VER == 1927 (Visual Studio 2019 Version 16.7)
	*/
	if (_MSC_VER == 1600) std::cout << "(Visual Studio 2010 version 10.0)\n";
	else if (_MSC_VER == 1700) std::cout << "(Visual Studio 2012 version 11.0)\n";
	else if (_MSC_VER == 1800) std::cout << "(Visual Studio 2013 version 12.0)\n";
	else if (_MSC_VER == 1900) std::cout << "(Visual Studio 2015 version 14.0)\n";
	else if (_MSC_VER == 1910) std::cout << "(Visual Studio 2017 version 15.0)\n";
	else if (_MSC_VER == 1911) std::cout << "(Visual Studio 2017 version 15.3)\n";
	else if (_MSC_VER == 1912) std::cout << "(Visual Studio 2017 version 15.5)\n";
	else if (_MSC_VER == 1913) std::cout << "(Visual Studio 2017 version 15.6)\n";
	else if (_MSC_VER == 1914) std::cout << "(Visual Studio 2017 version 15.7)\n";
	else if (_MSC_VER == 1915) std::cout << "(Visual Studio 2017 version 15.8)\n";
	else if (_MSC_VER == 1916) std::cout << "(Visual Studio 2017 version 15.9)\n";
	else if (_MSC_VER == 1920) std::cout << "(Visual Studio 2019 Version 16.0)\n";
	else if (_MSC_VER == 1921) std::cout << "(Visual Studio 2019 Version 16.1)\n";
	else if (_MSC_VER == 1922) std::cout << "(Visual Studio 2019 Version 16.2)\n";
	else if (_MSC_VER == 1923) std::cout << "(Visual Studio 2019 Version 16.3)\n";
	else if (_MSC_VER == 1924) std::cout << "(Visual Studio 2019 Version 16.4)\n";
	else if (_MSC_VER == 1925) std::cout << "(Visual Studio 2019 Version 16.5)\n";
	else if (_MSC_VER == 1926) std::cout << "(Visual Studio 2019 Version 16.6)\n";
	else if (_MSC_VER == 1927) std::cout << "(Visual Studio 2019 Version 16.7)\n";
	else std::cout << "Microsoft Visual C++: " << _MSC_VER << '\n';

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
	if (_MSVC_LANG == 201402l)      std::cout << "/std:c++14\n";
	else if (_MSVC_LANG == 201703l) std::cout << "/std:c++17\n";
	else if (_MSVC_LANG == 201704l) std::cout << "/std:c++latest\n";
	else  std::cout << "_MSVC_LANG: " << _MSVC_LANG << '\n';

#else
	if (__cplusplus == 202003L) std::cout << "C++20\n";
	else if (__cplusplus == 201703L) std::cout << "C++17\n";
	else if (__cplusplus == 201402L) std::cout << "C++14\n";
	else if (__cplusplus == 201103L) std::cout << "C++11\n";
	else if (__cplusplus == 199711L) std::cout << "C++98\n";
	else std::cout << __cplusplus << " pre-standard C++\n";
#endif

#if MIXED
	using int32    = integer<32>;
	using fixpnt32 = fixpnt<32,16>;
	using posit32  = posit<32,2>;
	using areal32  = areal<32,8>;
	using lns32    = lns<32>;
#endif

	// check difficult midpoint and lerp operations on different number systems

	std::streamsize precision = std::cout.precision();

#if (__cplusplus == 202003L) || (_MSVC_LANG == 202003L)

	cout << std::setprecision(50);
	cout << "midpoint          " << std::midpoint(5,7) << endl;
	cout << "lerp              " << std::lerp(5,7,0.5f) << endl;

	float a=10.0f, b=20.0f;
 
   	 std::cout << "a=" << a << '\n'
              	   << "b=" << b << '\n'
                   << "mid point=" << std::lerp(a,b,0.5f) << '\n'
                   << std::boolalpha << (a == std::lerp(a,b,0.0f)) << '\n';
#endif

	 std::cout << std::setprecision(precision);
	 std::cout << std::endl;
	
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
