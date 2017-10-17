// -1 -> round-down, 0 -> no rounding, +1 -> round-up
// _fraction contains the fraction without the hidden bit
int rounding_decision(const std::bitset<nbits>& _fraction, unsigned int nr_of_fraction_bits) {
	if (_trace_rounding) std::cout << "_fraction bits to process: " << nr_of_fraction_bits << " " << _fraction << std::endl;
	// check if there are any bits set past the cut-off
	int rounding_direction = 0;
	if (nr_of_fraction_bits == 0) {
		if (_fraction.test(nbits - 1)) {
			rounding_direction = 1;
			if (_trace_rounding) std::cout << "Rounding up" << std::endl;
		}
		else {
			rounding_direction = -1;
			if (_trace_rounding) std::cout << "Rounding down" << std::endl;
		}
		return rounding_direction;
	}
	// first bit after the cut-off is at nbits - 1 - nr_of_fraction_bits
	if (nbits >= 4 + nr_of_fraction_bits) {
		if (_trace_rounding) std::cout << "_fraction bits to process: " << nr_of_fraction_bits << " " << _fraction << std::endl;
		rounding_direction = -1;
		for (int i = nbits - 1 - nr_of_fraction_bits; i >= 0; --i) {
			if (_fraction.test(i)) {
				rounding_direction = 1;
				if (_trace_rounding) std::cout << "Fraction bit set: round up" << std::endl;
				break;
			}
		}
		if (rounding_direction == -1) {
			if (_trace_rounding) std::cout << "Fraction bits not set: round down" << std::endl;
		}
	}
	else {
		if (_fraction.test(nbits - 1)) {
			rounding_direction = 1;
			if (_trace_rounding) std::cout << "fraction indicates nearest is up" << std::endl;
		}
		else {
			rounding_direction = -1;
			if (_trace_rounding) std::cout << "fraction indicates nearest is down" << std::endl;

		}
	}
	return rounding_direction;
}
unsigned int estimate_nr_fraction_bits(int k) {
	unsigned int nr_of_regime_bits;
	if (k < 0) {
		k = -k - 1;
		nr_of_regime_bits = (k < nbits - 2 ? k + 2 : nbits - 1);
	}
	else {
		nr_of_regime_bits = (k < nbits - 2 ? k + 2 : nbits - 1);
	}
	unsigned int nr_of_exp_bits = (nbits - 1 - nr_of_regime_bits > es ? es : nbits - 1 - nr_of_regime_bits);
	return (nbits - 1 - nr_of_regime_bits - nr_of_exp_bits > 0 ? nbits - 1 - nr_of_regime_bits - nr_of_exp_bits : 0);
}


int round(bool _negative, int _scale, std::bitset<nbits>& _fraction) {
	if (nbits > 3) {
		switch (rounding_decision(_fraction, estimate_nr_fraction_bits(_scale >> es))) {
		case -1:
			if (_trace_rounding) std::cout << "Rounding down to nearest" << std::endl;
			break;
		case 0:
			if (_trace_rounding) std::cout << "No Rounding required" << std::endl;
			break;
		case 1:
			if (_trace_rounding) std::cout << "Rounding up to nearest" << std::endl;
			_scale += 1;
			break;
		}
	}

	return _scale;
}
// this routine will not allocate 0 or infinity due to the test on (0,minpos], and [maxpos,inf)
// TODO: is that the right functionality? right now the special cases are deal with in the
// assignment operators for integer/float/double. I don't like that distribution of knowledge.
void convert_to_posit(bool _negative, int _scale, std::bitset<nbits>& _frac) {
	reset();
	if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;
	_sign = _negative;
	int posit_size = static_cast<int>(nbits);
	int es_size = static_cast<int>(es);
	// deal with minpos/maxpos special cases
	int k = (_scale >> es);
	if (_trace_conversion) std::cout << (_negative ? "sign -1 " : "sign 1 ") << " scale = " << _scale << " fraction " << _frac << " es = " << es << " k = " << k << std::endl;
	if (k < 0) {
		// minpos is at k = -(nbits-2) and minpos*useed is at k = -(nbits-3)
		if (k <= -(posit_size - 2)) { // <= minpos     NOTE: 0 is dealt with in special case
			if (_trace_conversion) std::cout << "value between 0 and minpos: round up" << std::endl;
			_regime.assign_regime_pattern(_negative, 2 - int(posit_size));  // assign minpos
			_raw_bits = (_sign ? twos_complement(collect()) : collect());
			_raw_bits.set(posit_size - 1, _sign);
			if (_trace_conversion) std::cout << (_sign ? "sign -1 " : "sign  1 ") << "regime " << _regime << " exp " << _exponent << " fraction " << _fraction << " posit    " << *this << std::endl;
			return;
		}
		else if (-(posit_size - 3) <= k && k < -(posit_size - 2)) {							// regime rounding
			if (_trace_conversion) std::cout << "minpos < value <= minpos*useed: round depending on _regime: incoming fraction " << _frac << std::endl;
			if (_frac[nbits - 1]) k--;
		}
		else if (es_size > 0 && -(posit_size - 3 - es_size) <= k && k < -(posit_size - 3)) {	// exponent rounding
			if (_trace_conversion) std::cout << "minpos*useed < value <= (minpos >> es): round depending on _exponent" << std::endl;
			if (_frac[nbits - 1]) _scale++;
		}
		else {																				// fraction rounding
			if (_trace_conversion) std::cout << "value > (minpos >> es): round depending on _fraction" << std::endl;
			_scale = round(_negative, _scale, _frac);
			k = (_scale >> es);
		}
	}
	else {
		// maxpos is at k = nbits-2 and maxpos/useed is at k = nbits-3
		if (k >= (posit_size - 2)) { // maxpos            NOTE: INFINITY is dealt with in special case
			if (_trace_conversion) std::cout << "value between maxpos and INFINITY: round down" << std::endl;
			_regime.assign_regime_pattern(_negative, posit_size - 2);	// assign maxpos
			_raw_bits = (_sign ? twos_complement(collect()) : collect());
			_raw_bits.set(posit_size - 1, _sign);
			if (_trace_conversion) std::cout << (_sign ? "sign -1 " : "sign  1 ") << "regime " << _regime << " exp " << _exponent << " fraction " << _fraction << " posit    " << *this << std::endl;
			return;
		}
		else if ((posit_size - 3) <= k && k < (posit_size - 2)) {						// regime rounding
			if (_trace_conversion) std::cout << "maxpos < value <= maxpos/useed: round depending on _regime: incoming fraction " << _frac << std::endl;
			if (_frac[nbits - 1]) k++;
		}
		else if (es_size > 0 && (posit_size - 3 - es_size) <= k && k < (posit_size - 3)) {	// exponent rounding
			if (_trace_conversion) std::cout << "(maxpos >> es) < value <= maxpos/useed: round depending on _exponent" << std::endl;
			if (_frac[nbits - 1]) _scale++;
		}
		else {																			// fraction rounding
			if (_trace_conversion) std::cout << "value < (maxpos >> es): round depending on _fraction" << std::endl;
			_scale = round(_negative, _scale, _frac);
			k = (_scale >> es);
		}
	}
	if (_trace_conversion) std::cout << (_negative ? "sign -1 " : "sign 1 ") << " scale = " << _scale << " fraction " << _frac << " es = " << es << " k = " << k << std::endl;

	// construct the posit
	unsigned int nr_of_regime_bits = _regime.assign_regime_pattern(_sign, k);
	unsigned int nr_of_exp_bits = _exponent.assign_exponent_bits(_scale, nr_of_regime_bits);
	unsigned int remaining_bits = (nbits - 1 - nr_of_regime_bits - nr_of_exp_bits > 0 ? nbits - 1 - nr_of_regime_bits - nr_of_exp_bits : 0);
	_fraction.assign_fraction(remaining_bits, _frac);
	if (_trace_conversion) std::cout << (_sign ? "sign -1 " : "sign  1 ") << "regime " << _regime << " exp " << _exponent << " fraction " << _fraction;
	// store raw bit representation
	_raw_bits = (_sign ? twos_complement(collect()) : collect());
	_raw_bits.set(posit_size - 1, _sign);
	if (_trace_conversion) std::cout << " posit    " << *this << std::endl;
}