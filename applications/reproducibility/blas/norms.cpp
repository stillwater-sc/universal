// norms.cpp: example program showing different norms that use the quire for reproducible linear algebra
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
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
#pragma warning(disable : 5045) // warning C5045: Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified 
#endif 

// enable the following define to show the intermediate steps in the fused-dot product
// #define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_MUL
#define QUIRE_TRACE_ADD
// configure posit environment using fast posits
#define POSIT_FAST_POSIT_8_0 1
#define POSIT_FAST_POSIT_16_1 1
#define POSIT_FAST_POSIT_32_2 1
#define POSIT_FAST_POSIT_64_3 0  // TODO
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
// Stillwater BLAS library
#include <blas/blas.hpp>

template<typename Vector>
void PrintProducts(const Vector& a, const Vector& b) {
	constexpr size_t nbits = Vector::value_type::nbits;
	constexpr size_t es = Vector::value_type::es;
	sw::universal::quire<nbits, es> q(0);
	for (size_t i = 0; i < a.size(); ++i) {
		q += sw::universal::quire_mul(a[i], b[i]);
		std::cout << a[i] << " * " << b[i] << " = " << a[i] * b[i] << std::endl << "quire " << q << std::endl;
	}
	typename Vector::value_type sum;
	sw::universal::convert(q.to_value(), sum);     // one and only rounding step of the fused-dot product
	std::cout << "fdp result " << sum << std::endl;
}

template<typename ResultScalar, typename RefScalar>
void reportOnCatastrophicCancellation(const std::string& type, const ResultScalar& v, const RefScalar& ref) {
	constexpr size_t COLUMN_WIDTH = 15;
	std::cout << type << std::setw(COLUMN_WIDTH) << v << (v == ref ? " <----- PASS" : " <-----      FAIL") << '\n';
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc == 1) std::cout << argv[0] << '\n';

	using Scalar = posit<16, 2>;
	// generate an interesting vector
	blas::vector<Scalar> v = { 1.0, 2.0, 3.0, 4.0, 5.0 };
	for (int p = 1; p < 10; ++p) {
		std::cout << "L" << p << "-norm            : " << norm(v, p) << '\n';
	}
	std::cout << "Linf-norm          : " << normLinf(v) << '\n';

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
