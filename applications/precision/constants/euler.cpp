// euler.cpp: generating a 'perfect' approximation of Euler's constant e for a given number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit library with arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>

// best practice for C++ is to assign a literal
// but even this literal gets rounded in an assignment to an IEEE double
constexpr 
double e = 2.718281828459045235360287471;
//    e  = 2.718281828459045235360287471    value of above literal
//   ref = 2.71828182845904523536028747135266249775724709369995

// first 50 digits of e
static std::string e50 = "2.\
71828182845904523536028747135266249775724709369995";
// first 1000 digits of pi
static std::string e1000 = "2.\
71828182845904523536028747135266249775724709369995\
95749669676277240766303535475945713821785251664274\
27466391932003059921817413596629043572900334295260\
59563073813232862794349076323382988075319525101901\
15738341879307021540891499348841675092447614606680\
82264800168477411853742345442437107539077744992069\
55170276183860626133138458300075204493382656029760\
67371132007093287091274437470472306969772093101416\
92836819025515108657463772111252389784425056953696\
77078544996996794686445490598793163688923009879312\
77361782154249992295763514822082698951936680331825\
28869398496465105820939239829488793320362509443117\
30123819706841614039701983767932068328237646480429\
53118023287825098194558153017567173613320698112509\
96181881593041690351598888519345807273866738589422\
87922849989208680582574927961048419844436346324496\
84875602336248270419786232090021609902353043699418\
49146314093431738143640546253152096183690888707016\
76839642437814059271456354906130310720851038375051\
01157477041718986106873969655212671546889570350354";

int main()
try {
	using namespace sw::universal;

	std::cout << "Perfect approximations of Euler's constant E for different number systems\n";

	std::cout << e1000 << '\n';
	std::cout << "e   = " << std::setprecision(27) << e << '\n';
	std::cout << "ref = " << e50 << '\n';

	// 1000 digits -> 1.e1000 -> 2^3322 -> 1.051103774764883380737596422798e+1000 -> you will need 3322 bits to represent 1000 digits of phi
	// 
	// TODO: we need to implement parse(string) on the Universal number systems to calculate error

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
