// mathematical_identities.cpp: verify the BBP series for pi with double vs efloat (issue #1099)
//
// The Bailey-Borwein-Plouffe (BBP) formula
//
//   pi = sum_{n>=0} 1/16^n ( 4/(8n+1) - 2/(8n+4) - 1/(8n+5) - 1/(8n+6) )
//
// converges at ~1.2 decimal digits per term. Evaluating it in double bottoms out
// at ~15-16 digits (double's ceiling); the SAME templated summation in efloat
// keeps gaining digits with each term. Both are checked against an independent
// oracle -- efloat_pi(), a stored ~1000-digit constant -- so this program
// numerically verifies the identity and shows how far each type can confirm it.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <iomanip>
#include <universal/number/efloat/efloat.hpp>

// Sum the first `terms` terms of the BBP series for pi. Same code on double and
// efloat; the running 1/16^n factor is carried as a reciprocal to avoid growth.
template<typename Real>
Real bbp_pi(int terms) {
	Real sum(0.0);
	Real inv16(1.0);  // 16^-n, updated each term
	for (int n = 0; n < terms; ++n) {
		Real term = Real(4.0) / Real(static_cast<double>(8 * n + 1)) -
		            Real(2.0) / Real(static_cast<double>(8 * n + 4)) -
		            Real(1.0) / Real(static_cast<double>(8 * n + 5)) - Real(1.0) / Real(static_cast<double>(8 * n + 6));
		sum   = sum + term * inv16;
		inv16 = inv16 / Real(16.0);
	}
	return sum;
}

namespace {
using efloat512 = sw::universal::efloat<16>;  // 16 limbs * 32 = 512 bits (~154 digits)

// Decimal digits to which `value` matches the oracle pi (the higher, the better).
int agreement_digits(const efloat512& value, const efloat512& oracle) {
	efloat512 d = value - oracle;
	d.setsign(false);
	if (d.iszero())
		return 154;  // limited by the 512-bit oracle
	int digits = static_cast<int>(-static_cast<double>(d.scale()) * 0.301029995663981);
	return digits < 0 ? 0 : digits;
}
}

int main() try {
	using namespace sw::universal;

	const efloat512 oracle = efloat_pi<16>();  // independent reference (~154 digits at 512 bits)

	std::cout << "Verifying the BBP series for pi:\n";
	std::cout << "  pi = sum 1/16^n ( 4/(8n+1) - 2/(8n+4) - 1/(8n+5) - 1/(8n+6) )\n";
	std::cout << "Both types run the SAME summation; each is checked against the efloat_pi() oracle.\n\n";

	// -------------------------------------------------------------------------
	// double vs efloat at a healthy term count.
	// -------------------------------------------------------------------------
	{
		const int terms = 150;
		efloat512 dsum(double(bbp_pi<double>(terms)));  // double result, lifted for comparison
		efloat512 esum = bbp_pi<efloat512>(terms);
		std::cout << "After " << terms << " terms:\n";
		std::cout << "  double : matches pi to ~" << agreement_digits(dsum, oracle)
		          << " digits   <- double's ceiling (it cannot represent pi more precisely)\n";
		std::cout << "  efloat : matches pi to ~" << agreement_digits(esum, oracle)
		          << " digits   <- 512-bit summation keeps verifying further\n\n";
	}

	// -------------------------------------------------------------------------
	// Convergence table: efloat gains ~1.2 digits/term; double is pinned at ~15.
	// -------------------------------------------------------------------------
	std::cout << "  " << std::left << std::setw(8) << "terms" << std::setw(22) << "double agree (digits)"
	          << "efloat agree (digits)\n";
	std::cout << "  " << std::string(52, '-') << "\n";
	for (int terms = 20; terms <= 150; terms += 20) {
		efloat512 dsum(double(bbp_pi<double>(terms)));
		efloat512 esum = bbp_pi<efloat512>(terms);
		std::cout << "  " << std::left << std::setw(8) << terms << std::setw(22) << agreement_digits(dsum, oracle)
		          << agreement_digits(esum, oracle) << "\n";
	}

	std::cout << "\nSame identity, same code: double stalls at ~15 digits while efloat verifies pi\n";
	std::cout << "to well over 100 digits -- useful as an oracle for checking numerical algorithms.\n";

	return EXIT_SUCCESS;
} catch (const char* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
} catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
