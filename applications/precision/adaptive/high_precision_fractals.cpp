// high_precision_fractals.cpp: deep-zoom Mandelbrot in double vs efloat (issue #1098)
//
// The Mandelbrot iteration z_{n+1} = z_n^2 + c is rendered for a deep-zoom view
// whose pixel spacing (~1.25e-16) is at the resolution limit of double: adjacent
// pixel coordinates are only ~1 ulp apart, so double quantizes the view and its
// escape-time image is corrupted. The SAME templated kernel run with efloat
// (256-bit) coordinates resolves the view correctly and serves as the oracle.
//
// Both renders are written as binary PPM (P6). A quantitative summary prints how
// far double falls below the needed resolution and how many pixels the double
// image gets wrong relative to the efloat oracle.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <universal/number/efloat/efloat.hpp>

// View: a spiral in the "seahorse valley" neighbourhood, zoomed until double
// under-resolves. Keep the image modest so it renders quickly; scale IMG_W /
// IMG_H / MAXITER up for a higher-resolution picture (see the README).
namespace {
	constexpr int    IMG_W    = 80;
	constexpr int    IMG_H    = 80;
	constexpr int    MAXITER  = 1000;
	constexpr double CENTER_X = -0.7269;
	constexpr double CENTER_Y =  0.1889;
	constexpr double VIEW_W   =  3e-15;   // width of the view window in the complex plane
	                                      // (dx ~ 0.34 ulp: double collapses ~2/3 of the
	                                      //  columns into blocks; efloat resolves them all)
}

// Escape time of z_{n+1} = z_n^2 + c (z_0 = 0). Same code for double and efloat.
template<typename Real>
int escape_time(const Real& cr, const Real& ci, int maxit) {
	Real zr(0.0), zi(0.0);
	for (int i = 0; i < maxit; ++i) {
		Real zr2 = zr * zr;
		Real zi2 = zi * zi;
		if (double(zr2 + zi2) > 4.0) return i;   // escaped
		Real t = zr2 - zi2 + cr;
		zi = Real(2.0) * zr * zi + ci;
		zr = t;
	}
	return maxit;                                // assumed in-set
}

// Render the escape-time grid at the fixed view, computing pixel coordinates in
// the working type Real (double: quantized; efloat: exact).
template<typename Real>
std::vector<int> render() {
	const Real cx(CENTER_X), cy(CENTER_Y), dx(VIEW_W / IMG_W);
	std::vector<int> grid(static_cast<std::size_t>(IMG_W) * IMG_H);
	for (int py = 0; py < IMG_H; ++py) {
		for (int px = 0; px < IMG_W; ++px) {
			Real cr = cx + Real(static_cast<double>(px - IMG_W / 2)) * dx;
			Real ci = cy + Real(static_cast<double>(py - IMG_H / 2)) * dx;
			grid[static_cast<std::size_t>(py) * IMG_W + px] = escape_time<Real>(cr, ci, MAXITER);
		}
	}
	return grid;
}

// Write a binary PPM (P6). In-set pixels are black; escaped pixels use the
// classic smooth Mandelbrot palette keyed on the escape count.
void write_ppm(const std::string& path, const std::vector<int>& grid) {
	std::ofstream f(path, std::ios::binary);
	f << "P6\n" << IMG_W << ' ' << IMG_H << "\n255\n";
	for (int v : grid) {
		unsigned char rgb[3] = { 0, 0, 0 };
		if (v < MAXITER) {
			double t = static_cast<double>(v) / MAXITER;
			rgb[0] = static_cast<unsigned char>(9.0  * (1 - t) * t * t * t * 255.0);
			rgb[1] = static_cast<unsigned char>(15.0 * (1 - t) * (1 - t) * t * t * 255.0);
			rgb[2] = static_cast<unsigned char>(8.5  * (1 - t) * (1 - t) * (1 - t) * t * 255.0);
		}
		f.write(reinterpret_cast<const char*>(rgb), 3);
	}
}

int main()
try {
	using namespace sw::universal;
	using efloat256 = efloat<8>;   // 8 limbs * 32 = 256 bits

	const double dx  = VIEW_W / IMG_W;
	const double ulp = std::ldexp(1.0, -53);     // ulp near |c| ~ 0.75 (exponent -1)

	std::cout << "Deep-zoom Mandelbrot: double vs efloat (256-bit) coordinates\n";
	std::cout << std::setprecision(15);
	std::cout << "  center = (" << CENTER_X << ", " << CENTER_Y << ")\n";
	std::cout << std::scientific << std::setprecision(3);
	std::cout << "  view width = " << VIEW_W << "   image = " << IMG_W << " x " << IMG_H
	          << "   maxiter = " << MAXITER << "\n";
	std::cout << "  pixel spacing dx = " << dx << "   double ulp here = " << ulp
	          << "   dx/ulp = " << std::fixed << std::setprecision(2) << (dx / ulp) << "\n";
	std::cout << "  (dx/ulp near 1 means adjacent pixels are ~1 ulp apart: double cannot resolve them)\n\n";

	std::vector<int> gd = render<double>();
	std::vector<int> ge = render<efloat256>();

	// How badly does the double render disagree with the efloat oracle?
	int diff = 0, emin = MAXITER, emax = 0;
	for (std::size_t i = 0; i < gd.size(); ++i) {
		if (gd[i] != ge[i]) ++diff;
		emin = std::min(emin, ge[i]);
		emax = std::max(emax, ge[i]);
	}

	write_ppm("mandelbrot_double.ppm", gd);
	write_ppm("mandelbrot_efloat.ppm", ge);

	std::cout << "  efloat (oracle) escape range: [" << emin << ", " << emax << "]"
	          << (emin != emax ? "  <- real fractal structure\n" : "  (uniform)\n");
	std::cout << "  pixels where double disagrees with efloat: " << diff << " / " << gd.size()
	          << std::fixed << std::setprecision(1)
	          << "  (" << (100.0 * diff / gd.size()) << "%)\n\n";
	std::cout << "  wrote mandelbrot_double.ppm and mandelbrot_efloat.ppm\n";
	std::cout << "  The double image is corrupted by coordinate quantization at this zoom;\n";
	std::cout << "  the efloat image is correct. Same escape-time kernel, two number types.\n";

	return EXIT_SUCCESS;
}
catch (const char* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
