// cfloat.cpp: test vector generator for a classic float hardware ALU
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/hw/alu.hpp>
#include <universal/number/cfloat/cfloat.hpp>

#include <universal/verification/test_status.hpp> // ReportTestResult

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// fp<8,2> without subnormals, max-exponent values or saturation
	constexpr unsigned nbits = 8;
	constexpr unsigned es = 2;
	using fp8_2_nosubsupsat = cfloat<nbits, es, std::uint16_t, false, false, false>;

	using Real = fp8_2_nosubsupsat;

	if (argc != 4) {
		std::cerr << "Usage: hw_cfloat [add | sub | mul | div | sqrt] a b\n";
		std::cerr << "Example: hw_cfloat add 1.5 -1.5\n";
		return EXIT_SUCCESS;  // needed for regression test success
	}
	std::string op = argv[1];
	std::cout << "generating fp<8,4,nosub,nosup,nosat> test vectors for " << op;

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
