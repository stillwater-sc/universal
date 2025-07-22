// fit_sin.cpp: fit y = sin(x) with a third order polynomial using gradient descent
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the cfloat and lns environment
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>
#include <blas/blas.hpp>
#include <universal/dnn/dnn.hpp>
#include <math/constants/double_constants.hpp>

/*
We will use a problem of fitting the function y=sin(x) with a third order polynomial as our example.
The network will have four parameters (a, b, c, d), and will be trained with gradient descent to fit random data
by minimizing the Euclidean distance between the network output and the true output.

In the same directory there is a graphic, sin-function-fit.png that graphs the resulting polynomial fit.
 */

template<typename Scalar>
void SinFunctionFit(int iterations = 501) {
	using namespace sw::universal;

	constexpr int nrSamples = 1024;

	using Vector = blas::vector<Scalar>;

	Vector x(nrSamples), y(nrSamples), y_pred(nrSamples);
	// create linear samples between -pi and pi
	x = blas::linspace(-d_pi, d_pi, nrSamples);  // we create a vector<double> due to d_pi and then assign to vector<Scalar>
	y = blas::sin(x);

	// model parameters
	Scalar a{ 0.123 }, b{ 0.435 }, c{ 0.586 }, d{ 0.295 }; // initialize model weights with random data
	auto x1(x);
	auto x2(x1); x2 *= x;
	auto x3(x2); x3 *= x;
	Vector av(nrSamples);
	av = a;

	std::cout << "Sin function fit using a third order polynomial with Scalar type " << type_tag(a) << '\n';

	Scalar learning_rate = 1e-6;
	for (int r = 0; r < iterations; ++r) {
		// forward pass
		// y = a + bx + cx^2 + dx^3
		auto bx(x1); bx *= b;
		auto cxx(x2); cxx *= c;
		auto dxxx(x3); dxxx *= d;
		y_pred = av + bx + cxx + dxxx;

		// compute and print loss function
		Scalar loss = (blas::square(y_pred - y)).sum();
		if (r % 100 == 0) {
			std::cout << "[ " << std::setw(4) << r << "] : " << loss << '\n';
		}

		// backward pass
		Vector grad_y_pred = Scalar(2.0) * (y_pred - y);
		Scalar grad_a = grad_y_pred.sum();
		Vector grad_y_times(grad_y_pred);
		grad_y_times *= x1;
		Scalar grad_b = grad_y_times.sum();
		grad_y_times = grad_y_pred;
		grad_y_times *= x2;
		Scalar grad_c = grad_y_times.sum();
		grad_y_times = grad_y_pred;
		grad_y_times *= x3;
		Scalar grad_d = grad_y_times.sum();

		// update weights
		a -= (learning_rate * grad_a);
		b -= (learning_rate * grad_b);
		c -= (learning_rate * grad_c);
		d -= (learning_rate * grad_d);
	}

	std::cout << "Result : y = " << a << " + " << b << "x + " << c << "x^2 + " << d << "x^3\n";
}

#define MANUAL_TESTING 0

int main()
try {
	using namespace sw::universal;

#if MANUAL_TESTING
	int iterations{ 2000 };
#else
	int iterations{ 1 };
#endif

	using bf16 = cfloat<16, 8, uint16_t, true, true, false>;
	SinFunctionFit<float>(iterations);    // Result : y = -0.2031700 + 0.800356x + -0.0207303x^2 + -0.0852961x^3:  loss = 13.1245
	SinFunctionFit<fp32>(iterations);     // Result : y = -0.2031700 + 0.800356x + -0.0207303x^2 + -0.0852961x^3:  loss = 13.1245
	SinFunctionFit<bf16>(iterations);     // Result : y =  0.0190430 + 0.396484x + -0.0191650x^2 + -0.0280762x^3:  loss = 64.0000
	SinFunctionFit<fp16>(iterations);     // Result : y =    nan     +   nanx    +     nanx^2    +    nanx^3    :  loss = NaN

	// hypothesis: the c and d terms are squares and cubes and they need a lot of dynamic range
	// we can pick a posit with saturating behavior and large dynamic range to check
	using p16_2 = posit<16, 2>;
	using p16_3 = posit<16, 3>;
	using p16_4 = posit<16, 4>;
	SinFunctionFit<p16_2>(iterations);    // Result : y = -0.2219240 + 0.743164x + -0.0205994x^2 + -0.0772095x^3:  loss = 17.3125
	SinFunctionFit<p16_3>(iterations);    // Result : y = -0.2207030 + 0.627930x + -0.0206146x^2 + -0.0608521x^3:  loss = 38.8125
	SinFunctionFit<p16_4>(iterations);    // Result : y = -0.1250000 + 0.500000x + -0.0204468x^2 + -0.0426636x^3:  loss = 80.25

	// logarithmic number systems also have large dynamic range
	using l16_4 = lns<16, 4, uint16_t>;    // large dynamic range, low precision
	using l16_8 = lns<16, 8, uint16_t>;    // medium dynamic range, medium precision
	using l16_12 = lns<16, 12, uint16_t>;  // low dynamic range, high precision
	using l16_14 = lns<16, 14, uint16_t>;
	SinFunctionFit<l16_4>(iterations);    // Result : y =  0.1250000 + 0.439063x +  0.5452540x^2 + -0.0405262x^3:  loss = 1386.76
	SinFunctionFit<l16_8>(iterations);    // Result : y = -0.0837292 + 0.395063x + -0.0201537x^2 + -0.0280423x^3:  loss = 119.299
	SinFunctionFit<l16_12>(iterations);   // Result : y =  0.1230070 + 0.434995x +  0.5860130x^2 +  0.2949960x^3:  loss = 15.9973
	SinFunctionFit<l16_14>(iterations);   // Result : y =  0         + 0x        +  0.585988x^2  +  0x^3        :  loss = 1.99992


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
