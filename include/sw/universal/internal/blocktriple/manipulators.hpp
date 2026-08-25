#pragma once
// manipulators.hpp: text produced from a blocktriple
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The <iomanip> half of blocktriple's text layer (#1334): type_tag, to_binary,
// to_triple, and the out-of-line definition of blocktriple::to_string, declared
// in blocktriple.hpp but defined here so the core needs no <sstream>.
//
// to_binary and to_triple read _sign, _scale and _significand directly, so they
// remain friends of the class; blocktriple.hpp declares that friendship and this
// header supplies the definitions.
//
// to_string(BlockTripleOperator) lives here rather than in iostream.hpp because
// type_tag needs the operator's NAME, not a stream insertion. Keeping the string
// form in the <iomanip> layer is what stops this header depending on that one.
#include <cstdint>
#include <ios>
#include <iomanip>
#include <sstream>
#include <string>
#include <typeinfo>
#include <universal/native/integers.hpp>          // to_binary(uint64_t)
#include <universal/number/support/decimal.hpp>   // support::decimal, for power_of_five
#include <universal/internal/blocktriple/blocktriple.hpp>

namespace sw { namespace universal {

// the operator tag as a string
inline std::string to_string(BlockTripleOperator op) {
	switch (op) {
	case BlockTripleOperator::REP:  return "REP";
	case BlockTripleOperator::ADD:  return "ADD";
	case BlockTripleOperator::MUL:  return "MUL";
	case BlockTripleOperator::DIV:  return "DIV";
	case BlockTripleOperator::SQRT: return "SQRT";
	default: return "NOP";
	}
}

// Generate a type tag for this type: blocktriple<fbits, operator, unsigned int>
template<unsigned fbits, BlockTripleOperator op, typename bt>
std::string type_tag(const blocktriple<fbits, op, bt>& = {}) {
	std::stringstream s;
	s << "blocktriple<"
	  << std::setw(3) << fbits << ", "
      << to_string(op) << ", "
	  << type_tag(bt{}) << '>';
	return s.str();
}

// convert blocktriple to a formatted decimal string
// Follows the same interface as ereal::to_string() for consistency.
// Uses support::decimal for exact binary-to-decimal conversion.
// power_of_five: a decimal helper used only by to_string, which is why it lives
// in the text layer. It was a private static member of blocktriple, and keeping
// it there meant the core carried support/decimal.hpp (#1334).
// Compute 5^n using repeated squaring with support::decimal
inline support::decimal power_of_five(size_t n) {
	support::decimal result;
	result.setdigit(1);
	support::decimal base;
	base.setdigit(5);
	while (n > 0) {
		if (n & 1) support::mul(result, base);
		if (n > 1) support::mul(base, base);
		n >>= 1;
	}
	return result;
}

template<unsigned fbits, BlockTripleOperator op, typename bt>
std::string blocktriple<fbits, op, bt>::to_string(std::streamsize precision, std::streamsize width,
		bool fixed, bool scientific, bool internal,
		bool left, bool showpos, bool uppercase,
		char fill) const
{
	std::string s;
	if (fixed && scientific) fixed = false; // scientific takes precedence
	if (!fixed && !scientific) scientific = true; // defaultfloat -> scientific

	if (_nan) {
		s = _sign ? (uppercase ? "SNAN" : "snan") : (uppercase ? "QNAN" : "qnan");
		goto apply_width;
	}

	{
		// sign prefix for non-NaN values
		if (_sign) { s += '-'; } else { if (showpos) s += '+'; }

		if (_inf) {
			s += uppercase ? "INF" : "inf";
		}
		else if (_zero) {
			s += '0';
			if (precision > 0) {
				s += '.';
				s.append(static_cast<size_t>(precision), '0');
			}
			if (!fixed) {
				s += uppercase ? "E+00" : "e+00";
			}
		}
		else {
			// Normal value: exact binary-to-decimal via support::decimal
			// Value = significand * 2^(scale - radix)
			// where significand is stored as an integer with radix point at position 'radix'

			// Step 1: accumulate significand bits into a decimal integer
			support::decimal sig;
			sig.setzero();
			support::decimal bitWeight;
			bitWeight.setdigit(1);
			for (unsigned i = 0; i < bfbits; ++i) {
				if (_significand.at(i)) {
					support::add(sig, bitWeight);
				}
				support::add(bitWeight, bitWeight); // bitWeight *= 2
			}

			// Step 2: compute effective exponent
			long long effExp = static_cast<long long>(_scale) - static_cast<long long>(radix);

			// Step 3: scale the significand to get exact decimal digits
			// sig * 2^effExp = exact value
			// If effExp >= 0: multiply sig by 2^effExp
			// If effExp < 0:  sig * 2^(-|e|) = sig * 5^|e| / 10^|e|
			//   so multiply sig by 5^|e| and remember |e| implied fractional digits

			long long impliedFracDigits = 0;

			if (effExp > 0) {
				if (effExp > 100000) {
					// Extreme case: compute approximate scientific notation
					// from significand and binary exponent directly.
					// decimal exponent = effExp * log10(2) + log10(sig_value)
					double sigVal = static_cast<double>(_significand);
					double log10Val = effExp * 0.30102999566398119521 + std::log10(sigVal);
					long long decExp = static_cast<long long>(std::floor(log10Val));
					double mantissa = std::pow(10.0, log10Val - decExp);
					if (mantissa >= 10.0) { mantissa /= 10.0; ++decExp; }
					if (mantissa < 1.0) { mantissa *= 10.0; --decExp; }
					std::ostringstream oss;
					oss.precision(precision);
					oss << std::fixed << mantissa;
					if (!fixed) {
						s += oss.str();
						s += uppercase ? 'E' : 'e';
						append_exponent(s, decExp);
					} else {
						// fixed with extreme exponent: just show the approximate value
						s += oss.str();
						s += uppercase ? 'E' : 'e';
						append_exponent(s, decExp);
					}
					goto apply_width;
				}
				support::decimal pow2;
				pow2.powerOf2(static_cast<size_t>(effExp));
				support::mul(sig, pow2);
			}
			else if (effExp < 0) {
				long long absExp = -effExp;
				if (absExp > 100000) {
					// Extreme case: compute approximate scientific notation
					double sigVal = static_cast<double>(_significand);
					double log10Val = effExp * 0.30102999566398119521 + std::log10(sigVal);
					long long decExp = static_cast<long long>(std::floor(log10Val));
					double mantissa = std::pow(10.0, log10Val - decExp);
					if (mantissa >= 10.0) { mantissa /= 10.0; ++decExp; }
					if (mantissa < 1.0) { mantissa *= 10.0; --decExp; }
					std::ostringstream oss;
					oss.precision(precision);
					oss << std::fixed << mantissa;
					s += oss.str();
					s += uppercase ? 'E' : 'e';
					append_exponent(s, decExp);
					goto apply_width;
				}
				support::decimal pow5 = power_of_five(static_cast<size_t>(absExp));
				support::mul(sig, pow5);
				impliedFracDigits = absExp;
			}

			sig.unpad();

			// Step 4: determine digit layout
			// sig now represents the exact value * 10^impliedFracDigits
			// Total decimal digits in sig
			long long totalDigits = static_cast<long long>(sig.size());
			// Position of decimal point: sciExponent is the power-of-10 of the MSD
			long long sciExponent = (totalDigits - 1) - impliedFracDigits;

			// Step 5: extract digits MSD-first into a vector
			std::vector<int> digits(static_cast<size_t>(totalDigits));
			for (long long i = 0; i < totalDigits; ++i) {
				digits[static_cast<size_t>(i)] = sig[static_cast<size_t>(totalDigits - 1 - i)];
			}

			// Step 6: determine how many digits we need and round
			long long neededDigits;
			if (fixed) {
				// fixed: we need (sciExponent + 1) integer digits + precision fraction digits
				neededDigits = sciExponent + 1 + precision;
				if (neededDigits < 1) neededDigits = 1;
			}
			else {
				// scientific: 1 integer digit + precision fraction digits
				neededDigits = 1 + precision;
			}

			// Round at the boundary
			if (neededDigits < totalDigits) {
				int roundDigit = digits[static_cast<size_t>(neededDigits)];
				if (roundDigit >= 5) {
					// propagate carry
					long long k = neededDigits - 1;
					while (k >= 0) {
						digits[static_cast<size_t>(k)]++;
						if (digits[static_cast<size_t>(k)] < 10) break;
						digits[static_cast<size_t>(k)] = 0;
						--k;
					}
					if (k < 0) {
						// carry out of MSD: insert a 1 at front
						digits.insert(digits.begin(), 1);
						totalDigits++;
						sciExponent++;
						// neededDigits stays same, but we may have one more digit
					}
				}
				digits.resize(static_cast<size_t>(neededDigits));
				totalDigits = neededDigits;
			}
			else if (neededDigits > totalDigits) {
				// pad with trailing zeros
				digits.resize(static_cast<size_t>(neededDigits), 0);
				totalDigits = neededDigits;
			}

			// Step 7: format the digits
			if (fixed) {
				long long intDigits = sciExponent + 1;
				if (intDigits <= 0) {
					s += "0.";
					for (long long z = 0; z < -intDigits && z < precision; ++z) s += '0';
					long long remaining = precision - (-intDigits);
					if (remaining < 0) remaining = 0;
					for (long long i = 0; i < remaining && i < totalDigits; ++i) {
						s += static_cast<char>('0' + digits[static_cast<size_t>(i)]);
					}
					// pad if needed
					long long written = (-intDigits) + std::min(remaining, totalDigits);
					for (long long i = written; i < precision; ++i) s += '0';
				}
				else {
					for (long long i = 0; i < intDigits && i < totalDigits; ++i) {
						s += static_cast<char>('0' + digits[static_cast<size_t>(i)]);
					}
					// pad integer part if digits ran out
					for (long long i = totalDigits; i < intDigits; ++i) s += '0';
					if (precision > 0) {
						s += '.';
						for (long long i = intDigits; i < intDigits + precision; ++i) {
							if (i < totalDigits)
								s += static_cast<char>('0' + digits[static_cast<size_t>(i)]);
							else
								s += '0';
						}
					}
				}
			}
			else {
				// scientific: d.dddddde+/-EEE
				if (totalDigits > 0)
					s += static_cast<char>('0' + digits[0]);
				else
					s += '0';
				if (precision > 0) {
					s += '.';
					for (long long i = 1; i <= precision; ++i) {
						if (i < totalDigits)
							s += static_cast<char>('0' + digits[static_cast<size_t>(i)]);
						else
							s += '0';
					}
				}
				s += uppercase ? 'E' : 'e';
				append_exponent(s, sciExponent);
			}
		}
	}

apply_width:
	// process width/fill/alignment
	if (width > 0 && s.length() < static_cast<size_t>(width)) {
		size_t nrCharsToFill = static_cast<size_t>(width) - s.length();
		if (internal) {
			const bool hasSign = !s.empty() && (s[0] == '-' || s[0] == '+');
			s.insert(hasSign ? static_cast<std::string::size_type>(1)
			                 : static_cast<std::string::size_type>(0),
			         nrCharsToFill, fill);
		}
		else if (left) {
			s.append(nrCharsToFill, fill);
		}
		else {
			s.insert(static_cast<std::string::size_type>(0), nrCharsToFill, fill);
		}
	}

	return s;
}

template<unsigned fbits, BlockTripleOperator op, typename bt>
std::string to_binary(const sw::universal::blocktriple<fbits, op, bt>& a, bool nibbleMarker) {
	return to_triple(a, nibbleMarker);
}

template<unsigned fbits, BlockTripleOperator op, typename bt>
std::string to_triple(const blocktriple<fbits, op, bt>& a, bool nibbleMarker) {
	std::stringstream s;
	s << (a._sign ? "(-, " : "(+, ");
	s << std::setw(3) << a._scale << ", ";
	s << to_binary(a._significand, nibbleMarker) << ')';
	return s.str();
}

}} // namespace sw::universal
