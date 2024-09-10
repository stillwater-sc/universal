// float_to_decimal_string.cpp: experiments with algorithms to convert a Real value to a string of decimal digits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>

#include <universal/number/cfloat/cfloat.hpp>

int main(int argc, char** argv)
try {
    using namespace sw::universal;

    float f = 3.14156;
    float x = f * 1e6;
    long long int y = static_cast<long long int>(x);

    std::vector<char> V;
    while (y) {
        std::cout << y << " : " << (y % 10) << " : " << (y / 10) << '\n';
        V.push_back(y % 10 + '0');
        y /= 10;
    }
    std::reverse(V.begin(), V.end());

    std::string S;
    for (size_t i = 0; i < V.size() - 6; ++i) {
        S.push_back(V[i]);
    }
    S.push_back('.');
    for (size_t i = V.size() - 6; i < V.size(); ++i) {
        S.push_back(V[i]);
    }

    size_t i = S.size();
    while (i--) {
        if (S[i] == '0') {
            S.erase(S.begin() + i);
        }
        else {
            break;
        }
    }

    std::cout << "Custom conversion: " << S << std::endl;

    return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
