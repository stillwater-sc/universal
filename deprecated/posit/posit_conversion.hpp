
posit<nbits, es>& operator=(uint64_t rhs) {
	reset();
	if (rhs != 0) {
		unsigned int _scale = findMostSignificantBit(rhs) - 1;
		uint64_t _fraction_without_hidden_bit = (rhs << (64 - _scale));
		std::bitset<nbits - 2> _fraction = copy_integer_fraction<nbits - 2>(_fraction_without_hidden_bit);
		convert_to_posit(false, _scale, _fraction);
	}
	decode();
	return *this;
}

posit<nbits, es>& operator=(int64_t rhs) {
	reset();
	if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

	bool _sign = (0x8000000000000000 & rhs);  // 1 is negative, 0 is positive
	if (_sign) {
		// process negative number: process 2's complement of the input
		unsigned int _scale = findMostSignificantBit(-rhs) - 1;
		uint64_t _fraction_without_hidden_bit = (-rhs << (64 - _scale));
		std::bitset<nbits - 2> _fraction = copy_integer_fraction<nbits - 2>(_fraction_without_hidden_bit);
		convert_to_posit(_sign, _scale, _fraction);
		take_2s_complement();
	}
	else {
		// process positive number
		if (rhs != 0) {
			unsigned int _scale = findMostSignificantBit(rhs) - 1;
			uint64_t _fraction_without_hidden_bit = (rhs << (64 - _scale));
			std::bitset<nbits - 2> _fraction = copy_integer_fraction<nbits - 2>(_fraction_without_hidden_bit);
			convert_to_posit(_sign, _scale, _fraction);
		}
	}
	return *this;
}

posit<nbits, es>& operator=(const float rhs) {
	reset();
	if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

	switch (std::fpclassify(rhs)) {
	case FP_ZERO:
		_sign = false;
		_regime.setZero();
		break;
	case FP_INFINITE:
		_sign = true;
		_regime.setZero();
		_raw_bits.set(nbits - 1, true);
		break;
	case FP_NAN:
		std::cerr << "float is NAN: returning 0" << std::endl;
		break;
	case FP_SUBNORMAL:
		std::cerr << "TODO: subnormal number: returning 0" << std::endl;
		break;
	case FP_NORMAL:
	{
		if (rhs == 0.0) break;  // 0 is a special case
		bool _negative = extract_sign(rhs);
		int _scale = extract_exponent(rhs) - 1;
		uint32_t _23b_fraction_without_hidden_bit = extract_fraction(rhs);
		std::bitset<nbits - 2> _fraction = extract_float_fraction<nbits - 2>(_23b_fraction_without_hidden_bit);
		if (_trace_conversion) std::cout << "float " << rhs << " sign " << _negative << " scale " << _scale << " 23b fraction 0x" << std::hex << _23b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
		convert_to_posit(_negative, _scale, _fraction);
	}
	break;
	}
	return *this;
}

posit<nbits, es>& operator=(const double rhs) {
	reset();
	if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

	switch (std::fpclassify(rhs)) {
	case FP_ZERO:
		_sign = false;
		_regime.setZero();
		break;
	case FP_INFINITE:
		_sign = true;
		_regime.setZero();
		_raw_bits.set(nbits - 1, true);
		break;
	case FP_NAN:
		std::cerr << "float is NAN" << std::endl;
		break;
	case FP_SUBNORMAL:
		std::cerr << "TODO: subnormal number" << std::endl;
		break;
	case FP_NORMAL:
	{
		if (rhs == 0.0) break;  // 0 is a special case
		bool _negative = extract_sign(rhs);
		int _scale = extract_exponent(rhs) - 1;
		uint64_t _52b_fraction_without_hidden_bit = extract_fraction(rhs);
		std::bitset<nbits - 2> _fraction = extract_double_fraction<nbits - 2>(_52b_fraction_without_hidden_bit);
		if (_trace_conversion) std::cout << "double " << rhs << "sign " << _negative << " scale " << _scale << " 52b fraction 0x" << std::hex << _52b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
		convert_to_posit(_negative, _scale, _fraction);
	}
	break;
	}
	return *this;
}