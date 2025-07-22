
// __arm__ which is defined for 32bit arm, and 32bit arm only.
// __aarch64__ which is defined for 64bit arm, and 64bit arm only.

#if defined(UNIVERSAL_ARCH_POWER)
	union long_double_decoder {
		long_double_decoder() : ld{ 0.0l } {}
		long_double_decoder(long double _ld) : ld{ _ld } {}
		long double ld;
		struct {
			uint64_t fraction : 64;
			uint64_t upper : 48;
			uint64_t exponent : 15;
			uint64_t sign : 1;
		} parts;
	};

#elif defined(UNIVERSAL_ARCH_X86_64)
	union long_double_decoder {
		long_double_decoder() : ld{ 0.0l } {}
		long_double_decoder(long double _ld) : ld{ _ld } {}
		long double ld;
		struct {
			uint64_t fraction : 63;
			uint64_t bit63 : 1;
			uint64_t exponent : 15;
			uint64_t sign : 1;
		} parts;
	};

#else 
    // long double is mapped to double in ARM64
	union long_double_decoder {
		long_double_decoder() : ld{ 0.0l } {}
		long_double_decoder(long double _ld) : ld{ _ld } {}
		long double ld;
		struct {
			uint64_t fraction : 52;
			uint64_t exponent : 11;
			uint64_t sign : 1;
		} parts;
	};


	inline std::string to_hex(long double number, bool nibbleMarker = false, bool hexPrefix = true) {
		char hexChar[16] = {
			'0', '1', '2', '3', '4', '5', '6', '7',
			'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
		};
		long_double_decoder decoder;
		decoder.ld = number;
		// lower segment
		uint64_t bits = decoder.bits[1];
		//	std::cout << "\nconvert  : " << to_binary(bits, 32) << " : " << bits << '\n';
		std::stringstream s;
		if (hexPrefix) s << "0x";
		int nrNibbles = 16;
		int nibbleIndex = (nrNibbles - 1);
		uint64_t mask = (0xFull << (nibbleIndex * 4));
		//	std::cout << "mask       : " << to_binary(mask, nbits) << '\n';
		for (int n = nrNibbles - 1; n >= 0; --n) {
			uint64_t raw = (bits & mask);
			uint8_t nibble = static_cast<uint8_t>(raw >> (nibbleIndex * 4));
			s << hexChar[nibble];
			if (nibbleMarker && n > 0 && (n % 4) == 0) s << '\'';
			mask >>= 4;
			--nibbleIndex;
		}
		// lower segment
		bits = decoder.bits[0];
		nibbleIndex = (nrNibbles - 1);
		mask = (0xFull << (nibbleIndex * 4));
		//	std::cout << "mask       : " << to_binary(mask, nbits) << '\n';
		for (int n = nrNibbles - 1; n >= 0; --n) {
			uint64_t raw = (bits & mask);
			uint8_t nibble = static_cast<uint8_t>(raw >> (nibbleIndex * 4));
			s << hexChar[nibble];
			if (nibbleMarker && n > 0 && (n % 4) == 0) s << '\'';
			mask >>= 4;
			--nibbleIndex;
		}
		return s.str();
}

#endif

}} // namespace sw::universal

#endif // __clang__
