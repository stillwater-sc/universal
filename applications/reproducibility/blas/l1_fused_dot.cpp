// l1_fused_dot.cpp: example program showing a fused-dot product for error free linear algebra
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
#include <universal/number/posit1/posit1.hpp>
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
	using namespace sw::numeric::containers;
	using namespace sw::blas;

	if (argc == 1) std::cout << argv[0] << '\n';

	// generate an interesting vector x with 0.5 ULP round-off errors in each product
	// that the fused-dot product will be able to resolve
	// by progressively adding smaller values, a regular dot product loses these bits due to canceleation.
	// but a fused dot product leveraging a quire will be able to resolve these.
	//float eps      = std::numeric_limits<float>::epsilon();
	//float epsminus = 1.0f - eps;
	//float epsplus  = 1.0f + eps;

	std::streamsize prec = std::cout.precision();
	std::cout << std::setprecision(17);
	
	{
		using Scalar = float;
		using Vector = vector<Scalar>;
		Scalar a1 = 3.2e8, a2 = 1, a3 = -1, a4 = 8e7;
		Scalar b1 = 4.0e7, b2 = 1, b3 = -1, b4 = -1.6e8;
		Vector a = { a1, a2, a3, a4 };
		Vector b = { b1, b2, b3, b4 };

		std::cout << "a: " << a << '\n';
		std::cout << "b: " << b << '\n';

		std::cout << "\n\n";
		reportOnCatastrophicCancellation("IEEE float   BLAS dot(x,y)  : ", dot(a,b), 2);
	}

	{
		using Scalar = double;
		using Vector = vector<Scalar>;
		Scalar a1 = 3.2e8, a2 = 1, a3 = -1, a4 = 8e7;
		Scalar b1 = 4.0e7, b2 = 1, b3 = -1, b4 = -1.6e8;
		Vector a = { a1, a2, a3, a4 };
		Vector b = { b1, b2, b3, b4 };

		reportOnCatastrophicCancellation("IEEE double  BLAS dot(x,y)  : ", dot(a, b), 2);
	}

	{
		// a little verbose but enabling different precisions to be injected
		// float, double, long double
		// so that you can convince yourself that this is a property of posits and quires
		// and not some input precision shenanigans. The magic is all in the quire
		// accumulating UNROUNDED multiplies: that gives you in affect double the 
		// fraction bits.

		using Real = float;
		Real a1 = 3.2e8, a2 = 1, a3 = -1, a4 = 8e7;
		Real b1 = 4.0e7, b2 = 1, b3 = -1, b4 = -1.6e8;

#ifdef LATER
		{
			using Scalar = posit<8, 0>;
			vector<Scalar> x = { a1, a2, a3, a4 };
			vector<Scalar> y = { b1, b2, b3, b4 };

			reportOnCatastrophicCancellation("posit< 8,0> fused dot(x,y)  : ", fdp(x, y), 2);
		}
		{
			using Scalar = posit<8, 2>;
			vector<Scalar> x = { a1, a2, a3, a4 };
			vector<Scalar> y = { b1, b2, b3, b4 };

			reportOnCatastrophicCancellation("posit< 8,2> fused dot(x,y)  : ", fdp(x, y), 2);
		}
		{
			using Scalar = posit<8, 3>;
			vector<Scalar> x = { a1, a2, a3, a4 };
			vector<Scalar> y = { b1, b2, b3, b4 };

			reportOnCatastrophicCancellation("posit< 8,3> fused dot(x,y)  : ", fdp(x, y), 2);
		}
#endif
		{
			using Scalar = posit<16, 1>;
			std::vector<Scalar> x = { a1, a2, a3, a4 };
			std::vector<Scalar> y = { b1, b2, b3, b4 };

			reportOnCatastrophicCancellation("posit<16,1> fused dot(x,y)  : " , fdp(x, y), 2);
		}
		{
			using Scalar = posit<16, 2>;
			std::vector<Scalar> x = { a1, a2, a3, a4 };
			std::vector<Scalar> y = { b1, b2, b3, b4 };

			reportOnCatastrophicCancellation("posit<16,2> fused dot(x,y)  : ", fdp(x, y), 2);
		}
		{
			using Scalar = posit<32, 2>;
			std::vector<Scalar> x = { a1, a2, a3, a4 };
			std::vector<Scalar> y = { b1, b2, b3, b4 };

			reportOnCatastrophicCancellation("posit<32,2> fused dot(x,y)  : ", fdp(x, y), 2);
			//PrintProducts(x, y);
		}
		{
			using Scalar = posit<64, 1>;
			std::vector<Scalar> x = { a1, a2, a3, a4 };
			std::vector<Scalar> y = { b1, b2, b3, b4 };

			reportOnCatastrophicCancellation("posit<64,1> fused dot(x,y)  : ", fdp(x, y), 2);
		}
		{
			using Scalar = posit<64, 0>;
			std::vector<Scalar> x = { a1, a2, a3, a4 };
			std::vector<Scalar> y = { b1, b2, b3, b4 };

			reportOnCatastrophicCancellation("posit<64,0> fused dot(x,y)  : ", fdp(x, y), 2);
		}

		{
			using Scalar = posit<32, 1>;
			std::vector<Scalar> x = { a1, a2, a3, a4 };
			std::vector<Scalar> y = { b1, b2, b3, b4 };

			reportOnCatastrophicCancellation("posit<32,1> fused dot(x,y)  : ", fdp(x, y), 2);

			std::cout << "Reason why posit<32,1> fails\n";
			PrintProducts(x, y);
			std::cout << "Cannot represent integer value " << a1 << " != " << x[0] << '\n';
//			std::cout << "Cannot represent integer value " << b1 << " != " << y[0] << endl;
			std::cout << "Product is " << a1*b1 << " but quire_mul approximation yields " << quire_mul(x[0],y[0]) << '\n';
			std::cout << "Cannot represent integer value " << a4 << " != " << x[3] << '\n';
			std::cout << "Cannot represent integer value " << b4 << " != " << y[3] << '\n';
			std::cout << "Product is " << a4*b4 << " but quire_mul approximation yields " << quire_mul(x[3],y[3]) << '\n';
		}

		std::cout << std::setprecision(prec);
	}

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
