#pragma once
// posit_manipulators.hpp: definitions of helper functions for posit type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <iomanip>
#include <cmath>  // for frexp/frexpf

// This file contains functions that use the posit type.
// If you have helper functions that the posit type could use, but does not depend on 
// the posit type, you can add them to the file posit_helpers.hpp.

namespace sw {
	namespace unum {

		// DEBUG/REPORTING HELPERS
		template<size_t nbits, size_t es>
		std::string spec_to_string(posit<nbits, es> p) {
			std::stringstream ss;
			ss << " posit<" << std::setw(3) << nbits << "," << es << "> ";
			ss << "useed scale  " << std::setw(4) << useed_scale<nbits, es>() << "     ";
			ss << "minpos scale " << std::setw(10) << minpos_scale<nbits, es>() << "     ";
			ss << "maxpos scale " << std::setw(10) << maxpos_scale<nbits, es>();
			return ss.str();
		}

		template<size_t nbits, size_t es>
		std::string components_to_string(const posit<nbits, es>& p) {
			std::stringstream ss;
			// TODO: hardcoded field width is governed by pretty printing posit tables, which by construction will always be small posits
			ss << std::setw(14) << p.get() << " " << std::setw(14) << p.get_decoded()
				<< " Sign : " << std::setw(2) << p.sign_value()
				<< " Regime : " << std::setw(3) << p.regime_k()
				<< " Exponent : " << std::setw(5) << p.get_exponent().value()
				<< " Fraction : " << std::setw(8) << std::setprecision(21) << 1.0 + p.fraction_value()
				<< " Value : " << std::setw(16) << p
				<< std::setprecision(0);
			return ss.str();
		}

		template<size_t nbits, size_t es>
		std::string component_values_to_string(posit<nbits, es> p) {
			std::stringstream ss;
			// TODO: hardcoded field sizes
			if (p.isZero()) {
				ss << " zero    " << std::setw(103) << "b" << p.get();
				return ss.str();
			}
			else if (p.isInfinite()) {
				ss << " infinite" << std::setw(103) << "b" << p.get();
				return ss.str();
			}

			ss << std::setw(14) << to_binary(p.get())
				<< " Sign : " << std::setw(2) << p.sign()
				<< " Regime : " << p.regime_int()
				<< " Exponent : " << p.exponent_int()
				<< std::hex
				<< " Fraction : " << p.fraction_int()
				<< " Value : " << p.to_int64()
				<< std::dec;
			return ss.str();
		}

		template<size_t nbits, size_t es>
		std::string pretty_print(const posit<nbits, es>& p, int printPrecision = std::numeric_limits<double>::max_digits10) {
			static constexpr size_t fbits = nbits - 3 - es;  // TODO: is there a better solution to gain access to the posit's fbits value?
			std::stringstream ss;
			ss << ( p.get_sign() ? "s1 r" : "s0 r" );
			std::bitset<nbits-1> r = p.get_regime().get();
			int regimeBits = (int)p.get_regime().nrBits();
			int nrOfRegimeBitsProcessed = 0;
			for (int i = nbits - 2; i >= 0; --i) {
				if (regimeBits > nrOfRegimeBitsProcessed++) {
					ss << (r[i] ? "1" : "0");
				}
			}
			ss << " e";
			std::bitset<es> e = p.get_exponent().get();
			int exponentBits = (int)p.get_exponent().nrBits();
			int nrOfExponentBitsProcessed = 0;
			for (int i = int(es) - 1; i >= 0; --i) {
				if (exponentBits > nrOfExponentBitsProcessed++) {
					ss << (e[i] ? "1" : "0");
				}
			}
			ss << " f";
			std::bitset<fbits> f = p.get_fraction().get();
			int fractionBits = (int)p.get_fraction().nrBits();
			int nrOfFractionBitsProcessed = 0;
			for (int i = int(p.fbits) - 1; i >= 0; --i) {
				if (fractionBits > nrOfFractionBitsProcessed++) {
					ss << (f[i] ? "1" : "0");
				}
			}
			ss << " q";
			ss << p.get_quadrant() << " v"
				<< std::setprecision(printPrecision) << p
				<< std::setprecision(0);
			return ss.str();
		}

		template<size_t nbits, size_t es>
		std::string info_print(const posit<nbits, es>& p, int printPrecision) {
			std::stringstream ss;
			ss << "raw: " << p.get() << " decoded: " << p.get_decoded() << " "
				<< p.get_quadrant() << " "
				<< (p.get_sign() ? "negative r" : "positive r")
				<< p.get_regime() << " e"
				<< p.get_exponent() << " f"
				<< p.get_fraction() << " : value "
				<< std::setprecision(printPrecision) << p
				<< std::setprecision(0);
			return ss.str();
		}

		enum ColorCode {
			FG_DEFAULT = 39,
			FG_BLACK = 30,
			FG_RED = 31,
			FG_GREEN = 32,
			FG_YELLOW = 33,
			FG_BLUE = 34,
			FG_MAGENTA = 35,
			FG_CYAN = 36,
			FG_LIGHT_GRAY = 37,
			FG_DARK_GRAY = 90,
			FG_LIGHT_RED = 91,
			FG_LIGHT_GREEN = 92,
			FG_LIGHT_YELLOW = 93,
			FG_LIGHT_BLUE = 94,
			FG_LIGHT_MAGENTA = 95,
			FG_LIGHT_CYAN = 96,
			FG_WHITE = 97,

			BG_DEFAULT = 49,
			BG_BLACK = 40,
			BG_RED = 41,
			BG_GREEN = 42,
			BG_YELLOW = 43,
			BG_BLUE = 44,
			BG_MAGENTA = 45,
			BG_CYAN = 46,
			BG_LIGHT_GRAY = 47,
			BG_DARK_GRAY = 100,
			BG_LIGHT_RED = 101,
			BG_LIGHT_GREEN = 102,
			BG_LIGHT_YELLOW = 103,
			BG_LIGHT_BLUE = 104,
			BG_LIGHT_MAGENTA = 105,
			BG_LIGHT_CYAN = 106,
			BG_WHITE = 107

		};
		class Color {
			ColorCode code;
		public:
			Color(ColorCode pCode) : code(pCode) {}
			friend std::ostream&
				operator<<(std::ostream& os, const Color& mod) {
				return os << "\033[" << mod.code << "m";
			}
		};

		template<size_t nbits, size_t es>
		std::string color_print(const posit<nbits, es>& p) {
			std::stringstream ss;
			Color red(ColorCode::FG_RED);
			Color yellow(ColorCode::FG_YELLOW);
			Color blue(ColorCode::FG_BLUE);
			Color magenta(ColorCode::FG_MAGENTA);
			Color cyan(ColorCode::FG_CYAN);
			Color white(ColorCode::FG_WHITE);
			Color def(ColorCode::FG_DEFAULT);
			ss << red << (p.isNegative() ? "1" : "0");

			std::bitset<nbits - 1> r = p.get_regime().get();
			int regimeBits = (int)p.get_regime().nrBits();
			int nrOfRegimeBitsProcessed = 0;
			for (int i = nbits - 2; i >= 0; --i) {
				if (regimeBits > nrOfRegimeBitsProcessed++) {
					ss << yellow << (r[i] ? "1" : "0");
				}
			}

			std::bitset<es> e = p.get_exponent().get();
			int exponentBits = (int)p.get_exponent().nrBits();
			int nrOfExponentBitsProcessed = 0;
			for (int i = int(es) - 1; i >= 0; --i) {
				if (exponentBits > nrOfExponentBitsProcessed++) {
					ss << cyan << (e[i] ? "1" : "0");
				}
			}

			bitblock<posit<nbits, es>::fbits> f = p.get_fraction().get();
			int fractionBits = (int)p.get_fraction().nrBits();
			int nrOfFractionBitsProcessed = 0;
			for (int i = int(p.fbits) - 1; i >= 0; --i) {
				if (fractionBits > nrOfFractionBitsProcessed++) {
					ss << magenta << (f[i] ? "1" : "0");
				}
			}

			ss << def;
			return ss.str();
		}
		
		// generate a full binary representation table for a given posit configuration
		template<size_t nbits, size_t es>
		void GeneratePositTable(std::ostream& ostr) 
		{
			ostr << "Generate Posit Lookup table for a POSIT<" << nbits << "," << es << ">" << std::endl;

			const size_t size = (1 << nbits);
			posit<nbits, es>	myPosit;

			const size_t index_column = 5;
			const size_t bin_column = 16;
			const size_t k_column = 8;
			const size_t sign_column = 8;
			const size_t scale_column = 8;
			const size_t regime_value_column = 30;
			const size_t regime_column = 16;
			const size_t exponent_column = 16;
			const size_t fraction_column = 16;
			const size_t value_column = 30;

			ostr << std::setw(index_column) << " # "
				<< std::setw(bin_column) << " Binary"
				<< std::setw(bin_column) << " Decoded"
				<< std::setw(k_column) << " k"
				<< std::setw(sign_column) << "sign"
				<< std::setw(scale_column) << "scale"
		//		<< std::setw(regime_value_column) << " regime"
				<< std::setw(regime_column) << " regime"
				<< std::setw(exponent_column) << " exponent"
				<< std::setw(fraction_column) << " fraction"
				<< std::setw(value_column) << " value" << std::endl;
			for (int i = 0; i < size; i++) {
				myPosit.set_raw_bits(i);
				regime<nbits,es>   r = myPosit.get_regime();
				exponent<nbits,es> e = myPosit.get_exponent();
				fraction<myPosit.fbits>    f = myPosit.get_fraction();
				ostr << std::setw(4) << i << ": "
					<< std::setw(bin_column) << myPosit.get()
					<< std::setw(bin_column) << myPosit.get_decoded()
					<< std::setw(k_column) << myPosit.regime_k()
					<< std::setw(sign_column) << myPosit.sign_value()
					<< std::setw(scale_column) << myPosit.scale()
		//			<< std::setw(regime_value_column) << std::setprecision(22) << r.value() << std::setprecision(0)
					<< std::setw(regime_column) << std::right << r
					<< std::setw(exponent_column) << std::right << e 
					<< std::setw(fraction_column) << std::right << f
					<< std::setw(value_column) << std::setprecision(22) << myPosit << std::setprecision(0)
					<< std::endl;
			}
		}

	}  // namespace unum

}  // namespace sw

