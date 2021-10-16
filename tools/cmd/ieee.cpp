// ieee.cpp: cli to show the sign/scale/fraction components of a 32b/64/128b IEEE floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <limits>
#include <universal/native/ieee754.hpp>
#include <universal/functions/isrepresentable.hpp>
#include <universal/internal/value/value>

std::string version_string(int a, int b, int c) {
	std::ostringstream ss;
	ss << a << '.' << b << '.' << c;
	return ss.str();
}

std::string report_compiler_version() {
#if defined(__clang__)
	/* Clang/LLVM. ---------------------------------------------- */
	return version_string(__clang_major__, __clang_minor__, __clang_patchlevel__);

#elif defined(__ICC) || defined(__INTEL_COMPILER)
	/* Intel ICC/ICPC. ------------------------------------------ */
	return std::string("Intel Compiler");

#elif defined(__GNUC__) || defined(__GNUG__)
	/* GNU GCC/G++. --------------------------------------------- */
	return version_string(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);

#elif defined(__HP_cc) || defined(__HP_aCC)
	/* Hewlett-Packard C/C++. ---------------------------------- */
	return std::string("Hewlett-Packard C/C++ compiler");

#elif defined(__IBMC__) || defined(__IBMCPP__)
	/* IBM XL C/C++. -------------------------------------------- */
	return std::string("IBM XL C/C++");

#elif defined(_MSC_VER)
	/* Microsoft Visual Studio. --------------------------------- */
	// Visual C++ compiler is 15.00.20706.01, the _MSC_FULL_VER will be 15002070601
	char version[16];
	sprintf(version, "%d", _MSC_FULL_VER);
	return std::string("Microsoft Visual Studio C++ compiler version ") + std::string(version);

#elif defined(__PGI)
	/* Portland Group PGCC/PGCPP. ------------------------------- */
	return std::string("Portland Group PGCC/PGCPP");

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
	/* Oracle Solaris Studio. ----------------------------------- */
	return std::string("Oracle Solaris Studio");
#endif
}

// receive a float and print the components of a IEEE float representations
int main(int argc, char** argv)
try {
	using namespace sw::universal::internal;

	// long double attributes
	constexpr int f_prec = std::numeric_limits<float>::max_digits10;
	constexpr int d_prec = std::numeric_limits<double>::max_digits10;
	constexpr int q_prec = std::numeric_limits<long double>::max_digits10;

	constexpr int f_fbits = std::numeric_limits<float>::digits - 1;
	constexpr int d_fbits = std::numeric_limits<double>::digits - 1;
	constexpr int q_fbits = std::numeric_limits<long double>::digits - 1;

	if (argc != 2) {
		std::cerr << "Show the truncated value and (sign/scale/fraction) components of different floating point types.\n";
		std::cerr << "Usage: ieee floating_point_value\n";
		std::cerr << "Example: ieee 0.03124999\n";
		std::cerr << "input value:                0.03124999\n";
		std::cerr << "      float:              0.0312499907 (+,-6,11111111111111111111011)\n";
		std::cerr << "     double:      0.031249989999999998 (+,-6,1111111111111111111101010100001100111000100011101110)\n";
		std::cerr << "long double:  0.0312499899999999983247 (+,-6,111111111111111111101001011110100011111111111110001111111001111)\n";

		return EXIT_SUCCESS;   // signal successful completion for ctest
	}

	std::streamsize old_precision = std::cout.precision();
	int width = q_prec + 4;

	long double q = std::stold(argv[1]);
	double d      = double(q);
	float f       = float(d);

	std::cout << "compiler              : " << report_compiler_version() << '\n';
	std::cout << "float precision       : " << f_fbits << " bits\n";
	std::cout << "double precision      : " << d_fbits << " bits\n";
	std::cout << "long double precision : " << q_fbits << " bits\n";

	std::cout << '\n';

	std::cout << "Representable?        : " << (isRepresentableInBinary(argv[1]) ? "maybe" : "no") << "\n\n";

	std::cout << "Decimal representations\n";
	std::cout << "input value: " << std::setprecision(f_prec) << std::setw(width) << argv[1] << '\n';
	std::cout << "      float: " << std::setprecision(f_prec) << std::setw(width) << f << '\n';
	std::cout << "     double: " << std::setprecision(d_prec) << std::setw(width) << d << '\n';
	std::cout << "long double: " << std::setprecision(q_prec) << std::setw(width) << q << '\n';

	std::cout << '\n';

	std::cout << "Hex representations\n";
	std::cout << "input value: " << std::setprecision(f_prec) << std::setw(width) << argv[1] << '\n';
	std::cout << "      float: " << std::setprecision(f_prec) << std::setw(width) << f << "    hex: " << to_hex(f) << '\n';
	std::cout << "     double: " << std::setprecision(d_prec) << std::setw(width) << d << "    hex: " << to_hex(d) << '\n';
	std::cout << "long double: " << std::setprecision(q_prec) << std::setw(width) << q << "    hex: " << to_hex(q) << '\n';

	std::cout << '\n';

	std::cout << "Binary representations:\n";
	std::cout << "      float: " << std::setprecision(f_prec) << std::setw(width) << f << "    bin: " << to_binary(f) << '\n';
	std::cout << "     double: " << std::setprecision(d_prec) << std::setw(width) << d << "    bin: " << to_binary(d) << '\n';
	std::cout << "long double: " << std::setprecision(q_prec) << std::setw(width) << q << "    bin: " << to_binary(q) << '\n';

	std::cout << '\n';

	std::cout << "Native triple representations (sign, scale, fraction):\n";
	std::cout << "      float: " << std::setprecision(f_prec) << std::setw(width) << f << "    triple: " << to_triple(f) << '\n';
	std::cout << "     double: " << std::setprecision(d_prec) << std::setw(width) << d << "    triple: " << to_triple(d) << '\n';
	std::cout << "long double: " << std::setprecision(q_prec) << std::setw(width) << q << "    triple: " << to_triple(q) << '\n';

	std::cout << '\n';

	value<f_fbits> vf(f);
	value<d_fbits> vd(d);
	value<q_fbits> vq(q);

	std::cout << "Scientific triple representation (sign, scale, fraction):\n";
	std::cout << "input value: " << std::setprecision(f_prec) << std::setw(width) << argv[1] << '\n';
	std::cout << "      float: " << std::setprecision(f_prec) << std::setw(width) << f << "    triple: " << to_triple(vf) << '\n';
	std::cout << "     double: " << std::setprecision(d_prec) << std::setw(width) << d << "    triple: " << to_triple(vd) << '\n';
	std::cout << "long double: " << std::setprecision(q_prec) << std::setw(width) << q << "    triple: " << to_triple(vq) << '\n';
	// TODO: implement a parse for that value to represent exactly
	std::cout << "      exact: " << "TBD" << '\n';

	std::cout << std::setprecision(old_precision) << std::endl;

	return EXIT_SUCCESS;
}
catch (const char* const msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
