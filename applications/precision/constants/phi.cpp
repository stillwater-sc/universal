// phi.cpp: generating a 'perfect' approximation of the Golden Ratio constant phi for a given number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// enable arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/fixpnt/fixpnt.hpp>

// best practice for C++ is to assign a literal
// but even this literal gets rounded in an assignment to an IEEE double
constexpr 
double phi_ = 1.618033988749895;
//     phi  = 1.618033988749894902525739    value of above literal
//     ref  = 1.61803398874989484820458683436563811772030917980576

// first 50 digits of phi
static std::string phi50 = "1.\
61803398874989484820458683436563811772030917980576";
// first 1000 digits of phi
static std::string phi1000 = "1.\
61803398874989484820458683436563811772030917980576\
28621354486227052604628189024497072072041893911374\
84754088075386891752126633862223536931793180060766\
72635443338908659593958290563832266131992829026788\
06752087668925017116962070322210432162695486262963\
13614438149758701220340805887954454749246185695364\
86444924104432077134494704956584678850987433944221\
25448770664780915884607499887124007652170575179788\
34166256249407589069704000281210427621771117778053\
15317141011704666599146697987317613560067087480710\
13179523689427521948435305678300228785699782977834\
78458782289110976250030269615617002504643382437764\
86102838312683303724292675263116533924731671112115\
88186385133162038400522216579128667529465490681131\
71599343235973494985090409476213222981017261070596\
11645629909816290555208524790352406020172799747175\
34277759277862561943208275051312181562855122248093\
94712341451702237358057727861600868838295230459264\
78780178899219902707769038953219681986151437803149\
97411069260886742962267575605231727775203536139362";


template<unsigned fbits>
sw::universal::fixpnt<fbits+5, fbits> goldenRatio() {
	sw::universal::fixpnt<fbits + 5, fbits> phi(5);
	return (1 + sqrt(phi)) / 2;
}

int main()
try {
	using namespace sw::universal;

	std::cout << "Perfect approximations of the Golden Ratio constant phi for different number systems\n";

	std::cout << phi1000 << '\n';
	std::cout << "phi  = " << std::setprecision(25) << phi_ << '\n';
	std::cout << "ref  = " << phi50 << '\n';

	// 1000 digits -> 1.e1000 -> 2^3322 -> 1.051103774764883380737596422798e+1000 -> you will need 3322 bits to represent 1000 digits of phi
	// 
	// TODO: we need to implement parse(string) on the Universal number systems to calculate error

	// phi are the roots of the question: phi^2 - phi - 1 = 0
	// +phi = (1 + sqrt(5))/2  -phi = (1 - sqrt(5))/2

	// 50 digits -> 1.e50 -> 2^165 -> so we need 165 bits to represent 50 digits of phi
	// 10 digits ->  33 bits
	// 20 digits ->  66 bits
	// 30 digits ->  99 bits
	// 40 digits -> 132 bits
	// fixpnt<40, 33> phi10(5);
	std::cout << phi50 << '\n';
	std::cout << goldenRatio<33>() << '\n';
	std::cout << goldenRatio<66>() << '\n';
	std::cout << goldenRatio<99>() << '\n';
	std::cout << goldenRatio<132>() << '\n';
	std::cout << goldenRatio<165>() << '\n';
	std::cout << goldenRatio<198>() << '\n';
	std::cout << goldenRatio<231>() << '\n';


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
