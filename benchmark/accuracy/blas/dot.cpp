// dot.cpp: accuracy/precision measurement of mixed-precision dot product
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifdef _MSC_VER
#pragma warning(disable : 4514) // warning C4514: 'std::complex<float>::complex': unreferenced inline function has been removed
#pragma warning(disable : 4571) // warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
#pragma warning(disable : 4625) // warning C4625: 'std::moneypunct<char,true>': copy constructor was implicitly defined as deleted
#pragma warning(disable : 4626) // warning C4626: 'std::codecvt_base': assignment operator was implicitly defined as deleted
#pragma warning(disable : 4710) // warning C4710: 'int swprintf_s(wchar_t *const ,const size_t,const wchar_t *const ,...)': function not inlined
#pragma warning(disable : 4774) // warning C4774: 'sprintf_s' : format string expected in argument 3 is not a string literal
#pragma warning(disable : 4820) // warning C4820: 'std::_Mpunct<_Elem>': '4' bytes padding added after data member 'std::_Mpunct<_Elem>::_Kseparator'
#pragma warning(disable : 5026) // warning C5026 : 'std::_Generic_error_category' : move constructor was implicitly defined as deleted
#pragma warning(disable : 5027) // warning C5027 : 'std::_Generic_error_category' : move assignment operator was implicitly defined as deleted
#endif 

// enable the following define to show the intermediate steps in the fused-dot product
// #define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_MUL
#define QUIRE_TRACE_ADD
// configure posit environment using fast posits
#define POSIT_FAST_POSIT_8_0 1
#define POSIT_FAST_POSIT_16_1 1
#define POSIT_FAST_POSIT_32_2 1
#define POSIT_FAST_POSIT_64_3 0  // TODO
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/number/decimal/decimal.hpp>
#include <universal/blas/blas.hpp>

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::streamsize prec = std::cout.precision();
	std::cout << std::setprecision(17);
	
	{
		using Scalar = decimal;
		using Vector = sw::universal::blas::vector<Scalar>;
//		Scalar a1 = 3.2e8, a2 = 1, a3 = -1, a4 = 8e7;             // TODO: <--- bug conversion from double
//		Scalar b1 = 4.0e7, b2 = 1, b3 = -1, b4 = -1.6e8;
		Scalar a1 = 320'000'000, a2 = 1, a3 = -1, a4 =   80'000'000;
		Scalar b1 =  40'000'000, b2 = 1, b3 = -1, b4 = -160'000'000;
		Vector a = { a1, a2, a3, a4 };
		Vector b = { b1, b2, b3, b4 };

		std::cout << "a: " << a << '\n';
		std::cout << "b: " << b << '\n';

		std::cout << "\n\n";
		decimal v = dot(a, b);
		std::cout << v << (v == 2 ? " <----- PASS\n" : " <-----      FAIL\n");
	}

	std::cout << std::setprecision(prec);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
