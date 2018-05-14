// l1_fused_dot.cpp: example program showing a fused-dot product for error free linear algebra
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the mathematical constants in cmath: old-style preprocessor magic which isn't best practice anymore
#define _USE_MATH_DEFINES
#include "common.hpp"

#define POSIT_TRACE_DEBUG
#define POSIT_TRACE_MUL
#include <posit>

#include "blas.hpp"


const double pi = 3.14159265358979323846;  // best practice for C++

template<typename Ty>
Ty minValue(const std::vector<Ty>& samples) {
	typename std::vector<Ty>::const_iterator it = min_element(samples.begin(), samples.end());
	return *it;
}
template<typename Ty>
Ty maxValue(const std::vector<Ty>& samples) {
	typename std::vector<Ty>::const_iterator it = max_element(samples.begin(), samples.end());
	return *it;
}
template<typename Ty>
std::ostream& DisplaySample(std::ostream& ostr, const Ty& value, const Ty& min, const Ty& max) {
	// min is 0 stars
	// 0   is 40 stars
	// max is 80 stars
	int maxStars = 80;
	float sample = float(value);
	float range = float(max - min);
	float midPoint = range/2.0f;
	float portion = (sample + midPoint)/range;
	int ub = int(maxStars * portion);
	for (int i = 0; i < ub; i++) {
		ostr << "*";
	}
	return ostr << std::endl;
}

template<typename Ty>
void DisplaySignal(std::ostream& ostr, const std::vector<Ty>& samples) {
	Ty min = minValue(samples);
	Ty max = maxValue(samples);
	// create a horizontal display
	int cnt = 0;
	ostr << std::fixed << std::setprecision(3);
	for (typename std::vector<Ty>::const_iterator it = samples.begin(); it != samples.end(); it++) {
		ostr << std::setw(3) << cnt++ << " " << std::setw(6) << float(*it) << " ";
		DisplaySample(ostr, *it, min, max);
	}
	ostr << std::setprecision(5);
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::blas;

	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	constexpr size_t capacity = 6;   // 2^3 accumulations of maxpos^2
	constexpr size_t vecSizePwr = 5;
	constexpr size_t vecSize = (size_t(1) << vecSizePwr);

	// generate an interesting vector x with 0.5 ULP round-off errors in each product
	// that the fused-dot product will be able to resolve
	// by progressively adding smaller values, a regular dot product loses these bits due to canceleation.
	// but a fused dot product leveraging a quire will be able to resolve these.
	float eps      = std::numeric_limits<float>::epsilon();
	float epsminus = 1.0f - eps;
	float epsplus  = 1.0f + eps;

	typedef float        IEEEType;
	typedef posit<27, 1> PositType;
	vector<IEEEType> xieee = { epsplus,	epsplus, epsplus, epsplus, epsplus };
	vector<IEEEType> yieee = { 1.5f, 1.25f, 1.125f, 1.0625f, 1.03125f };
	vector<PositType> xposit = { epsplus,	epsplus, epsplus, epsplus, epsplus };
	vector<PositType> yposit = { 1.5f, 1.25f, 1.125f, 1.0625f, 1.03125f };
	cout << setprecision(17);
	cout << "dot(x,y)      : " << sw::blas::dot(xieee.size(), xieee, 1, yieee, 1) << endl;
	cout << "fused_dot(x,y): " << sw::blas::fused_dot(xposit.size(), xposit, 1, yposit, 1) << endl;
	cout << setprecision(5);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
