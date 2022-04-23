#ifdef DEPRECATED
	integer& operator*=(const integer& rhs) {
		integer<nbits, BlockType, NumberType> base(*this);
		integer<nbits, BlockType, NumberType> multiplicant(rhs);
		clear();
		for (unsigned i = 0; i < nbits; ++i) {
			if (base.at(i)) {
				operator+=(multiplicant);
			}
			multiplicant <<= 1;
		}
		// since we used operator+=, which enforces the nulling of leading bits
		// we don't need to null here
		return *this;
	}
#endif