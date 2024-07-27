// irrational_powers.cpp: experiments with irrational numbers and their approximations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>

/*
 * Idea from this youtube video: https://youtu.be/4lvk7lYQ1RQ
 *
 * an irrational number to the power of an irrational number: can it ever be rational?
 *
 * for r, q element of Irrational numbers, is it possible that
 * r ^ q = a / b?
 *
 * Answer is yes, it can be. Basic demonstration:
 *
 * Take r and q as sqrt(2). r^q is irrational.
 * Take r^q, an irrational number, and raise it to the power of sqrt(2) again:
 *
 * (r^q)^q => r^(q*q) -> q*q = 2 -> r^2 = 2
 *
 * Let's see what happens when you use floating-point arithmetic
*/

static constexpr size_t FIELD_WIDTH = 75;

template<typename Real>
Real r_to_q_to_q(const Real& r, const Real& q) {
	return pow(pow(r,q),q);
}

template<typename Real>
void evaluate(double _r, double _q) 
{
	std::stringstream s;
	s << _r << "^(" << _q << ")^(" << _q << ") = ";

	Real r(_r);
	Real q(_q);

	std::stringstream t;
	t << std::right << std::setw(FIELD_WIDTH) << sw::universal::type_tag(r) << ":   ";

	std::cout << t.str() << s.str() << r_to_q_to_q(r, q) << '\n';
}

void CompareIrrationalPowers(double _r, double _q) {
	using Fixpnt = sw::universal::fixpnt<32, 16>;
	using Posit8 = sw::universal::posit<8, 2>;
	using Posit16 = sw::universal::posit<16, 2>;
	using Posit32 = sw::universal::posit<32, 2>;
	using Posit64 = sw::universal::posit<64, 2>;
	using Posit128 = sw::universal::posit<128, 2>;
	using Cfloat8 = sw::universal::cfloat<8, 2>;
	using Cfloat16 = sw::universal::cfloat<16, 5>;
	using Cfloat32 = sw::universal::cfloat<32, 8>;


	evaluate<Fixpnt>(_r, _q);
	evaluate<Posit8>(_r, _q);
	evaluate<Posit16>(_r, _q);
	evaluate<Posit32>(_r, _q);
	evaluate<Posit64>(_r, _q);
	evaluate<Posit128>(_r, _q);
	evaluate<Cfloat8>(_r, _q);
	evaluate<Cfloat16>(_r, _q);
	evaluate<Cfloat32>(_r, _q);
	evaluate<float>(_r, _q);
	std::cout << '\n';
}

int main()
try {
	using namespace sw::universal;

	CompareIrrationalPowers(sqrt(2.0), sqrt(2.0));
	CompareIrrationalPowers(pow(2.0, 0.5), pow(2.0, 0.5));
	CompareIrrationalPowers(pow(3.0, 1.0/3.0), sqrt(3.0));
	CompareIrrationalPowers(pow(3.333333333333333, 1.0 / 3.33333333333333), sqrt(3.3333333333333333333));

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
