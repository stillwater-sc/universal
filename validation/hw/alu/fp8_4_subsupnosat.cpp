// fp8_4_subsupnosat.cpp: test vector generator for a fp<8,4, subnormals, max-exponent values, non-saturating> hardware ALU
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/hw/alu.hpp>
#include <universal/number/cfloat/cfloat.hpp>

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// fp8_4_subsupnosat has subnormals and max-exponent values
	constexpr unsigned nbits = 8;
	constexpr unsigned es = 4;
	using fp8_4_subsupnosat = cfloat<nbits, es, std::uint16_t, true, true, false>;

	using Real = fp8_4_subsupnosat;

	if (!(argc == 2 || argc == 4)) {
		std::cerr << "Usage: hw_fp8_4_subsupnosat [add | sub | mul | div | sqrt] [a b]\n";
		std::cerr << "Example: hw_fp8_4_subsupnosat add 1.5 -1.5\n";
		return EXIT_SUCCESS;  // needed for regression test success
	}
	std::string op = argv[1];
	std::cout << "generating fp<8,4,sub, sup, nosat> test vectors for " << op;

	float fa, fb;
	if (argc == 4) {
		fa = atof(argv[2]);
		fb = atof(argv[3]);
		std::cout << " " << fa << ' ' << fb << '\n';
		ExecuteOp<Real>(op, fa, fb);
	}
	else {
		std::cout << '\n';
		// generate a verification table
		if (op != "sqrt") {
			GenerateBinaryOpTestVectors<Real>(std::cout, op);
		}
		else {
			GenerateUnaryOpTestVectors<Real>(std::cout, op);
		}
	}

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
