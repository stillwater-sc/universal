//  equivalence.cpp : do the classic and cascade multi-component types compute the same answer?
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// issue #1315: this is the guardrail for the performance comparison next door. A speed difference
// between dd and dd_cascade is only meaningful if both are doing the same job. If one of them
// renormalizes less aggressively, or truncates a series earlier, it will be faster and less
// accurate, and reporting that as a win would be wrong.
//
// So: run both implementations of a pair over the same operands and report, per operation,
// how often they agree bit-for-bit and how far apart they are when they do not. Divergence is
// reported, not judged - the classic and cascade families implement different algorithms
// (Bailey/Hida vs Priest renormalization), and where they disagree is exactly the information
// needed to decide whether a benchmark row is a fair comparison.
//
// The distance metric is the difference of the two multi-component values, expressed in units of
// the last place of the target format's significand (106 bits for dd, 159 for td, 212 for qd).
// The limb differences are accumulated in double: the leading limbs cancel to the size of the
// trailing limbs, so this is accurate enough for a diagnostic magnitude even though it is not an
// exact expansion subtraction.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "hp_types.hpp"

namespace {

	constexpr std::size_t NR_SAMPLES = 4096;
	constexpr std::size_t DOT_LENGTH = 256;
	constexpr std::size_t HORNER_DEGREE = 20;

	// catastrophic disagreement: the two implementations are not computing the same thing,
	// which invalidates the benchmark rather than merely characterizing it
	constexpr double RELATIVE_ALARM = 1.0e-9;

	const std::vector<double>& lhsData() {
		static const std::vector<double> data = hpbench::sampleData(NR_SAMPLES, 0xA1A1ull);
		return data;
	}

	const std::vector<double>& rhsData() {
		static const std::vector<double> data = hpbench::sampleData(NR_SAMPLES, 0xB2B2ull);
		return data;
	}

	struct Divergence {
		std::string op;
		std::string pair;
		std::size_t samples{ 0 };
		std::size_t identical{ 0 };
		double      maxUlps{ 0.0 };
		double      maxRelative{ 0.0 };
		bool        structural{ false };   // one produced a NaN or infinity where the other did not
	};

	// relative difference of two multi-component values, and the same difference expressed in
	// ulps of a significand of 'precisionBits' bits
	template<typename Classic, typename Cascade>
	void accumulateDistance(Classic& a, Cascade& b, unsigned nrLimbs, double precisionBits, Divergence& d) {
		++d.samples;
		bool identical = true;
		double difference{ 0.0 };
		for (unsigned i = 0; i < nrLimbs; ++i) {
			double x = a[i];
			double y = b[i];
			if (std::isnan(x) != std::isnan(y) || std::isinf(x) != std::isinf(y)) {
				d.structural = true;
				return;
			}
			if (x != y) identical = false;
			difference += x - y;
		}
		if (identical) {
			++d.identical;
			return;
		}
		double magnitude = std::fabs(double(a[0]));
		double relative = (magnitude > 0.0) ? std::fabs(difference) / magnitude : std::fabs(difference);
		if (relative > d.maxRelative) d.maxRelative = relative;
		double ulps = relative * std::pow(2.0, precisionBits);
		if (ulps > d.maxUlps) d.maxUlps = ulps;
	}

	template<typename Classic, typename Cascade, typename BinaryOp>
	Divergence compareBinary(const std::string& op, const std::string& pair, unsigned nrLimbs, double precisionBits, BinaryOp f) {
		const std::vector<double>& lhs = lhsData();
		const std::vector<double>& rhs = rhsData();
		Divergence d;
		d.op = op;
		d.pair = pair;
		for (std::size_t i = 0; i < lhs.size(); ++i) {
			Classic classic = f(Classic(lhs[i]), Classic(rhs[i]));
			Cascade cascade = f(Cascade(lhs[i]), Cascade(rhs[i]));
			accumulateDistance(classic, cascade, nrLimbs, precisionBits, d);
		}
		return d;
	}

	template<typename Classic, typename Cascade, typename UnaryOp>
	Divergence compareUnary(const std::string& op, const std::string& pair, unsigned nrLimbs, double precisionBits, UnaryOp f) {
		const std::vector<double>& lhs = lhsData();
		Divergence d;
		d.op = op;
		d.pair = pair;
		for (std::size_t i = 0; i < lhs.size(); ++i) {
			Classic classic = f(Classic(lhs[i]));
			Cascade cascade = f(Cascade(lhs[i]));
			accumulateDistance(classic, cascade, nrLimbs, precisionBits, d);
		}
		return d;
	}

	// the two composite kernels from kernels.cpp, so the guardrail covers what is benchmarked
	template<typename Scalar>
	Scalar dotKernel(const std::vector<double>& x, const std::vector<double>& y, std::size_t offset, std::size_t N) {
		Scalar sum(0.0);
		for (std::size_t i = 0; i < N; ++i) {
			sum = sum + Scalar(x[offset + i]) * Scalar(y[offset + i]);
		}
		return sum;
	}

	template<typename Scalar>
	Scalar hornerKernel(const std::vector<double>& coefficients, std::size_t offset, std::size_t degree) {
		Scalar x(0.5);
		Scalar p(coefficients[offset + degree]);
		for (std::size_t i = degree; i > 0; --i) {
			p = p * x + Scalar(coefficients[offset + i - 1]);
		}
		return p;
	}

	template<typename Classic, typename Cascade>
	Divergence compareDot(const std::string& pair, unsigned nrLimbs, double precisionBits) {
		const std::vector<double>& x = lhsData();
		const std::vector<double>& y = rhsData();
		Divergence d;
		d.op = "dot N=256";
		d.pair = pair;
		for (std::size_t offset = 0; offset + DOT_LENGTH <= x.size(); offset += DOT_LENGTH) {
			Classic classic = dotKernel<Classic>(x, y, offset, DOT_LENGTH);
			Cascade cascade = dotKernel<Cascade>(x, y, offset, DOT_LENGTH);
			accumulateDistance(classic, cascade, nrLimbs, precisionBits, d);
		}
		return d;
	}

	template<typename Classic, typename Cascade>
	Divergence compareHorner(const std::string& pair, unsigned nrLimbs, double precisionBits) {
		const std::vector<double>& c = lhsData();
		Divergence d;
		d.op = "horner deg 20";
		d.pair = pair;
		for (std::size_t offset = 0; offset + HORNER_DEGREE + 1 <= c.size(); offset += HORNER_DEGREE + 1) {
			Classic classic = hornerKernel<Classic>(c, offset, HORNER_DEGREE);
			Cascade cascade = hornerKernel<Cascade>(c, offset, HORNER_DEGREE);
			accumulateDistance(classic, cascade, nrLimbs, precisionBits, d);
		}
		return d;
	}

	template<typename Classic, typename Cascade>
	void comparePair(const std::string& pair, unsigned nrLimbs, double precisionBits, std::vector<Divergence>& results) {
		results.push_back(compareBinary<Classic, Cascade>("add", pair, nrLimbs, precisionBits, [](auto a, auto b) { return a + b; }));
		results.push_back(compareBinary<Classic, Cascade>("subtract", pair, nrLimbs, precisionBits, [](auto a, auto b) { return a - b; }));
		results.push_back(compareBinary<Classic, Cascade>("multiply", pair, nrLimbs, precisionBits, [](auto a, auto b) { return a * b; }));
		results.push_back(compareBinary<Classic, Cascade>("divide", pair, nrLimbs, precisionBits, [](auto a, auto b) { return a / b; }));
		results.push_back(compareUnary<Classic, Cascade>("sqrt", pair, nrLimbs, precisionBits, [](auto a) { using std::sqrt; return sqrt(a); }));
		results.push_back(compareUnary<Classic, Cascade>("exp", pair, nrLimbs, precisionBits, [](auto a) { using std::exp; return exp(a); }));
		results.push_back(compareUnary<Classic, Cascade>("log", pair, nrLimbs, precisionBits, [](auto a) { using std::log; return log(a); }));
		results.push_back(compareUnary<Classic, Cascade>("sin", pair, nrLimbs, precisionBits, [](auto a) { using std::sin; return sin(a); }));
		results.push_back(compareUnary<Classic, Cascade>("cos", pair, nrLimbs, precisionBits, [](auto a) { using std::cos; return cos(a); }));
		results.push_back(compareDot<Classic, Cascade>(pair, nrLimbs, precisionBits));
		results.push_back(compareHorner<Classic, Cascade>(pair, nrLimbs, precisionBits));
	}

	// Where the two families disagree, 'they differ' is not actionable on its own: one of them is
	// wrong and the benchmark reader needs to know which. These identities are evaluated entirely
	// inside a single type, so they need no external oracle:
	//
	//     sqrt(x)^2 == x        exp(log(x)) == x        sin(x)^2 + cos(x)^2 == 1
	//
	// A correct implementation leaves a residual of a few ulps (the identity itself rounds). A
	// residual many orders of magnitude larger means that implementation is losing digits, and any
	// speed advantage it shows in the benchmark next door is bought with accuracy.
	struct Residual {
		std::string type;
		double sqrtUlps{ 0.0 };
		double expLogUlps{ 0.0 };
		double pythagorasUlps{ 0.0 };
	};

	// relative difference of two values of the same type, evaluated in that type and then read out
	// as a double: the subtraction of two nearby multi-component values is exact in its leading
	// components, which is all the magnitude needs
	template<typename Scalar>
	double relativeDifference(Scalar a, Scalar b) {
		Scalar difference = a - b;
		double denominator = std::fabs(double(b));
		if (denominator == 0.0) return std::fabs(double(difference));
		return std::fabs(double(difference)) / denominator;
	}

	template<typename Scalar>
	Residual residuals(const std::string& type, double precisionBits) {
		using std::sqrt; using std::exp; using std::log; using std::sin; using std::cos;
		const std::vector<double>& data = lhsData();
		const double ulp = std::pow(2.0, precisionBits);
		Residual r;
		r.type = type;
		for (std::size_t i = 0; i < data.size(); ++i) {
			Scalar x(data[i]);
			Scalar root = sqrt(x);
			r.sqrtUlps = std::max(r.sqrtUlps, relativeDifference(root * root, x) * ulp);
			Scalar roundtrip = exp(log(x));
			r.expLogUlps = std::max(r.expLogUlps, relativeDifference(roundtrip, x) * ulp);
			Scalar s = sin(x);
			Scalar c = cos(x);
			r.pythagorasUlps = std::max(r.pythagorasUlps, relativeDifference(s * s + c * c, Scalar(1.0)) * ulp);
		}
		return r;
	}

	void reportResiduals(const std::vector<Residual>& residuals) {
		std::cout << "\nself-consistency residuals (max ulps of the type's own significand)\n";
		std::cout << std::left << std::setw(16) << "type" << std::right
			<< std::setw(18) << "sqrt(x)^2 - x"
			<< std::setw(18) << "exp(log(x)) - x"
			<< std::setw(22) << "sin^2 + cos^2 - 1" << '\n';
		std::cout << std::string(74, '-') << '\n';
		for (const auto& r : residuals) {
			std::cout << std::left << std::setw(16) << r.type << std::right << std::defaultfloat << std::setprecision(4)
				<< std::setw(18) << r.sqrtUlps
				<< std::setw(18) << r.expLogUlps
				<< std::setw(22) << r.pythagorasUlps << '\n';
		}
	}

	void report(const std::vector<Divergence>& results) {
		std::cout << std::left << std::setw(16) << "operation" << std::setw(24) << "pair"
			<< std::right << std::setw(10) << "samples" << std::setw(16) << "bit-identical"
			<< std::setw(16) << "max ulp diff" << std::setw(14) << "max rel diff" << '\n';
		std::cout << std::string(96, '-') << '\n';
		for (const auto& d : results) {
			std::cout << std::left << std::setw(16) << d.op << std::setw(24) << d.pair << std::right
				<< std::setw(10) << d.samples
				<< std::setw(16) << d.identical;
			if (d.structural) {
				std::cout << std::setw(16) << "NaN/inf" << std::setw(14) << "mismatch";
			}
			else if (d.identical == d.samples) {
				std::cout << std::setw(16) << "0" << std::setw(14) << "0";
			}
			else {
				// the ulp distance runs from fractions of an ulp to 1e18 when an implementation
				// only agrees in its leading limbs, so it needs a scale-free format
				std::cout << std::setw(16) << std::defaultfloat << std::setprecision(4) << d.maxUlps
					<< std::setw(14) << std::scientific << std::setprecision(2) << d.maxRelative;
			}
			std::cout << '\n';
		}
	}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;

	std::cout << "classic vs cascade equivalence over the benchmark operands (universal#1315)\n";
	std::cout << "==========================================================================\n";
	std::cout << "  samples/op     : " << NR_SAMPLES << " operand pairs drawn from [0.5, 2.0)\n";
	std::cout << "  ulp reference  : the last place of the target significand, 106 bits for dd,"
		<< " 159 for td, 212 for qd\n";
	std::cout << "  interpretation : bit-identical == samples means the benchmark compares two\n";
	std::cout << "                   implementations of the same computation. A few ulps means the\n";
	std::cout << "                   families renormalize differently. Orders of magnitude means the\n";
	std::cout << "                   corresponding benchmark row is not a like-for-like comparison.\n\n";

	std::vector<Divergence> results;
	comparePair<dd, dd_cascade>("dd vs dd_cascade", 2, 106.0, results);
	comparePair<qd, qd_cascade>("qd vs qd_cascade", 4, 212.0, results);
	// td_cascade has no classic counterpart; compare it against dd to show that the third limb is
	// carrying real information rather than noise (these are expected to differ by construction)
	comparePair<dd, td_cascade>("dd vs td_cascade", 2, 106.0, results);

	report(results);

	std::vector<Residual> selfConsistency;
	selfConsistency.push_back(residuals<double>("double", 53.0));
	selfConsistency.push_back(residuals<dd>("dd", 106.0));
	selfConsistency.push_back(residuals<dd_cascade>("dd_cascade", 106.0));
	selfConsistency.push_back(residuals<td_cascade>("td_cascade", 159.0));
	selfConsistency.push_back(residuals<qd>("qd", 212.0));
	selfConsistency.push_back(residuals<qd_cascade>("qd_cascade", 212.0));
	reportResiduals(selfConsistency);

	bool alarm{ false };
	for (const auto& d : results) {
		if (d.structural) {
			std::cout << "\nFAIL: " << d.pair << " / " << d.op << " produced a NaN or infinity in one implementation only\n";
			alarm = true;
		}
		// the dd vs td_cascade rows are expected to differ beyond dd's precision: td carries more
		if (d.pair != "dd vs td_cascade" && d.maxRelative > RELATIVE_ALARM) {
			std::cout << "\nFAIL: " << d.pair << " / " << d.op << " differ by " << d.maxRelative
				<< " relative, which is far above the " << RELATIVE_ALARM << " alarm threshold:\n"
				<< "      these are not two implementations of the same computation, and the\n"
				<< "      corresponding benchmark row cannot be read as a like-for-like comparison\n";
			alarm = true;
		}
	}

	std::cout << (alarm ? "\nequivalence check FAILED\n" : "\nequivalence check passed\n");
	return alarm ? EXIT_FAILURE : EXIT_SUCCESS;
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
