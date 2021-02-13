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
	using namespace std;
	using namespace sw::universal::internal;

	// long double attributes
	constexpr int f_prec = std::numeric_limits<float>::max_digits10;
	constexpr int d_prec = std::numeric_limits<double>::max_digits10;
	constexpr int q_prec = std::numeric_limits<long double>::max_digits10;

	constexpr int f_fbits = std::numeric_limits<float>::digits - 1;
	constexpr int d_fbits = std::numeric_limits<double>::digits - 1;
	constexpr int q_fbits = std::numeric_limits<long double>::digits - 1;

	if (argc != 2) {
		cerr << "Show the truncated value and (sign/scale/fraction) components of different floating point types." << endl;
		cerr << "Usage: ieee floating_point_value\n";
		cerr << "Example: ieee 0.03124999\n";
                cerr << "input value:                0.03124999\n";
                cerr << "      float:              0.0312499907 (+,-6,11111111111111111111011)\n";
                cerr << "     double:      0.031249989999999998 (+,-6,1111111111111111111101010100001100111000100011101110)\n";
                cerr << "long double:  0.0312499899999999983247 (+,-6,111111111111111111101001011110100011111111111110001111111001111)" << endl;

		return EXIT_SUCCESS;   // signal successful completion for ctest
	}

	std::streamsize old_precision = cout.precision();
	int width = q_prec + 4;

	long double q = stold(argv[1]);
	double d      = double(q);
	float f       = float(d);

	cout << "compiler              : " << report_compiler_version() << '\n';
	cout << "float precision       : " << f_fbits << " bits\n";
	cout << "double precision      : " << d_fbits << " bits\n";
	cout << "long double precision : " << q_fbits << " bits\n";

	cout << endl;

	cout << "Representable?        : " << (isRepresentableInBinary(argv[1]) ? "maybe" : "no") << "\n\n";

	cout << "Decimal representations\n";
	cout << "input value: " << setprecision(f_prec) << setw(width) << argv[1] << '\n';
	cout << "      float: " << setprecision(f_prec) << setw(width) << f << '\n';
	cout << "     double: " << setprecision(d_prec) << setw(width) << d << '\n';
	cout << "long double: " << setprecision(q_prec) << setw(width) << q << '\n';

	cout << endl;

	cout << "Hex representations\n";
	cout << "input value: " << setprecision(f_prec) << setw(width) << argv[1] << '\n';
	cout << "      float: " << setprecision(f_prec) << setw(width) << f << "    hex: " << to_hex(f) << '\n';
	cout << "     double: " << setprecision(d_prec) << setw(width) << d << "    hex: " << to_hex(d) << '\n';
	cout << "long double: " << setprecision(q_prec) << setw(width) << q << "    hex: " << to_hex(q) << '\n';

	cout << endl;

	cout << "Binary representations:\n";
	cout << "      float: " << setprecision(f_prec) << setw(width) << f << "    bin: " << to_binary(f) << '\n';
	cout << "     double: " << setprecision(d_prec) << setw(width) << d << "    bin: " << to_binary(d) << '\n';
	cout << "long double: " << setprecision(q_prec) << setw(width) << q << "    bin: " << to_binary(q) << '\n';

	cout << endl;

	cout << "Native triple representations (sign, scale, fraction):\n";
	cout << "      float: " << setprecision(f_prec) << setw(width) << f << "    triple: " << to_triple(f) << '\n';
	cout << "     double: " << setprecision(d_prec) << setw(width) << d << "    triple: " << to_triple(d) << '\n';
	cout << "long double: " << setprecision(q_prec) << setw(width) << q << "    triple: " << to_triple(q) << '\n';

	cout << '\n';

	value<f_fbits> vf(f);
	value<d_fbits> vd(d);
	value<q_fbits> vq(q);

	cout << "Scientific triple representation (sign, scale, fraction):\n";
	cout << "input value: " << setprecision(f_prec) << setw(width) << argv[1] << '\n';
	cout << "      float: " << setprecision(f_prec) << setw(width) << f << "    triple: " << components(vf) << '\n';
	cout << "     double: " << setprecision(d_prec) << setw(width) << d << "    triple: " << components(vd) << '\n';
	cout << "long double: " << setprecision(q_prec) << setw(width) << q << "    triple: " << components(vq) << '\n';
	// TODO: implement a parse for that value to represent exactly
	cout << "      exact: " << "TBD" << '\n';

	cout << setprecision(old_precision) << endl;

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
