// numeric_constants.cpp: experiments with mixed-precision representations of important numerical constants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/integer/integer.hpp>
#include <numbers>   // since C++20

std::string version_string(int a, int b, int c) {
	std::ostringstream ss;
	ss << a << '.' << b << '.' << c;
	return ss.str();
}

template<typename Scalar>
void Represent(std::ostream& ostr, Scalar s, std::streamsize precision = 17, bool hexFormat = false) {
	// preserve the existing ostream precision
	auto old_precision = std::cout.precision();
	ostr << std::setprecision(precision);

	ostr << std::setw(15) << s;
	if (hexFormat) {
		ostr << " : " << std::setw(70) << sw::universal::color_print(s) << " : " << sw::universal::hex_format(s);
	}
	ostr << std::endl;

	// restore the previous ostream precision
	ostr << std::setprecision(old_precision);
}

void Sample(std::ostream& ostr, long double constant) {
	using namespace sw::universal;
	Represent(ostr << minmax_range< long double  >() << " : ", constant, 23);
	Represent(ostr << minmax_range< double       >() << " : ", double(constant), 15);
	Represent(ostr << minmax_range< float        >() << " : ", float(constant), 6);
	Represent(ostr << minmax_range< posit<32, 2> >() << " : ", posit<32, 2>(constant), 4, true);
	Represent(ostr << minmax_range< posit<32, 3> >() << " : ", posit<32, 3>(constant), 6, true);
	Represent(ostr << minmax_range< posit<40, 3> >() << " : ", posit<40, 3>(constant), 8, true);
	Represent(ostr << minmax_range< posit<48, 3> >() << " : ", posit<48, 3>(constant), 10, true);
	Represent(ostr << minmax_range< posit<56, 3> >() << " : ", posit<56, 3>(constant), 12, true);
	Represent(ostr << minmax_range< posit<64, 3> >() << " : ", posit<64, 3>(constant), 15, true);
}

void CompareIEEEValues(std::ostream& ostr, long double constant) {
	using namespace sw::universal;

	constexpr int f_prec = std::numeric_limits<float>::max_digits10;
	constexpr int d_prec = std::numeric_limits<double>::max_digits10;
	constexpr int q_prec = std::numeric_limits<long double>::max_digits10;

	constexpr int f_fbits = std::numeric_limits<float>::digits - 1;
	constexpr int d_fbits = std::numeric_limits<double>::digits - 1;
	constexpr int q_fbits = std::numeric_limits<long double>::digits - 1;

	float f = float(constant);
	double d = double(constant);
	long double q = constant;

	internal::value<f_fbits> vf(f);
	internal::value<d_fbits> vd(d);
	internal::value<q_fbits> vq(q);

	int width = q_prec + 5;

	std::streamsize old_precision = std::cout.precision();

	ostr << report_compiler_version() << std::endl;
	ostr << "float precision       : " << f_fbits << " bits\n";
	ostr << "double precision      : " << d_fbits << " bits\n";
	ostr << "long double precision : " << q_fbits << " bits\n";

	std::cout << std::endl;

//	ostr << "input value: " << std::setprecision(f_prec) << std::setw(width) << constant << endl;
	ostr << "      float: " << std::setprecision(f_prec) << std::setw(width) << f << " " << to_triple(vf) << std::endl;
	ostr << "     double: " << std::setprecision(d_prec) << std::setw(width) << d << " " << to_triple(vd) << std::endl;
	ostr << "long double: " << std::setprecision(q_prec) << std::setw(width) << q << " " << to_triple(vq) << std::endl;

	std::cout << std::setprecision(old_precision);
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::cout << "The Avogadro constant NA is exactly 6.02214076*10^+23 reciprocal mole.\n";
	Sample(std::cout, NA);

	std::cout << "----\n\n";
	CompareIEEEValues(std::cout, h);

	integer<128> i;
	if (parse("66260701500000000000000000000000000", i)) {
		std::cout << "h = " << i << '\n';
	}
	else {
		std::cerr << "error parsing h\n";
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

