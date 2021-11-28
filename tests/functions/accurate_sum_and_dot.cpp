// accurate_sum_and_dot.cpp: twosum/twoproduct/cascadingdot
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <limits>
#include <vector>
#include <universal/native/ieee754.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/functions/twosum.hpp>

int main()
try {
	using namespace sw::universal;
	using namespace sw::function;

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
		std::cout << to_binary(a) << '\n';
		std::cout << to_binary(b) << '\n';
		std::cout << to_binary(s) << '\n';
		std::cout << to_binary(r) << '\n';

		double da(a), db(b), ds(s), dr(r);
		double sum = da + db;
		std::cout << "sum " << sum << " vs " << (ds + dr) << '\n';
	}

	{
		using Real = float;
		constexpr size_t N = 2;
		std::vector<Real> v(N);
		v[0] = 0.5f + std::numeric_limits<Real>::epsilon() / 2.0f;
		for (size_t i = 1; i < N; ++i) {
			v[i] = 1.0f;
		}
		int i = 0;
		for (Real e : v) {
			std::cout << "v[" << i++ << "] = " << e << '\n';
		}

		Real a(v[0]), b(v[1]), s, r;

		std::cout << "---\n";
		twoSum(a, b, s, r);
		std::cout << a << " + " << b << " = " << s << " + " << r << '\n';
		std::cout << to_binary(a) << '\n';
		std::cout << to_binary(b) << '\n';
		std::cout << to_binary(s) << '\n';
		std::cout << to_binary(r) << '\n';

		s = 0; r = 0;
		cascadingSum(v, s, r);
		std::cout << s << " + " << r << '\n';
		std::cout << to_binary(s) << '\n';
		std::cout << to_binary(r) << '\n';
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
