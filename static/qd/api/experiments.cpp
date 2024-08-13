// experiments.cpp: experiments with the quad-double floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <numbers>
// minimum set of include files to reflect source code dependencies
// Configure the qd template environment
// enable/disable arithmetic exceptions
#define DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/qd/qd.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/native/error_free_ops.hpp>

namespace sw {
	namespace universal {

		/* s = quick_three_accum(a, b, c) adds c to the dd-pair (a, b).
		 * If the result does not fit in two doubles, then the sum is
		 * output into s and (a,b) contains the remainder.  Otherwise
		 * s is zero and (a,b) contains the sum. */

		/// <summary>
		/// quick_three_accumulate calculates the relationship a + b + c = s + r
		/// </summary>
		/// <param name="a"></param>
		/// <param name="b"></param>
		/// <param name="c"></param>
		/// <returns></returns>
		inline double quick_three_accum(volatile double& a, volatile double& b, double c) {
			volatile double s;
			bool za, zb;

			s = two_sum(b, c, b);
			s = two_sum(a, s, a);

			za = (a != 0.0);
			zb = (b != 0.0);

			if (za && zb)
				return s;

			if (!zb) {
				b = a;
				a = s;
			}
			else {
				a = s;
			}

			return 0.0;
		}

		inline qd accurate_addition(const qd& a, const qd& b) {
			double u, v;
			int i{ 0 }, j{ 0 }, k{ 0 };
			if (std::abs(a[i]) > std::abs(b[j])) {
				u = a[i++];
			}
			else {
				u = b[j++];
			}
			if (std::abs(a[i]) > std::abs(b[j])) {
				v = a[i++];
			}
			else {
				v = b[j++];
			}

			u = quick_two_sum(u, v, v);

			double x[4] = { 0.0, 0.0, 0.0, 0.0 };
			while (k < 4) {
				if (i >= 4 && j >= 4) {
					x[k] = u;
					if (k < 3) {
						x[++k] = v;
					}
					break;
				}
				double t;
				if (i >= 4) {
					t = b[j++];
				}
				else if (j >= 4) {
					t = a[i++];
				}
				else if (std::abs(a[i]) > std::abs(b[j])) {
					t = a[i++];
				}
				else {
					t = b[j++];
				}

				double s = quick_three_accum(u, v, t);

				if (s != 0.0) {
					x[k++] = s;
				}
			}

			// add the rest
			for (k = i; k < 4; k++) x[3] += a[k];
			for (k = j; k < 4; k++) x[3] += b[k];

			renorm(x[0], x[1], x[2], x[3]);
			return qd(x[0], x[1], x[2], x[3]);
		}

		inline qd approximate_addition(const qd& a, const qd& b) {
			volatile double s0, s1, s2, s3;
			volatile double t0, t1, t2, t3;

			s0 = two_sum(a[0], b[0], t0);
			s1 = two_sum(a[1], b[1], t1);
			s2 = two_sum(a[2], b[2], t2);
			s3 = two_sum(a[3], b[3], t3);

			s1 = two_sum(s1, t0, t0);
			three_sum(s2, t0, t1);
			three_sum2(s3, t0, t2);
			t0 = t0 + t1 + t3;

			renorm(s0, s1, s2, s3, t0);
			return qd(s0, s1, s2, s3);
		}

		inline qd manual_approximate_addition(const qd& a, const qd& b) {
			// Same as above, but addition re-organized to minimize data dependencies
			double s0, s1, s2, s3;
			double t0, t1, t2, t3;

			double v0, v1, v2, v3;
			double u0, u1, u2, u3;
			double w0, w1, w2, w3;

			s0 = a[0] + b[0];
			s1 = a[1] + b[1];
			s2 = a[2] + b[2];
			s3 = a[3] + b[3];

			v0 = s0 - a[0];
			v1 = s1 - a[1];
			v2 = s2 - a[2];
			v3 = s3 - a[3];

			u0 = s0 - v0;
			u1 = s1 - v1;
			u2 = s2 - v2;
			u3 = s3 - v3;

			w0 = a[0] - u0;
			w1 = a[1] - u1;
			w2 = a[2] - u2;
			w3 = a[3] - u3;

			u0 = b[0] - v0;
			u1 = b[1] - v1;
			u2 = b[2] - v2;
			u3 = b[3] - v3;

			t0 = w0 + u0;
			t1 = w1 + u1;
			t2 = w2 + u2;
			t3 = w3 + u3;

			s1 = two_sum(s1, t0, t0);
			three_sum(s2, t0, t1);
			three_sum2(s3, t0, t2);
			t0 = t0 + t1 + t3;

			renorm(s0, s1, s2, s3, t0);
			return qd(s0, s1, s2, s3);
		}
	}
}



int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "quad-double (qd) experiments";
	int nrOfFailedTestCases = 0;

	auto oldPrec = std::cout.precision();

	double a0 = 1.0;
	double a1 = ulp(a0) / 2.0;
	double a2 = ulp(a1) / 2.0;
	double a3 = ulp(a2) / 2.0;

	ReportValue(a0, "a0 = 1.0");
	ReportValue(a1, "a1 = ulp(a0) / 2.0");
	ReportValue(a2, "a2 = ulp(a1) / 2.0");
	ReportValue(a3, "a3 = ulp(a2) / 2.0");
	renorm(a0, a1, a2, a3);  // double check this is a normalized quad-double configuration
	ReportValue(a0, "a0 = 1.0");
	ReportValue(a1, "a1 = ulp(a0) / 2.0");
	ReportValue(a2, "a2 = ulp(a1) / 2.0");
	ReportValue(a3, "a3 = ulp(a2) / 2.0");

	double b0 = 1.0;
	double b1 = ulp(b0) / 2.0;
	double b2 = ulp(b1) / 2.0;
	double b3 = ulp(b2) / 2.0;

	qd a(a0, a1, a2, a3);
	qd b(b0, b1, b2, b3);

	qd accurate_sum = accurate_addition(a, b);
	ReportValue(accurate_sum[0], "accurate_sum[0]");
	ReportValue(accurate_sum[1], "accurate_sum[1]");
	ReportValue(accurate_sum[2], "accurate_sum[2]");
	ReportValue(accurate_sum[3], "accurate_sum[3]");

	qd approximate_sum = approximate_addition(a, b);
	ReportValue(approximate_sum[0], "approximate_sum[0]");
	ReportValue(approximate_sum[1], "approximate_sum[1]");
	ReportValue(approximate_sum[2], "approximate_sum[2]");
	ReportValue(approximate_sum[3], "approximate_sum[3]");

	std::cout << to_quad(accurate_sum) << '\n';
	std::cout << to_binary(accurate_sum, true) << '\n';

	std::cout << std::setprecision(oldPrec);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}


/*
void ThreeSumExperiments() {
	using namespace sw::universal;
	std::cout << "three_sum experiments\n";

	double a{ 1.0 };
	double b{ ulp(a) / 2.0 };
	double c{ ulp(b) / 2.0 };
	ReportValue(a, "a = 1.0");
	ReportValue(b, "b = ulp(1.0)/2");
	ReportValue(c, "c = ulp(b)/2");

	std::cout << "two_sum\n";
	double r{ 0 };
	double s = two_sum(a, b, r);
	ReportValue(s, "sum");
	ReportValue(r, "residual");

	std::cout << "three_sum\n";
	double aa{ a }, bb{ b }, cc{ c };
	ReportValue(a, "a");
	ReportValue(b, "b");
	ReportValue(c, "c");
	three_sum(a, b, c);
	ReportValue(a, "a");
	ReportValue(b, "b");
	ReportValue(c, "c");

	std::cout << "three_sum2\n";
	a = aa, b = bb, c = cc; // reload
	ReportValue(a, "a");
	ReportValue(b, "b");
	ReportValue(c, "c");
	three_sum2(a, b, c);
	ReportValue(a, "a");
	ReportValue(b, "b");
	// c is unchanged

	std::cout << "three_sum3\n";
	a = aa, b = bb, c = cc; // reload
	ReportValue(a, "a");
	ReportValue(b, "b");
	ReportValue(c, "c");
	double sum = three_sum3(a, b, c);
	ReportValue(sum, "three_sum3");

}

three_sum experiments
             a = 1.0 : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
      b = ulp(1.0)/2 : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
        c = ulp(b)/2 : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
two_sum
                 sum : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
            residual : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
three_sum
 in                a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
                   c : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
 out               a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
                   c : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
three_sum2
 in                a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
                   c : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
 out               a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
three_sum3
 in                a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
                   c : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
 out      three_sum3 : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1

*/