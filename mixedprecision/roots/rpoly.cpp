// rpoly.cpp: mixed-precision experiments with Rpoly method
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>

#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>

#include <universal/blas/blas.hpp>

// Stable quadratic roots according to BKP Horn.
// http://people.csail.mit.edu/bkph/articles/Quadratics.pdf
template <typename Real>
void FindQuadraticPolynomialRoots(const Real a,
                                  const Real b,
                                  const Real c,
                                  std::vector<std::complex<Real>>& roots) { // TODO: std::complex does not work with arbitrary typess
  
  using namespace sw::universal;
  using std::sqrt, std::abs;
  const Real D = b * b - 4 * a * c;
  const Real sqrt_D = sqrt(abs(D));

  // Real roots.
  if (D >= 0) {
    if (b >= 0) {
      roots[0] = std::complex<Real>((-b - sqrt_D) / (2.0 * a), 0);
      roots[1] = std::complex<Real>((2.0 * c) / (-b - sqrt_D), 0);
    } else {
      roots[0] = std::complex<Real>((2.0 * c) / (-b + sqrt_D), 0);
      roots[1] = std::complex<Real>((-b + sqrt_D) / (2.0 * a), 0);
    }
    return;
  }

  // Use the normal quadratic formula for the complex case.
  roots[0] = std::complex<Real>(-b / (2.0 * a), sqrt_D / (2.0 * a));
  roots[1] = std::complex<Real>(-b / (2.0 * a), -sqrt_D / (2.0 * a));
}

// Specialized resolved fused dot product that assumes unit stride and a standard vector
namespace sw { namespace universal {
    template<typename Vector>
    sw::universal::enable_if_posit<sw::universal::value_type<Vector>, sw::universal::value_type<Vector> > // as return type
    my_fdp(const Vector& x, const Vector& y) {
        constexpr unsigned nbits = Vector::value_type::nbits;
        constexpr unsigned es = Vector::value_type::es;
        constexpr unsigned capacity = 20; // support vectors up to 1M elements
        quire<nbits, es, capacity> q(0);
        size_t ix, iy, n = size(x);
        for (ix = 0, iy = 0; ix < n && iy < n; ++ix, ++iy) {
            std::cout << to_binary(x[ix]) << ", " << sw::universal::to_binary(y[iy]) << '\n';
            q += sw::universal::quire_mul(x[ix], y[iy]);
            std::cout << q << '\n';
        }
        typename Vector::value_type sum;
        convert(q.to_value(), sum);     // one and only rounding step of the fused-dot product
        return sum;
    }
}}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

    {
        using Real = float;
        Real a{1.0f}, b{2.5f}, c{-10.1f};
        std::vector<std::complex<Real>> roots(2);

        FindQuadraticPolynomialRoots(a, b, c, roots);
        std::cout << roots[0] << " : " << roots[1] << '\n';
    }

    {
        constexpr bool hasSubnormal = true;
        constexpr bool hasSupernormal = true;
        constexpr bool isSaturating = false;
        using Real = cfloat<16, 5, std::uint8_t, hasSubnormal, hasSupernormal, isSaturating>;
        using Vector = blas::vector<Real>;
        
        Vector a = {1.0f, 1.0f}, b = {2.0f, 2.0f};
        Real c = 0.0f;
        c = a * b;
        std::cout << c << '\n';
    }

    {
        using Real = posit<16, 2>;
        using Vector = blas::vector<Real>;
        
        Real minPos(SpecificValue::minpos), maxPos(SpecificValue::maxpos);
        std::cout << minPos << ", sq: " << minPos * minPos << ", 8*: " << 8 * minPos << ", 9*: " << 9 * minPos << '\n';
        
        Vector a = {minPos, maxPos, maxPos, minPos}, b = {2.0f, maxPos, -maxPos, 2.0f};
        Real c = 0.0f;
        for(size_t i = 0; i < a.size(); ++i){
            Real d = a[i] * b[i];
            std::cout << to_binary(a[i]) << '\n' << to_binary(b[i]) << '\n' << to_binary(d) << '\n'; 
            c += d;
            std::cout << to_binary(c) << '\n';
        }

        std::cout << c << '\n'; // expect: minPos
        c = my_fdp(a, b);
        std::cout << c << '\n'; // expect: 2 * minPos
    }

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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
