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


namespace sw {
	namespace universal {

		template<typename Scalar>
		Scalar E(Scalar z, bool verbose = false) {
			using std::exp;
			if (z == Scalar(0.0)) return Scalar(1.0);
			Scalar e_of_z = exp(z);

			Scalar numerator = e_of_z - Scalar(1.0);
			if (verbose) {
				std::cout << "E(" << z << ")\n";
				std::cout << "  exp(z = " << z << ") = " << to_binary(e_of_z) << " : " << e_of_z << '\n';
				std::cout << "  (exp(z) - 1.0) = " << to_binary(numerator) << " : " << numerator << '\n';
				std::cout << "E(" << z << ") = " << to_binary(numerator / z) << " : " << numerator / z << '\n';
			}
			return numerator / z;
		}

		/// <summary>
		/// sample the E function in the interval [-1.0, 1.0]
		/// </summary>
		/// <typeparam name="Scalar"></typeparam>
		/// <param name="samples">nr of samples in the interval</param>
		template<typename Scalar>
		void sampleE(int samples) {
			Scalar x{ -1.0 };
			Scalar dx = Scalar(2.0) / Scalar(samples);
			for (int i = 0; i < samples; ++i) {
				std::cout << std::setw(10) << i << " : " << x << " : " << E(x) << '\n';
				x += dx;
			}
		}

		template<typename Scalar>
		Scalar Q(Scalar x, bool verbose = false) {
			using std::sqrt, std::abs;
			Scalar xsquare = x * x;
			Scalar xsquare_plus_one = xsquare + Scalar(1.0);
			Scalar sqrt_of_xsquare_plus_one = sqrt(xsquare_plus_one);
			Scalar xplus = x + sqrt_of_xsquare_plus_one;
			Scalar xminus = x - sqrt_of_xsquare_plus_one;

			Scalar abs_xminus = abs(xminus);
			Scalar one_over_xplus = Scalar(1.0) / xplus;
			Scalar q_of_x = abs_xminus - one_over_xplus;
			if (verbose) {
				std::cout << "Q(x=" << x << ")\n";
				std::cout << "  1st term  : " << to_binary(abs_xminus) << " : " << abs_xminus << '\n';
				std::cout << "  2nd term  : " << to_binary(one_over_xplus) << " : " << one_over_xplus << '\n';
				std::cout << "Q(x=" << x << ")  : " << to_binary(q_of_x) << " : " << q_of_x << '\n';
			}
			return q_of_x;
		}

		/// <summary>
		/// sample the Q function in the interval [-1.0, 1.0]
		/// </summary>
		/// <typeparam name="Scalar"></typeparam>
		/// <param name="samples">nr of samples in the interval</param>
		template<typename Scalar>
		void sampleQ(int samples) {
			Scalar x{ -1.0 };
			Scalar dx = Scalar(2.0) / Scalar(samples);
			for (int i = 0; i < samples; ++i) {
				std::cout << std::setw(10) << i << " : " << x << " : " << Q(x) << '\n';
				x += dx;
			}
		}

		template<typename Scalar>
		Scalar H(Scalar x, bool verbose = false) {
			Scalar q_of_x = Q(x, verbose);
			Scalar qx_squared = q_of_x * q_of_x;
			Scalar e_of_qx_squared = E(qx_squared, verbose);
			if (verbose) {
				std::cout << "H(x=" << x << ")\n";
				std::cout << "  Q(x=" << x << ") = " << to_binary(q_of_x) << " : " << q_of_x << '\n';
				std::cout << "  Q(x)*Q(x) = " << to_binary(qx_squared) << " : " << qx_squared << '\n';
				std::cout << "  E(Q^2) = " << to_binary(e_of_qx_squared) << " : " << e_of_qx_squared << '\n';
			}
			return e_of_qx_squared;
		}

		/// <summary>
		/// sample the H function in the interval [-1.0, 1.0]
		/// </summary>
		/// <typeparam name="Scalar"></typeparam>
		/// <param name="samples">nr of samples in the interval</param>
		template<typename Scalar>
		void sampleH(int samples) {
			Scalar x{ -1.0 };
			Scalar dx = Scalar(2.0) / Scalar(samples);
			for (int i = 0; i < samples; ++i) {
				std::cout << std::setw(10) << i << " : " << x << " : " << H(x) << '\n';
				x += dx;
			}
		}

		template<typename Scalar>
		void eval_H_at(Scalar x) {
			std::cout << "H(" << x << ") = " << H(x) << '\n';
		}

		template<typename Scalar>
		void sample_set() {
			Scalar x{ 0.0 };
			std::cout << "Scalar = " << type_tag(x) << '\n';
			eval_H_at(Scalar(1.0f));
			eval_H_at(Scalar(15.0f));
			eval_H_at(Scalar(9999.0f));
			std::cout << '\n';
		}
	}
}

int main()
try {
	using namespace sw::universal;

	//sampleE<float>(10);
	//sampleQ<float>(10);
	//sampleH<float>(10);

	sample_set<float>();
	sample_set<double>();
	sample_set<cfloat<128, 11>>();
	sample_set<cfloat<128, 11, std::uint32_t, true>>();
	sample_set<dd>();
	sample_set<qd>();
	sample_set<posit<256, 2>>();


	// Question: why does double-double work, but cfloat<128.11,subnormals> not work?
	std::cout << "Question: why does double-double work, but cfloat<128.11,subnormals> not work?\n";
	H(cfloat < 128, 11, std::uint32_t, true>{ 15.0f }, true);
	H(dd{ 15.0f }, true);
	std::cout << "Because the exp() function for cfloat<128,11> is not implemented yet\n";

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

