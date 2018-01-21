#if 0   // DEPRECATED
	// this routine will not allocate 0 or infinity due to the test on (0,minpos], and [maxpos,inf)
	// TODO: is that the right functionality? right now the special cases are deal with in the
	// assignment operators for integer/float/double. I don't like that distribution of knowledge.
	void convert_to_posit(value<fbits>& v) {
		convert_to_posit(v.sign(), v.scale(), v.fraction());
	}
	void convert_to_posit(bool _negative, int _scale, std::bitset<fbits> _frac) {
		setToZero();
		if (_trace_conversion) std::cout << "sign " << (_negative ? "-1 " : " 1 ") << "scale " << _scale << " fraction " << _frac << std::endl;

		// construct the posit
		_sign = _negative;	
		unsigned int nr_of_regime_bits = _regime.assign_regime_pattern(_scale >> es);
		bool geometric_round = _exponent.assign_exponent_bits(_scale, nr_of_regime_bits);
		unsigned int nr_of_exp_bits    = _exponent.nrBits();
		unsigned int remaining_bits    = nbits - 1 - nr_of_regime_bits - nr_of_exp_bits > 0 ? nbits - 1 - nr_of_regime_bits - nr_of_exp_bits : 0;
		bool round_up = _fraction.assign_fraction(remaining_bits, _frac);
		if (round_up) 
                    project_up();
		// store raw bit representation
		_raw_bits = _sign ? twos_complement(collect()) : collect();
		_raw_bits.set(nbits - 1, _sign);
		if (_trace_conversion) std::cout << "raw bits: "  << _raw_bits << " posit bits: "  << (_sign ? "1|" : "0|") << _regime << "|" << _exponent << "|" << _fraction << " posit value: " << *this << std::endl;
	}


	/** Generalized conversion function (could replace convert_to_posit). \p _frac is fraction of arbitrary size with hidden bit at \p hpos.
         *  \p hpos == \p FBits means that the hidden bit is in front of \p _frac, i.e. \p _frac is a pure fraction without hidden bit.
         *  
         * 
         */
	template <size_t FBits>
	void convert(bool _negative, int _scale, std::bitset<FBits> _frac, int hpos) {
		if (_trace_conversion) std::cout << "------------------- CONVERT ------------------" << std::endl;
        setToZero();
        if (_trace_conversion) std::cout << "sign " << (_negative ? "-1 " : " 1 ") << "scale " << std::setw(3) << _scale << " fraction " << _frac << std::endl;
                
        // construct the posit
		_sign = _negative;
		int k = calculate_unconstrained_k<nbits, es>(_scale);
		// interpolation rule checks
		if (check_inward_projection_range(_scale)) {    // regime dominated
			if (_trace_conversion) std::cout << "inward projection" << std::endl;
			// we are projecting to minpos/maxpos
			_regime.assign_regime_pattern(k);
			// store raw bit representation
			_raw_bits = _sign ? twos_complement(collect()) : collect();
			_raw_bits.set(nbits - 1, _sign);
			// we are done
			if (_trace_rounding) std::cout << "projection  rounding ";
		} 
		else {
			unsigned int nr_of_regime_bits = _regime.assign_regime_pattern(k);
			bool carry = false;
			switch (_exponent.assign_exponent_bits(_scale, k, nr_of_regime_bits)) {
			case GEOMETRIC_ROUND_UP:
#ifdef INCREMENT_POSIT_CARRY_CHAIN
				carry = _exponent.increment();
				if (carry)_regime.increment();
#endif // INCREMENT_POSIT_CARRY_CHAIN
				break;
			case NO_ADDITIONAL_ROUNDING:
				break;
			case ARITHMETIC_ROUNDING:
				unsigned int nr_of_exp_bits = _exponent.nrBits();
				unsigned int remaining_bits = nbits - 1 - nr_of_regime_bits - nr_of_exp_bits > 0 ? nbits - 1 - nr_of_regime_bits - nr_of_exp_bits : 0;
				bool round_up = _fraction.assign(remaining_bits, _frac, hpos);
				if (round_up) project_up();
			}
			// store raw bit representation
			_raw_bits = _sign ? twos_complement(collect()) : collect();
			_raw_bits.set(nbits - 1, _sign);
		}

        if (_trace_conversion) std::cout << "raw bits: "  << _raw_bits << " posit bits: "  << (_sign ? "1|" : "0|") << _regime << "|" << _exponent << "|" << _fraction << " posit value: " << *this << std::endl;            
    }

#endif	

