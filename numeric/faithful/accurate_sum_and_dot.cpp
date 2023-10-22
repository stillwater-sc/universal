// accurate_sum_and_dot.cpp: twosum/twoproduct/cascadingdot
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/bit_cast.hpp>
#include <limits>
#include <vector>
#include <universal/native/ieee754.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/numerics/twosum.hpp>

template<typename Real>
int DemonstrateCascadeSum(size_t N = 10) {
	using namespace sw::universal;

	std::cout << "+-------------   cascade sum --------------+\n";
	std::vector<Real> v(N);
	v[0] = 0.5f + std::numeric_limits<Real>::epsilon() / 2.0f;
	v[1] = 1.0f;
	for (size_t i = 2; i < N; ++i) {
		v[i] = 1.0f + std::numeric_limits<Real>::epsilon();
	}
	size_t i = 0;
	for (Real e : v) {
		std::cout << "v[" << i++ << "] = " << e << '\n';
	}

	Real a(v[0]), b(v[1]), s, r;

	std::cout << "---\n";
	twoSum(a, b, s, r);
	std::cout << a << " + " << b << " = " << s << " + " << r << '\n';

	s = 0; r = 0;
	std::cout << "---cascading sum\n";
	cascadingSum(v, s, r);
	std::cout << s << " + " << r << '\n';

	// validate
	std::vector<double> dv(N);
	i = 0;
	for (auto e : v) {
		dv[i] = e; // convert to double
		++i;
	}
	double ds(s), dr(r);
	double sum = 0;
	for (auto e : dv) {
		sum += e;
		std::cout << to_triple(float(sum)) << "                              : " << float(sum) << '\n';
		std::cout << to_triple(sum) << " : " << sum << '\n';
	}
	std::cout << "results of the cascadeSum function\n";
	std::cout << to_triple(ds + dr) << " : " << (ds + dr) << " <- cascade calculation\n";
	std::cout << "sum " << sum << " vs " << (ds + dr) << '\n';

	return (sum == (ds + dr) ? 0 : 1);
}

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

	DemonstrateCascadeSum<float>();

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
