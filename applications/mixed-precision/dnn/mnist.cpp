// mnist.cpp: example program showing a mixed-precision LeNet-5 DNN
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the cfloat and lns environment
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/dnn/dnn.hpp>

int main()
try {
	using namespace sw::universal;
	using namespace sw::dnn;

	constexpr bool hasSubnormals = true;
	constexpr bool hasMaxExpValues = true;
	constexpr bool isSaturating = false;
	using WeightType = cfloat<8, 2, std::uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using ActivationType = lns<5, 2, std::uint8_t>;
	dnn<float> dnn("LeNet-5", 0.1f);

	unsigned N(1), C(3), H(224), W(224);
	auto convLayer1 = CreateConvolutionLayer<WeightType, ActivationType>(N, C, H, W, Activation::Tanh);
	std::cout << convLayer1 << '\n';
	dnn.addLayer(convLayer1);

	auto fcLayer = CreateFullyConnectedLayer<WeightType, ActivationType>(10ul, Activation::ReLU);
	std::cout << fcLayer << '\n';
	dnn.addLayer(fcLayer);

	std::cout << dnn << '\n';

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
