// arithmetic.cpp: faithfully rounded floating-point arithmetic
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/faithful/faithful.hpp>
#include <universal/functions/twosum.hpp>

int main()
try {
	using namespace sw::universal;


	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

	// float
	{
		using Real = float;
		Real a, b, s, r;

		a = 0.5f + std::numeric_limits<Real>::epsilon() / 2.0f;
		b = 1.0f;
		twoSum(a, b, s, r);
		std::cout << a << " + " << b << " = " << s << " + " << r << '\n';
		// validation using a double
		double da(a), db(b), ds(s), dr(r);
		double sum = da + db;
		std::cout << "sum " << sum << " vs " << (ds + dr) << '\n';
	}


	// restore the previous ostream precision
	std::cout << std::setprecision(precision);

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
