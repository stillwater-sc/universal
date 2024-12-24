// conversion.cpp: experiments with user-defined conversions between number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>

#include <universal/number/posit/posit.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>

#include <universal/blas/blas.hpp>

namespace sw {
	namespace universal {

		template<typename SrcType, typename TargetType>
		TargetType convert(SrcType v) {
			return TargetType(double(v));
		}

		template<>
		dd convert(qd v) {
			return dd(double(v));
		}

		template<>
		qd convert(dd v) {
			return qd(double(v));
		}

	}
}

int main()
try {
	using namespace sw::universal;

	blas::vector<posit<8, 1>> v{};
	for (int i = 5; i >= -5; --i) {
		v.push_back(posit<8, 1>(i));
	}
	std::cout << "original vector : " << v << '\n';
	auto w = compress<posit<8, 1>, posit<5, 1>>(v);
	std::cout << "compressed vector : " << w << '\n';
	
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

