template<size_t nbits, size_t es>
class posit {
	posit<nbits, es>& assign(int64_t rhs) {
		reset();
		if (rhs == 0) {
			return *this;
		}

		int msb;
		bool value_is_negative = false;
		if (rhs < 0) {
			rhs = -rhs;
			value_is_negative = true;
		}
		msb = findMostSignificantBit(rhs)-1;
		if (msb > maxpos_scale()) {
			// TODO: Can we make this a compile time evaluated function for literals?
			std::cerr << "msb = " << msb << " and maxpos_scale() = " << maxpos_scale() << std::endl;
			std::cerr << "Can't represent " << rhs << " with posit<" << nbits << "," << es << ">: maxpos = " << (1 << maxpos_scale()) << std::endl;
		}
		_Bits[nbits - 1] = false;
		unsigned int nr_of_regime_bits = assign_regime_pattern(msb >> es);
		std::cout << "Regime   " << to_binary<nbits>(_Bits) << std::endl;

		unsigned int nr_of_exp_bits = assign_exponent_bits(msb, nr_of_regime_bits);
		std::cout << "Exponent " << to_binary<nbits>(_Bits) << std::endl;

		unsigned int remainder_bits = (nbits - 1 - nr_of_regime_bits - nr_of_exp_bits > 0 ? nbits - 1 - nr_of_regime_bits - nr_of_exp_bits : 0);
		switch (bRoundingMode) {
			case POSIT_ROUND_DOWN:
			{
				if (remainder_bits > 0) {
					uint64_t mask = (1 << (msb-1));  // first bit is transformed into a hidden bit
					for (int i = 0; i < remainder_bits; i++) {
						_Bits[nbits - 2 - nr_of_regime_bits - nr_of_exp_bits - i] = rhs & mask;
						mask >>= 1;
					}
					std::cout << "Fraction " << to_binary<nbits>(_Bits) << std::endl;
				}
			}
				break;
			case POSIT_ROUND_TO_NEAREST:
				std::cerr << "ROUND_TO_NEAREST not implemented yet" << std::endl;
				break;
			default:
				std::cerr << "Undefined rounding mode" << std::endl;
				break;
		}

		if (value_is_negative) {
			_Bits = twos_complement(_Bits);
			_Bits.set(nbits - 1);
		}
		decode();
		return *this;
	}
};
