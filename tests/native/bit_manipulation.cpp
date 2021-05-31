//  bit_manipulation.cpp: experiments with the C++20 <bit> library
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <bit>

union float_decoder {
	float_decoder() : f{ 0.0f } {}
	float_decoder(float _f) : f{ _f } {}
	float f;
	struct {
		uint32_t fraction : 23;
		uint32_t exponent : 8;
		uint32_t sign : 1;
	} parts;
};

template<typename Integer,
	typename = typename std::enable_if< std::is_integral<Integer>::value, Integer >::type
>
inline std::string to_binary(const Integer& number, int nbits = 0, bool bNibbleMarker = true) {
	std::stringstream s;
	if (nbits == 0) nbits = 8 * sizeof(number);
	s << 'b';
	uint64_t mask = (uint64_t(1) << (nbits - 1));
	for (int i = int(nbits) - 1; i >= 0; --i) {
		s << ((number & mask) ? '1' : '0');
		if (bNibbleMarker && i > 0 && i % 4 == 0) s << '\'';
		mask >>= 1;
	}
	return s.str();
}

int main()
{
	using namespace std;

	// create a float with the following layout
	// b1.00001111.00011001011010001001001"
	float_decoder decoder;
	decoder.parts.fraction = 0b00011001011010001001001;
	decoder.parts.exponent = 0b0000'0001;
	decoder.parts.sign = 0b1;

	cout << decoder.f << endl;
#ifdef BIT_CAST_SUPPORT
	uint32_t bc = std::bit_cast<uint32_t, float>(decoder.f);
	cout << to_binary(bc, 32) << endl;
#endif

	return EXIT_SUCCESS;
}
