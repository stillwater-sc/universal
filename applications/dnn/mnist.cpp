// mnist.cpp: example program showing a mixed-precision LeNet-5 DNN
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the cfloat and lns environment
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/dnn/dnn.hpp>

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = false;
	using WeightType = cfloat<8, 2, std::uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using ActivationType = float;
	dnn::dnn<float> dnn;

	auto denseLayer = dnn::CreateDenseLayer<WeightType, ActivationType>(10ul, dnn::Activation::ReLU);
	std::cout << "First Dense Layer : " << denseLayer << '\n';

	dnn.addLayer(denseLayer);

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
