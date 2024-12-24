// harmonic_series.cpp: experiments with mixed-precision representations of the Harmonic Series
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>

#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>

#include <universal/number/rational/rational.hpp>

namespace sw {
	namespace universal {



		template<typename Scalar>
		Scalar f(Scalar x) {
			using std::log, std::abs;
			return log(abs(3 * (1 - x) + 1)) / 80 + x * x + 1;
		}

		template<typename Scalar>
		void report_on_f(Scalar x) {
			Scalar f_of_x = f(x);
			std::cout << "f(" << x << ") = " << to_binary(f_of_x) << " : " << f_of_x << '\n';
		}

		// Kahan's Smooth Surprise: minimize log(abs(3(1-x) + 1))/80 + x^2 + 1 in the interval [0.8, 2.0]
		template<typename Scalar>
		Scalar smooth_surprise(int samples = 10) {

			Scalar x{ 0.8 };
			Scalar dx = Scalar(1.2) / Scalar(samples);
			Scalar min = std::numeric_limits<Scalar>::max();
			for (int i = 0; i < samples; ++i) {
				Scalar y = f(x);
				if (y < min) min = y;
				//std::cout << std::setw(10) << i << " : " << x << " : " << y << '\n';
				x += dx;
			}
			return min;

		}


	}
}

int main()
try {
	using namespace sw::universal;

	int samples = 1024 * 512;
	std::cout << "minimum = " << smooth_surprise<float>(samples) << '\n';

	float f_4{ 4.0 }, f_3{ 3.0 }, f_4_3 = f_4 / f_3;
	report_on_f(f_4_3);
	double d_4{ 4.0 }, d_3{ 3.0 }, d_4_3 = d_4 / d_3;
	report_on_f(d_4_3);
	dd dd_4{ 4.0 }, dd_3{ 3.0 }, dd_4_3 = dd_4 / dd_3;
	report_on_f(dd_4_3);
	qd qd_4{ 4.0 }, qd_3{ 3.0 }, qd_4_3 = qd_4 / qd_3;
	report_on_f(qd_4_3);

	rational<64> r_4{ 4 }, r_3{ 3 }, r_4_3 = r_4 / r_3;
	report_on_f(r_4_3);

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

