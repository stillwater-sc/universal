// rpoly.cpp: mixed-precision experiments with Rpoly root finding method
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
// Portions of this file are adapted from work by Chris Sweeney (2015) under the 3-Clause BSD License.
// See https://github.com/sweeneychris/RpolyPlusPlus for more details

#include <iostream>
#include <iomanip>

#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>

#include <universal/blas/blas.hpp>

template <typename Real>
sw::universal::blas::vector<Real> Head(const sw::universal::blas::vector<Real>& vector, size_t size){
    sw::universal::blas::vector<Real> head(size);
    for (size_t i = 0; i < size && i < vector.size(); ++i){
        head[i] = vector[i];
    }

    return head;
}

template <typename Real>
sw::universal::blas::vector<Real> Tail(const sw::universal::blas::vector<Real>& vector, size_t size){
    sw::universal::blas::vector<Real> tail(size);
    size_t startIdx = vector.size() - size > 0 ? vector.size() - size : 0;
    for (size_t i = startIdx; i < vector.size(); ++i){
        tail[i] = vector[i - startIdx];
    }

    return tail;
}

template <typename Real>
void SetZero(sw::universal::blas::vector<Real>& vector, size_t size){
    vector.resize(size);
    for (size_t i = 0; i < size; ++i){
        vector[i] = 0;
    }
}

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

// Machine precision constants.
static const double mult_eps = std::numeric_limits<double>::epsilon();
static const double sum_eps = std::numeric_limits<double>::epsilon();
static const double kAbsoluteTolerance = 1e-14;
static const double kRelativeTolerance = 1e-10;

enum ConvergenceType{
    NO_CONVERGENCE = 0,
    LINEAR_CONVERGENCE = 1,
    QUADRATIC_CONVERGENCE = 2
};

// Evaluate the polynomial at x using the Horner scheme.
template <typename Real>
inline Real EvaluatePolynomial(const sw::universal::blas::vector<Real>& polynomial, const Real& x) {
    Real v = 0.0;
    for (size_t i = 0; i < polynomial.size(); ++i) {
        v = v * x + polynomial[i]; // TODO: quire?
    }
    return v;
}
// Evaluate the polynomial at complex x using the Horner scheme.
template <typename Real>
inline std::complex<Real> EvaluateComplexPolynomial(const sw::universal::blas::vector<Real>& polynomial, const std::complex<Real>& x) {
    std::complex<Real> v; // Should default to 0 + 0i
    for (size_t i = 0; i < polynomial.size(); ++i) {
        v = v * x + polynomial[i]; // TODO: quire?
    }
    return v;
}

// Remove leading terms with zero coefficients.
template <typename Real>
sw::universal::blas::vector<Real> RemoveLeadingZeros(const sw::universal::blas::vector<Real>& polynomial_in) {
  size_t i = 0;
  while (i < (polynomial_in.size() - 1) && polynomial_in[i] == Real(0.0)) {
    ++i;
  }
  return Tail(polynomial_in, polynomial_in.size() - i);
}

template <typename Real>
sw::universal::blas::vector<Real> DifferentiatePolynomial(const sw::universal::blas::vector<Real>& polynomial) {
    const int degree = polynomial.size() - 1;

    // Degree zero polynomials are constants, and their derivative does
    // not result in a smaller degree polynomial, just a degree zero
    // polynomial with value zero.
    if (degree == 0) {
        return sw::universal::blas::vector<Real>(1); // defaults to zero vector
    }

    sw::universal::blas::vector<Real> derivative(degree);
    for (int i = 0; i < degree; ++i) {
        derivative[i] = (degree - i) * polynomial[i];
    }

    return derivative;
}

template <typename Real>
sw::universal::blas::vector<Real> MultiplyPolynomials(const sw::universal::blas::vector<Real>& poly1, const sw::universal::blas::vector<Real>& poly2) {
    sw::universal::blas::vector<Real> multiplied_poly = sw::universal::blas::vector<Real>(poly1.size() + poly2.size() - 1); // defaults to zero vector
    for (size_t i = 0; i < poly1.size(); ++i) {
        for (size_t j = 0; j < poly2.size(); ++j) {
            multiplied_poly[i + j] += poly1[i] * poly2[j];
        }
    }
    return multiplied_poly;
}

template <typename Real>
sw::universal::blas::vector<Real> AddPolynomials(const sw::universal::blas::vector<Real>& poly1, const sw::universal::blas::vector<Real>& poly2) {
    if (poly1.size() > poly2.size()) {
        sw::universal::blas::vector<Real> sum = poly1;
        int diff = poly1.size() - poly2.size();
        // Add poly2 to the last poly2.size() elements of sum
        for (size_t i = 0;  i < poly2.size(); ++i){
            sum[diff + i] += poly2[i];
        }
        return sum;
    } else {
        sw::universal::blas::vector<Real> sum = poly2;
        int diff = poly2.size() - poly1.size();
        // Add poly1 to the last poly1.size() elements of sum
        for (size_t i = 0;  i < poly1.size(); ++i){
            sum[diff + i] += poly1[i];
        }
        return sum;
    }
}

template <typename Real>
Real FindRootIterativeNewton(const sw::universal::blas::vector<Real>& polynomial, const Real x0,
                             const Real epsilon, const int max_iterations) {
    using namespace sw::universal;
    using std::abs;
    
    Real root = x0;
    const blas::vector<Real> derivative = DifferentiatePolynomial(polynomial);
    Real prev;
    for (int i = 0; i < max_iterations; i++) {
        prev = root;
        root -= EvaluatePolynomial(polynomial, root) / EvaluatePolynomial(derivative, root);
        if (abs(prev - root) < epsilon) {
            break;
        }
    }
    return root;
}

// Solves for the root of the equation ax + b = 0.
template <typename Real>
Real FindLinearPolynomialRoots(const Real a, const Real b) {
  return -b / a;
}

// Stable quadratic roots according to BKP Horn.
// http://people.csail.mit.edu/bkph/articles/Quadratics.pdf
template <typename Real>
void FindQuadraticPolynomialRoots(const Real a, const Real b, const Real c,
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

// Perform division by a linear term of the form (z - x) and evaluate P at x.
template <typename Real>
void SyntheticDivisionAndEvaluate(const sw::universal::blas::vector<Real>& polynomial,
                                  const Real x,
                                  sw::universal::blas::vector<Real>& quotient,
                                  Real& eval) {
    SetZero(quotient, polynomial.size() - 1);
  
    quotient[0] = polynomial[0];
    for (size_t i = 1; i < polynomial.size() - 1; i++) {
        quotient[i] = polynomial[i] + quotient[i - 1] * x;
    }
    eval = polynomial[polynomial.size() - 1] + quotient[quotient.size() - 1] * x;
}

// Perform division of a polynomial by a quadratic factor. The quadratic divisor should have leading 1s.
template <typename Real>
void QuadraticSyntheticDivision(const sw::universal::blas::vector<Real>& polynomial,
                                const sw::universal::blas::vector<Real>& quadratic_divisor,
                                sw::universal::blas::vector<Real>& quotient,
                                sw::universal::blas::vector<Real>& remainder) {
    SetZero(quotient, polynomial.size() - 2);
    SetZero(remainder, 2);

    quotient[0] = polynomial[0];

    // If the quotient is a constant then polynomial is degree 2 and the math is simple.
    if (quotient.size() == 1) {
        // TODO: maybe this could be written cleaner??
        remainder = {polynomial[1] - polynomial[0] * quadratic_divisor[1], polynomial[2] - polynomial[0] * quadratic_divisor[2]};
        return; 
    }

    quotient[1] = polynomial[1] - polynomial[0] * quadratic_divisor[1];
    for (size_t i = 2; i < polynomial.size() - 2; i++) {
        quotient[i] = polynomial[i] - 
                        quotient[i - 2] * quadratic_divisor[2] - 
                        quotient[i - 1] * quadratic_divisor[1];
    }

    remainder[0] = polynomial[polynomial.size() - 2] -
                        quadratic_divisor[1] * quotient[quotient.size() - 1] -
                        quadratic_divisor[2] * quotient[quotient.size() - 2];
    remainder[1] = polynomial[polynomial.size() - 1] - 
                        quadratic_divisor[2] * quotient[quotient.size() - 1];
}

// Determines whether the iteration has converged by examining the three most recent values for convergence.
template <typename Real>
bool HasConverged(const sw::universal::blas::vector<Real>& sequence) { // TODO: could probably just be a normal vector, just for storing data so...
    using namespace sw::universal;
    using std::abs;
    
    const bool convergence_condition_1 =
        abs(sequence(1) - sequence(0)) < abs(sequence(0)) / 2.0;
    const bool convergence_condition_2 =
        abs(sequence(2) - sequence(1)) < abs(sequence(1)) / 2.0;

    // If the sequence has converged then return true.
    return convergence_condition_1 && convergence_condition_2;
}

// Determines if the root has converged by measuring the relative and absolute
// change in the root value. This stopping criterion is a simple measurement
// that proves to work well. It is referred to as "Ward's method" in the
// following reference:
//
// Nikolajsen, Jorgen L. "New stopping criteria for iterative root finding."
// Royal Society open science (2014)
template <typename Real>
bool HasRootConverged(const std::vector<std::complex<Real>>& roots) { 
    using namespace sw::universal;
    using std::abs;
    
    static const Real kRootMagnitudeTolerance = 1e-8;

    if (roots.size() != 3) {
        return false;
    }

    const Real e_i = abs(roots[2] - roots[1]);
    const Real e_i_minus_1 = abs(roots[1] - roots[0]);
    const Real mag_root = abs(roots[1]);
    if (e_i <= e_i_minus_1) {
        if (mag_root < kRootMagnitudeTolerance) {
            return e_i < kAbsoluteTolerance;
        } else {
            return e_i / mag_root <= kRelativeTolerance;
        }
    }

    return false;
}
template <typename Real>
bool HasRootConverged(const std::vector<Real>& roots) { // TODO: NOTE: this is not always real, it could be complex<Real> so should maybe rename for clarity
    using namespace sw::universal;
    using std::abs;
    
    static const Real kRootMagnitudeTolerance = 1e-8;
    if (roots.size() != 3) {
        return false;
    }

    const Real e_i = abs(roots[2] - roots[1]);
    const Real e_i_minus_1 = abs(roots[1] - roots[0]);
    const Real mag_root = abs(roots[1]);
    if (e_i <= e_i_minus_1) {
        if (mag_root < kRootMagnitudeTolerance) {
            return e_i < kAbsoluteTolerance;
        } else {
            return e_i / mag_root <= kRelativeTolerance;
        }
    }

    return false;
}

// Implementation closely follows the three-stage algorithm for finding roots of
// polynomials with real coefficients as outlined in: "A Three-Stage Algorithm
// for Real Polynomaials Using Quadratic Iteration" by Jenkins and Traub, SIAM
// 1970. Please note that this variant is different than the complex-coefficient
// version, and is estimated to be up to 4 times faster.
template <typename Real>
class JenkinsTraubSolver {
public:
    JenkinsTraubSolver(const sw::universal::blas::vector<Real>& coeffs, sw::universal::blas::vector<Real>& real_roots, sw::universal::blas::vector<Real>& complex_roots)
        : polynomial_(coeffs), real_roots_(real_roots), complex_roots_(complex_roots), num_solved_roots_(0) 
        { sigma_.resize(3); }

    // Extracts the roots using the Jenkins Traub method.
    bool ExtractRoots();

    private:
    // Removes any zero roots and divides polynomial by z.
    void RemoveZeroRoots();

    // Computes the magnitude of the roots to provide and initial search radius
    // for the iterative solver.
    Real ComputeRootRadius();

    // Computes the zero-shift applied to the K-Polynomial.
    void ComputeZeroShiftKPolynomial();

    // Stage 1 of the Jenkins-Traub method. This stage is not technically
    // necessary, but helps separate roots that are close to zero.
    void ApplyZeroShiftToKPolynomial(const int num_iterations);

    // Computes and returns the update of sigma(z) based on the current
    // K-polynomial.
    //
    // NOTE: This function is used by the fixed shift iterations (which hold sigma
    // constant) so sigma is *not* modified internally by this function. If you
    // want to change sigma, simply call
    //    sigma = ComputeNextSigma();
    void ComputeNextSigma(sw::universal::blas::vector<Real>& nextSigma);

    // Updates the K-polynomial based on the current value of sigma for the fixed
    // or variable shift stage.
    void UpdateKPolynomialWithQuadraticShift(
        const sw::universal::blas::vector<Real>& polynomial_quotient,
        const sw::universal::blas::vector<Real>& k_polynomial_quotient);

    // Apply fixed-shift iterations to the K-polynomial to separate the
    // roots. Based on the convergence of the K-polynomial, we apply a
    // variable-shift linear or quadratic iteration to determine a real root or
    // complex conjugate pair of roots respectively.
    ConvergenceType ApplyFixedShiftToKPolynomial(const std::complex<Real>& root, const int max_iterations);

    // Applies one of the variable shifts to the K-Polynomial. Returns true upon
    // successful convergence to a good root, and false otherwise.
    bool ApplyVariableShiftToKPolynomial(const ConvergenceType& fixed_shift_convergence, const std::complex<Real>& root);

    // Applies a quadratic shift to the K-polynomial to determine a pair of roots
    // that are complex conjugates. Return true if a root was successfully found.
    bool ApplyQuadraticShiftToKPolynomial(const std::complex<Real>& root,
                                        const int max_iterations);

    // Applies a linear shift to the K-polynomial to determine a single real root.
    // Return true if a root was successfully found.
    bool ApplyLinearShiftToKPolynomial(const std::complex<Real>& root,
                                        const int max_iterations);

    // Adds the root to the output variables.
    void AddRootToOutput(const Real real, const Real imag);

    // Solves polynomials of degree <= 2.
    bool SolveClosedFormPolynomial();

    // Helper variables to manage the polynomials as they are being manipulated
    // and deflated.
    sw::universal::blas::vector<Real> polynomial_;
    sw::universal::blas::vector<Real> k_polynomial_;
    // Sigma is the quadratic factor the divides the K-polynomial.
    sw::universal::blas::vector<Real> sigma_;

    // Let us define a, b, c, and d such that:
    //   P(z) = Q_P * sigma(z) + b * (z + u) + a
    //   K(z) = Q_K * sigma(z) + d * (z + u ) + c
    //
    // where Q_P and Q_K are the quotients from polynomial division of
    // sigma(z). Note that this means for a given a root s of sigma:
    //
    //   P(s)      = a - b * s_conj
    //   P(s_conj) = a - b * s
    //   K(s)      = c - d * s_conj
    //   K(s_conj) = c - d * s
    Real a_, b_, c_, d_;

    // Output reference variables.
    sw::universal::blas::vector<Real>& real_roots_;
    sw::universal::blas::vector<Real>& complex_roots_;
    int num_solved_roots_;

    // Keeps track of whether the linear and quadratic shifts have been attempted
    // yet so that we do not attempt the same shift twice.
    bool attempted_linear_shift_;
    bool attempted_quadratic_shift_;

    // Number of zero-shift iterations to perform.
    static const int kNumZeroShiftIterations = 20;

    // The number of fixed shift iterations is computed as
    //   # roots found * this multiplier.
    static const int kFixedShiftIterationMultiplier = 20;

    // If the fixed shift iterations fail to converge, we restart this many times
    // before considering the solve attempt as a failure.
    static const int kMaxFixedShiftRestarts = 20;

    // The maximum number of linear shift iterations to perform before considering
    // the shift as a failure.
    static const int kMaxLinearShiftIterations = 20;

    // The maximum number of quadratic shift iterations to perform before
    // considering the shift as a failure.
    static const int kMaxQuadraticShiftIterations = 20;

    // When quadratic shift iterations are stalling, we attempt a few fixed shift
    // iterations to help convergence.
    static const int kInnerFixedShiftIterations = 5;

    // During quadratic iterations, the real values of the root pairs should be
    // nearly equal since the root pairs are complex conjugates. This tolerance
    // measures how much the real values may diverge before consider the quadratic
    // shift to be failed.
    static const Real kRootPairTolerance;
};
template <typename Real>
const Real JenkinsTraubSolver<Real>::kRootPairTolerance = Real(0.01);

template <typename Real>
bool JenkinsTraubSolver<Real>::ExtractRoots() {
    if (polynomial_.size() == 0) {
        std::cout << "Invalid polynomial of size 0 passed to FindPolynomialRootsJenkinsTraub\n";
        return false;
    }

    // Remove any leading zeros of the polynomial.
    polynomial_ = RemoveLeadingZeros(polynomial_);
    // Normalize the polynomial.
    polynomial_ = polynomial_ / polynomial_[0]; // NOTE: polynomial /= polynomial_[0] does not work...
    const int degree = polynomial_.size() - 1;

    // Allocate the output roots.
    SetZero(real_roots_, degree);
    SetZero(complex_roots_, degree);

    // Remove any zero roots.
    RemoveZeroRoots();

    // Choose the initial starting value for the root-finding on the complex plane.
    const Real kDegToRad = M_PI / 180.0;
    Real phi = 49.0 * kDegToRad;

    // Iterate until the polynomial has been completely deflated.
    for (int i = 0; i < degree; i++) {
        // Compute the root radius.
        const Real root_radius = ComputeRootRadius();

        // Solve in closed form if the polynomial is small enough.
        if (polynomial_.size() <= 3) {
            break;
        }

        // Stage 1: Apply zero-shifts to the K-polynomial to separate the small zeros of the polynomial.
        ApplyZeroShiftToKPolynomial(kNumZeroShiftIterations);

        // Stage 2: Apply fixed shift iterations to the K-polynomial to separate the roots further.
        std::complex<Real> root;
        ConvergenceType convergence = NO_CONVERGENCE;
        for (int j = 0; j < kMaxFixedShiftRestarts; j++) {
            root = root_radius * std::complex<Real>(sw::universal::cos(phi), sw::universal::sin(phi));
            convergence = ApplyFixedShiftToKPolynomial(root, kFixedShiftIterationMultiplier * (i + 1));
            if (convergence != NO_CONVERGENCE) {
                break;
            }

            // Rotate the initial root value on the complex plane and try again.
            phi += 94.0 * kDegToRad;
        }

        // Stage 3: Find the root(s) with variable shift iterations on the
        // K-polynomial. If this stage was not successful then we return a failure.
        if (!ApplyVariableShiftToKPolynomial(convergence, root)) {
            return false;
        }
    }
    return SolveClosedFormPolynomial();
}

// Stage 1: Generate K-polynomials with no shifts (i.e. zero-shifts).
template <typename Real>
void JenkinsTraubSolver<Real>::ApplyZeroShiftToKPolynomial(const int num_iterations) {
    // K0 is the first order derivative of polynomial.
    k_polynomial_ = DifferentiatePolynomial(polynomial_) / polynomial_.size();
    for (int i = 1; i < num_iterations; i++) {
        ComputeZeroShiftKPolynomial();
    }
}

template <typename Real>
ConvergenceType JenkinsTraubSolver<Real>::ApplyFixedShiftToKPolynomial(const std::complex<Real>& root, const int max_iterations) {
    // Compute the fixed-shift quadratic:
    // sigma(z) = (x - m - n * i) * (x - m + n * i) = x^2 - 2 * m + m^2 + n^2.
    sigma_[0] = 1.0;
    sigma_[1] = -2.0 * root.real();
    sigma_[2] = root.real() * root.real() + root.imag() * root.imag();

    // Compute the quotient and remainder for divinding P by the quadratic divisor.
    // Since this iteration involves a fixed-shift sigma these may be computed once prior to any iterations.
    sw::universal::blas::vector<Real> polynomial_quotient, polynomial_remainder;
    QuadraticSyntheticDivision(polynomial_, sigma_, polynomial_quotient, polynomial_remainder);

    // Compute a and b from the above equations.
    b_ = polynomial_remainder[0];
    a_ = polynomial_remainder[1] - b_ * sigma_[1];

    // Precompute P(s) for later using the equation above.
    const std::complex<Real> p_at_root = a_ - b_ * std::conj(root);

    // These two containers hold values that we test for convergence such that the
    // zero index is the convergence value from 2 iterations ago, the first
    // index is from one iteration ago, and the second index is the current value.
    sw::universal::blas::vector<std::complex<Real>> t_lambda(3);
    sw::universal::blas::vector<Real> sigma_lambda(3);
    sw::universal::blas::vector<Real> k_polynomial_quotient, k_polynomial_remainder;
    for (int i = 0; i < max_iterations; ++i) {
        k_polynomial_ /= k_polynomial_[0];

        // Divide the shifted polynomial by the quadratic polynomial.
        QuadraticSyntheticDivision(k_polynomial_, sigma_, k_polynomial_quotient, k_polynomial_remainder);
        d_ = k_polynomial_remainder[0];
        c_ = k_polynomial_remainder[1] - d_ * sigma_[1];

        // Test for convergence.
        sw::universal::blas::vector<Real> variable_shift_sigma(3);
        ComputeNextSigma(variable_shift_sigma);
        const std::complex<Real> k_at_root = c_ - d_ * std::conj(root);


        t_lambda[0] = t_lambda[t_lambda.size() - 2];
        t_lambda[1] = t_lambda[t_lambda.size() - 1];

        sigma_lambda[0] = sigma_lambda[sigma_lambda.size() - 2];
        sigma_lambda[1] = sigma_lambda[sigma_lambda.size() - 1];

        t_lambda[2] = root - p_at_root / k_at_root;
        sigma_lambda[2] = variable_shift_sigma[2];

        // Return with the convergence code if the sequence has converged.
        if (HasConverged(sigma_lambda)) {
            return QUADRATIC_CONVERGENCE;
        } else if (HasConverged(t_lambda)) {
            return LINEAR_CONVERGENCE;
        }

        // Compute K_next using the formula above.
        UpdateKPolynomialWithQuadraticShift(polynomial_quotient, k_polynomial_quotient);
    }

    return NO_CONVERGENCE;
}

template <typename Real>
bool JenkinsTraubSolver<Real>::ApplyVariableShiftToKPolynomial(const ConvergenceType& fixed_shift_convergence, const std::complex<Real>& root) {
    attempted_linear_shift_ = false;
    attempted_quadratic_shift_ = false;

    if (fixed_shift_convergence == LINEAR_CONVERGENCE) {
        return ApplyLinearShiftToKPolynomial(root, kMaxLinearShiftIterations);
    } else if (fixed_shift_convergence == QUADRATIC_CONVERGENCE) {
        return ApplyQuadraticShiftToKPolynomial(root, kMaxQuadraticShiftIterations);
    }
    return false;
}

// Generate K-polynomials with variable-shifts. During variable shifts, the
// quadratic shift is computed as:
//                | K0(s1)  K0(s2)  z^2 |
//                | K1(s1)  K1(s2)    z |
//                | K2(s1)  K2(s2)    1 |
//    sigma(z) = __________________________
//                  | K1(s1)  K2(s1) |
//                  | K2(s1)  K2(s2) |
// Where K0, K1, and K2 are successive zero-shifts of the K-polynomial.
//
// The K-polynomial shifts are otherwise exactly the same as Stage 2 after accounting for a variable-shift sigma.
template <typename Real>
bool JenkinsTraubSolver<Real>::ApplyQuadraticShiftToKPolynomial(const std::complex<Real>& root, const int max_iterations) {
    using namespace sw::universal;
    using std::abs;

    // Only proceed if we have not already tried a quadratic shift.
    if (attempted_quadratic_shift_) {
        return false;
    }

    const Real kTinyRelativeStep = 0.01;

    // Compute the fixed-shift quadratic:
    // sigma(z) = (x - m - n * i) * (x - m + n * i) = x^2 - 2 * m + m^2 + n^2.
    sigma_[0] = 1.0;
    sigma_[1] = -2.0 * root.real();
    sigma_[2] = root.real() * root.real() + root.imag() * root.imag();

    // These two containers hold values that we test for convergence such that the
    // zero index is the convergence value from 2 iterations ago, the first
    // index is from one iteration ago, and the second index is the current value.
    blas::vector<Real> polynomial_quotient, polynomial_remainder, k_polynomial_quotient, k_polynomial_remainder;
    Real poly_at_root{0.0}, prev_poly_at_root{0.0}, prev_v{0.0};
    bool tried_fixed_shifts = false;

    // These containers maintain a history of the predicted roots. The convergence
    // of the algorithm is determined by the convergence of the root value.
    std::vector<std::complex<Real>> roots1, roots2;
    roots1.push_back(root);
    roots2.push_back(std::conj(root));
    for (int i = 0; i < max_iterations; i++) {
        // Terminate if the root evaluation is within our tolerance. This will
        // return false if we do not have enough samples.
        if (HasRootConverged(roots1) && HasRootConverged(roots2)) {
            AddRootToOutput(roots1[1].real(), roots1[1].imag());
            AddRootToOutput(roots2[1].real(), roots2[1].imag());
            polynomial_ = polynomial_quotient;
            return true;
        }

        QuadraticSyntheticDivision(polynomial_, sigma_, polynomial_quotient, polynomial_remainder);

        // Compute a and b from the above equations.
        b_ = polynomial_remainder[0];
        a_ = polynomial_remainder[1] - b_ * sigma_[1];

        std::vector<std::complex<Real>> roots(2);
        FindQuadraticPolynomialRoots(sigma_[0], sigma_[1], sigma_[2], roots);

        // Check that the roots are close. If not, then try a linear shift.
        if (abs(abs(roots[0].real()) - abs(roots[1].real())) > kRootPairTolerance * abs(roots[1].real())) {

            return ApplyLinearShiftToKPolynomial(root, kMaxLinearShiftIterations);
        }

        // If the iteration is stalling at a root pair then apply a few fixed shift
        // iterations to help convergence.
        poly_at_root = abs(a_ - roots[0].real() * b_) + abs(roots[0].imag() * b_);
        const Real rel_step = abs((sigma_[2] - prev_v) / sigma_[2]);
        if (!tried_fixed_shifts && rel_step < kTinyRelativeStep && prev_poly_at_root > poly_at_root) {
            tried_fixed_shifts = true;
            ApplyFixedShiftToKPolynomial(roots[0], kInnerFixedShiftIterations);
        }

        // Divide the shifted polynomial by the quadratic polynomial.
        QuadraticSyntheticDivision(k_polynomial_, sigma_, k_polynomial_quotient, k_polynomial_remainder);
        d_ = k_polynomial_remainder[0];
        c_ = k_polynomial_remainder[1] - d_ * sigma_[1];

        prev_v = sigma_[2];
        ComputeNextSigma(sigma_);

        // Compute K_next using the formula above.
        UpdateKPolynomialWithQuadraticShift(polynomial_quotient, k_polynomial_quotient);
        k_polynomial_ /= k_polynomial_[0];
        prev_poly_at_root = poly_at_root;

        // Save the roots for convergence testing.
        roots1.push_back(roots[0]);
        roots2.push_back(roots[1]);
        if (roots1.size() > 3) {
            roots1.erase(roots1.begin());
            roots2.erase(roots2.begin());
        }
    }

    attempted_quadratic_shift_ = true;
    return ApplyLinearShiftToKPolynomial(root, kMaxLinearShiftIterations);
}

// Generate K-Polynomials with variable-shifts that are linear. The shift is
// computed as:
//   K_next(z) = 1 / (z - s) * (K(z) - K(s) / P(s) * P(z))
//   s_next = s - P(s) / K_next(s)
template <typename Real>
bool JenkinsTraubSolver<Real>::ApplyLinearShiftToKPolynomial(const std::complex<Real>& root, const int max_iterations) {
    using namespace sw::universal;
    using std::abs;

    if (attempted_linear_shift_) {
        return false;
    }

    // Compute an initial guess for the root.
    Real real_root = (root - EvaluateComplexPolynomial(polynomial_, root) / EvaluateComplexPolynomial(k_polynomial_, root)).real();

    blas::vector<Real> deflated_polynomial, deflated_k_polynomial;
    Real polynomial_at_root{0.0}, k_polynomial_at_root{0.0};

    // This container maintains a history of the predicted roots. The convergence
    // of the algorithm is determined by the convergence of the root value.
    std::vector<Real> roots;
    roots.push_back(real_root);
    for (int i = 0; i < max_iterations; ++i) {
        // Terminate if the root evaluation is within our tolerance. This will
        // return false if we do not have enough samples.
        if (HasRootConverged(roots)) {
            AddRootToOutput(roots[1], 0);
            polynomial_ = deflated_polynomial;
            return true;
        }

        const Real prev_polynomial_at_root = polynomial_at_root;
        SyntheticDivisionAndEvaluate(polynomial_, real_root, deflated_polynomial, polynomial_at_root);

        // If the root is exactly the root then end early. Otherwise, the k
        // polynomial will be filled with inf or nans.
        if (abs(polynomial_at_root) <= kAbsoluteTolerance) {
            AddRootToOutput(real_root, 0);
            polynomial_ = deflated_polynomial;
            return true;
        }

        // Update the K-Polynomial.
        SyntheticDivisionAndEvaluate(k_polynomial_, real_root, deflated_k_polynomial, k_polynomial_at_root);
        k_polynomial_ = AddPolynomials(deflated_k_polynomial, -k_polynomial_at_root / polynomial_at_root * deflated_polynomial);

        k_polynomial_ /= k_polynomial_[0];

        // Compute the update for the root estimation.
        k_polynomial_at_root = EvaluatePolynomial(k_polynomial_, real_root);
        const Real delta_root = polynomial_at_root / k_polynomial_at_root;
        real_root -= polynomial_at_root / k_polynomial_at_root;

        // Save the root so that convergence can be measured. Only the 3 most
        // recently root values are needed.
        roots.push_back(real_root);
        if (roots.size() > 3) {
            roots.erase(roots.begin());
        }

        // If the linear iterations appear to be stalling then we may have found a
        // Real real root of the form (z - x^2). Attempt a quadratic variable
        // shift from the current estimate of the root.
        if (i >= 2 &&
            abs(delta_root) < 0.001 * abs(real_root) &&
            abs(prev_polynomial_at_root) < abs(polynomial_at_root)) {
            const std::complex<Real> new_root(real_root, 0);
            return ApplyQuadraticShiftToKPolynomial(new_root, kMaxQuadraticShiftIterations);
        }
    }

    attempted_linear_shift_ = true;
    return ApplyQuadraticShiftToKPolynomial(root, kMaxQuadraticShiftIterations);
}

template <typename Real>
void JenkinsTraubSolver<Real>::AddRootToOutput(const Real real, const Real imag) {
    real_roots_[num_solved_roots_] = real;
    complex_roots_[num_solved_roots_] = imag;

    ++num_solved_roots_;
}

template <typename Real>
void JenkinsTraubSolver<Real>::RemoveZeroRoots() {
    int num_zero_roots = 0;

    while (polynomial_[polynomial_.size() - 1 - num_zero_roots] == 0) {
        ++num_zero_roots; 
    }
    

    // The output roots have 0 as the default value so there is no need to explicitly add the zero roots.
    polynomial_.resize(polynomial_.size() - num_zero_roots); // TODO: made a bit of a change here (was .head().eval()), needs verification
}

template <typename Real>
bool JenkinsTraubSolver<Real>::SolveClosedFormPolynomial() {
    const int degree = polynomial_.size() - 1;

    // Is the polynomial constant?
    if (degree == 0) {
        std::cout << "Trying to extract roots from a constant polynomial in FindPolynomialRoots\n";
        // We return true with no roots, not false, as if the polynomial is constant
        // it is correct that there are no roots. It is not the case that they were
        // there, but that we have failed to extract them.
        return true;
    }

    // Linear
    if (degree == 1) {
        AddRootToOutput(FindLinearPolynomialRoots(polynomial_[0], polynomial_[1]), 0);
        return true;
    }

    // Quadratic
    if (degree == 2) {
        std::vector<std::complex<Real>> roots(2);
        FindQuadraticPolynomialRoots(polynomial_[0], polynomial_[1], polynomial_[2], roots);
        AddRootToOutput(roots[0].real(), roots[0].imag());
        AddRootToOutput(roots[1].real(), roots[1].imag());
        return true;
    }

    return false;
}

// Computes a lower bound on the radius of the roots of polynomial by examining
// the Cauchy sequence:
//
//    z^n + |a_1| * z^{n - 1} + ... + |a_{n-1}| * z - |a_n|
//
// The unique positive zero of this polynomial is an approximate lower bound of
// the radius of zeros of the original polynomial.
template <typename Real>
Real JenkinsTraubSolver<Real>::ComputeRootRadius() {
    static const Real kEpsilon = 1e-2;
    static const int kMaxIterations = 100;

    sw::universal::blas::vector<Real> poly = polynomial_;
    // Take the absolute value of all coefficients.
    for (size_t i = 0; i < poly.size(); ++i){
        poly[i] = abs(poly[i]); // TODO: might need sw::universal here...
    }
    // Negate the last coefficient.
    poly[poly.size() - 1] *= -1.0;

    // Find the unique positive zero using Newton-Raphson iterations.
    Real x0 = 1.0;
    return FindRootIterativeNewton(poly, x0, kEpsilon, kMaxIterations);
}

// The k polynomial with a zero-shift is
//  (K(x) - K(0) / P(0) * P(x)) / x.
//
// This is equivalent to:
//    K(x) - K(0)      K(0)     P(x) - P(0)
//    ___________   -  ____  *  ___________
//         x           P(0)          x
//
// Note that removing the constant term and dividing by x is equivalent to
// shifting the polynomial to one degree lower in our representation.
template <typename Real>
void JenkinsTraubSolver<Real>::ComputeZeroShiftKPolynomial() {
    // Evaluating the polynomial at zero is equivalent to the constant term
    // (i.e. the last coefficient). Note that reverse() is an expression and does
    // not actually reverse the vector elements.
    const Real polynomial_at_zero = polynomial_[polynomial_.size() - 1];
    const Real k_at_zero = k_polynomial_[k_polynomial_.size() - 1];

    k_polynomial_ = AddPolynomials(Head(k_polynomial_, k_polynomial_.size() - 1), -k_at_zero / polynomial_at_zero * Head(polynomial_, polynomial_.size() - 1));
}

// The iterations are computed with the following equation:
//              a^2 + u * a * b + v * b^2
//   K_next =  ___________________________ * Q_K
//                    b * c - a * d
//
//                      a * c + u * a * d + v * b * d
//             +  (z - _______________________________) * Q_P + b.
//                              b * c - a * d
//
// This is done using *only* realy arithmetic so it can be done very fast!
template <typename Real>
void JenkinsTraubSolver<Real>::UpdateKPolynomialWithQuadraticShift(const sw::universal::blas::vector<Real>& polynomial_quotient,
                                                                   const sw::universal::blas::vector<Real>& k_polynomial_quotient) {
    
    const Real coefficient_q_k = (a_ * a_ + sigma_[1] * a_ * b_ + sigma_[2] * b_ * b_) / (b_ * c_ - a_ * d_); // TODO: next segfault here
    sw::universal::blas::vector<Real> linear_polynomial(2);
    linear_polynomial[0] = 1.0;
    linear_polynomial[1] = -(a_ * c_ + sigma_[1] * a_ * d_ + sigma_[2] * b_ * d_) / (b_ * c_ - a_ * d_);
    
    k_polynomial_ = AddPolynomials(coefficient_q_k * k_polynomial_quotient, MultiplyPolynomials(linear_polynomial, polynomial_quotient));    
    k_polynomial_[k_polynomial_.size() - 1] += b_;
}

// Using a bit of algebra, the update of sigma(z) can be computed from the
// previous value along with a, b, c, and d defined above. The details of this
// simplification can be found in "Three Stage Variable-Shift Iterations for the
// Solution of Polynomial Equations With a Posteriori Error Bounds for the
// Zeros" by M.A. Jenkins, Doctoral Thesis, Stanford Univeristy, 1969.
//
// NOTE: we assume the leading term of quadratic_sigma is 1.0.
template <typename Real>
void JenkinsTraubSolver<Real>::ComputeNextSigma(sw::universal::blas::vector<Real>& nextSigma) {
    const Real u = sigma_[1];
    const Real v = sigma_[2];

    const Real b1 = -k_polynomial_[k_polynomial_.size() - 1] / polynomial_[polynomial_.size() - 1];
    const Real b2 = -(k_polynomial_[k_polynomial_.size() - 2] + b1 * polynomial_[polynomial_.size() - 2]) / polynomial_[polynomial_.size() - 1];

    const Real a1 = b_* c_ - a_ * d_;
    const Real a2 = a_ * c_ + u * a_ * d_ + v * b_* d_;
    const Real c2 = b1 * a2;
    const Real c3 = b1 * b1 * (a_ * a_ + u * a_ * b_ + v * b_ * b_);
    const Real c4 = v * b2 * a1 - c2 - c3;
    const Real c1 = c_ * c_ + u * c_ * d_ + v * d_ * d_ +
                    b1 * (a_ * c_ + u * b_ * c_ + v * b_ * d_) - c4;
    const Real delta_u = -(u * (c2 + c3) + v * (b1 * a1 + b2 * a2)) / c1;
    const Real delta_v = v * c4 / c1;

    // Update u and v in the quadratic sigma.
    nextSigma[0] = 1.0;
    nextSigma[1] = u + delta_u;
    nextSigma[2] = v + delta_v;
}

template <typename Real>
bool FindPolynomialRootsJenkinsTraub(const sw::universal::blas::vector<Real>& polynomial, sw::universal::blas::vector<Real>& real_roots, sw::universal::blas::vector<Real>& complex_roots) {
    JenkinsTraubSolver<Real> solver(polynomial, real_roots, complex_roots);
    return solver.ExtractRoots();
}

// cd build/mixedprecision/roots
// make mp_rpoly
// ./mp_rpoly
int main(int argc, char** argv) 
try {
	using namespace sw::universal;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

    // {
    //     using Vector = blas::vector<double>;

    //     Vector poly = {20.0, 4.0, -1.0, 0.0, -1.0, -0.2};
    //     Vector realRoots, complexRoots;

    //     FindPolynomialRootsJenkinsTraub(poly, realRoots, complexRoots);
    //     std::cout << realRoots << '\n';
    //     std::cout << complexRoots << '\n';
    // }
    // {
    //     using Vector = blas::vector<posit<32, 2>>;

    //     Vector poly = {20.0, 4.0, -1.0, 0.0, -1.0, -0.2};
    //     Vector realRoots, complexRoots;

    //     FindPolynomialRootsJenkinsTraub(poly, realRoots, complexRoots);
    //     std::cout << realRoots << '\n';
    //     std::cout << complexRoots << '\n';
    // }
    // {
    //     using Real = posit<32, 2>;
    //     using Vector = blas::vector<Real>;

    //     // (x-4)(x+0.5)(x-8)
    //     Vector root1 = {1, -4};
    //     Vector root2 = {1, 0.5};
    //     Vector root3 = {1, -8};
    //     Vector poly = MultiplyPolynomials(MultiplyPolynomials(root1, root2), root3);

    //     Vector realRoots, complexRoots;

    //     FindPolynomialRootsJenkinsTraub(poly, realRoots, complexRoots);
    //     std::cout << realRoots << '\n';
    //     std::cout << complexRoots << '\n';
    // }
    {
        using Real = double;
        using Vector = blas::vector<Real>;

        // (x-6)(x+1.5)(x-8)
        Vector root1 = {1, -6};
        Vector root2 = {1, 1.5};
        Vector root3 = {1, -8};
        Vector poly = MultiplyPolynomials(MultiplyPolynomials(root1, root2), root3);

        Vector realRoots, complexRoots;

        FindPolynomialRootsJenkinsTraub(poly, realRoots, complexRoots);
        std::cout << realRoots << '\n';
        std::cout << complexRoots << '\n';
    }
    {
        // Just to compare to 28-bit soft-float
        using Real = posit<28, 2>;
        using Vector = blas::vector<Real>;

        // (x-6)(x+1.5)(x-8)
        Vector root1 = {1, -6};
        Vector root2 = {1, 1.5};
        Vector root3 = {1, -8};
        Vector poly = MultiplyPolynomials(MultiplyPolynomials(root1, root2), root3);

        Vector realRoots, complexRoots;

        FindPolynomialRootsJenkinsTraub(poly, realRoots, complexRoots);
        std::cout << realRoots << '\n';
        std::cout << complexRoots << '\n';
    }
    {
        constexpr bool hasSubnormal = true;
        constexpr bool hasSupernormal = true;
        constexpr bool isSaturating = false;
        // Just chose arbitrary 'nbits' and 'es' to show soft-float
        using Real = cfloat<28, 10, std::uint16_t, hasSubnormal, hasSupernormal, isSaturating>;
        using Vector = blas::vector<Real>;

        // (x-6)(x+1.5)(x-8)
        Vector root1 = {1, -6};
        Vector root2 = {1, 1.5};
        Vector root3 = {1, -8};
        Vector poly = MultiplyPolynomials(MultiplyPolynomials(root1, root2), root3);

        Vector realRoots, complexRoots;

        FindPolynomialRootsJenkinsTraub(poly, realRoots, complexRoots);
        std::cout << realRoots << '\n';
        std::cout << complexRoots << '\n';
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
