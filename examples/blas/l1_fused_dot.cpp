// l1_fused_dot.cpp: example program showing a fused-dot product for error free linear algebra
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the following define to show the intermediate steps in the fused-dot product
// #define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_MUL
#define QUIRE_TRACE_ADD
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>

template<typename Ty>
Ty minValue(const std::vector<Ty>& samples) {
	typename std::vector<Ty>::const_iterator it = min_element(samples.begin(), samples.end());
	return *it;
}
template<typename Ty>
Ty maxValue(const std::vector<Ty>& samples) {
	typename std::vector<Ty>::const_iterator it = max_element(samples.begin(), samples.end());
	return *it;
}

template<typename Ty>
void PrintVector(std::ostream& ostr, const std::string& name, const std::vector<Ty>& v) {
	size_t d = v.size();
	ostr << "Vector: " << name << " is of size " << d << " elements" << std::endl;
	std::streamsize prec = ostr.precision();
	ostr << std::setprecision(17);
	for (size_t j = 0; j<d; ++j) std::cout << std::setw(20) << v[j] << " ";
	ostr << std::setprecision(prec) << std::endl;
}

template<typename Vector>
void PrintProducts(const Vector& a, const Vector& b) {
	constexpr size_t nbits = Vector::value_type::nbits;
	constexpr size_t es = Vector::value_type::es;
	sw::unum::quire<nbits, es> q = 0;
	for (size_t i = 0; i < a.size(); ++i) {
		q += sw::unum::quire_mul(a[i], b[i]);
		std::cout << a[i] << " * " << b[i] << " = " << a[i] * b[i] << std::endl << "quire " << q << std::endl;
	}
	typename Vector::value_type sum;
	sw::unum::convert(q.to_value(), sum);     // one and only rounding step of the fused-dot product
	std::cout << "fdp result " << sum << std::endl;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	//constexpr size_t nbits = 16;
	//constexpr size_t es = 1;
	//constexpr size_t capacity = 6;   // 2^3 accumulations of maxpos^2
	//constexpr size_t vecSizePwr = 5;
	//constexpr size_t vecSize = (size_t(1) << vecSizePwr);

	// generate an interesting vector x with 0.5 ULP round-off errors in each product
	// that the fused-dot product will be able to resolve
	// by progressively adding smaller values, a regular dot product loses these bits due to canceleation.
	// but a fused dot product leveraging a quire will be able to resolve these.
	//float eps      = std::numeric_limits<float>::epsilon();
	//float epsminus = 1.0f - eps;
	//float epsplus  = 1.0f + eps;

	std::streamsize prec = cout.precision();
	cout << setprecision(17);


	{
		using IEEEType = float;
		IEEEType a1 = 3.2e8, a2 = 1, a3 = -1, a4 = 8e7;
		IEEEType b1 = 4.0e7, b2 = 1, b3 = -1, b4 = -1.6e8;
		vector<IEEEType> xieee = { a1, a2, a3, a4 };
		vector<IEEEType> yieee = { b1, b2, b3, b4 };

		PrintVector(cout, "a: ", xieee);
		PrintVector(cout, "b: ", yieee);

		cout << endl << endl;
		cout << "IEEE float   BLAS dot(x,y)  : " << dot(xieee.size(), xieee, 1, yieee, 1) << "           <----- correct answer is 2" << endl;
	}

	{
		using IEEEType = double;
		IEEEType a1 = 3.2e8, a2 = 1, a3 = -1, a4 = 8e7;
		IEEEType b1 = 4.0e7, b2 = 1, b3 = -1, b4 = -1.6e8;
		vector<IEEEType> xieee = { a1, a2, a3, a4 };
		vector<IEEEType> yieee = { b1, b2, b3, b4 };

		cout << "IEEE double  BLAS dot(x,y)  : " << dot(xieee.size(), xieee, 1, yieee, 1) << "           <----- correct answer is 2" << endl;
	}

	{
		// a little verbose but enabling different precisions to be injected
		// float, double, long double
		// so that you can convince yourself that this is a property of posits and quires
		// and not some input precision shenanigans. The magic is all in the quire
		// accumulating UNROUNDED multiplies: that gives you in affect double the 
		// fraction bits.
		using IEEEType = float;
		IEEEType a1 = 3.2e8, a2 = 1, a3 = -1, a4 = 8e7;
		IEEEType b1 = 4.0e7, b2 = 1, b3 = -1, b4 = -1.6e8;

		{
			using PositType = posit<8, 3>;
			vector<PositType> x = { a1, a2, a3, a4 };
			vector<PositType> y = { b1, b2, b3, b4 };

			cout << "posit< 8,3> fused dot(x,y)  : " << fdp(x, y) << "           <----- correct answer is 2" << endl;
		}
		{
			using PositType = posit<16, 2>;
			vector<PositType> x = { a1, a2, a3, a4 };
			vector<PositType> y = { b1, b2, b3, b4 };

			cout << "posit<16,2> fused dot(x,y)  : " << fdp(x, y) << "           <----- correct answer is 2" << endl;
		}
		{
			using PositType = posit<32, 2>;
			vector<PositType> x = { a1, a2, a3, a4 };
			vector<PositType> y = { b1, b2, b3, b4 };

			cout << "posit<32,2> fused dot(x,y)  : " << fdp(x, y) << "           <----- correct answer is 2" << endl;
			//PrintProducts(x, y);
		}
		{
			using PositType = posit<64, 1>;
			vector<PositType> x = { a1, a2, a3, a4 };
			vector<PositType> y = { b1, b2, b3, b4 };

			cout << "posit<64,1> fused dot(x,y)  : " << fdp(x, y) << "           <----- correct answer is 2" << endl;
		}
		{
			using PositType = posit<64, 0>;
			vector<PositType> x = { a1, a2, a3, a4 };
			vector<PositType> y = { b1, b2, b3, b4 };

			cout << "posit<64,0> fused dot(x,y)  : " << fdp(x, y) << "           <----- correct answer is 2" << endl;
		}

		{
			using PositType = posit<16, 1>;
			vector<PositType> x = { a1, a2, a3, a4 };
			vector<PositType> y = { b1, b2, b3, b4 };

			cout << "posit<16,1> fused dot(x,y)  : " << fdp(x, y) << "           <----- correct answer is 2" << endl;
		}
		{
			using PositType = posit<32, 1>;
			vector<PositType> x = { a1, a2, a3, a4 };
			vector<PositType> y = { b1, b2, b3, b4 };

			cout << "posit<32,1> fused dot(x,y)  : " << fdp(x, y) << "           <----- correct answer is 2" << endl;

			cout << "Reason why posit<32,1> fails\n";
			PrintProducts(x, y);
			cout << "Cannot represent integer value " << a1 << " != " << x[0] << endl;
//			cout << "Cannot represent integer value " << b1 << " != " << y[0] << endl;
			cout << "Product is " << a1*b1 << " but quire_mul approximation yields " << quire_mul(x[0],y[0]) << endl;
			cout << "Cannot represent integer value " << a4 << " != " << x[3] << endl;
			cout << "Cannot represent integer value " << b4 << " != " << y[3] << endl;
			cout << "Product is " << a4*b4 << " but quire_mul approximation yields " << quire_mul(x[3],y[3]) << endl;
		}

		cout << setprecision(prec);
	}

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
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
