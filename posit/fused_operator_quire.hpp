#pragma once

#include <cmath>
#include <iostream>


const uint8_t POSIT_ROUND_DOWN = 0;
const uint8_t POSIT_ROUND_TO_NEAREST = 1;

/*
 class posit represents arbitrary configuration posits and their arithmetic
 */
template<size_t nbits> class quire : std::bitset<nbits> {
public:
	quire<nbits>() {}
	quire<nbits>(const quire& q) {
		*this = q;
	}
        // template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits>
	friend std::ostream& operator<< (std::ostream& ostr, const quire<nnbits>& p);
	template<size_t nnbits>
	friend std::istream& operator>> (std::istream& istr, quire<nnbits>& p);

	template<size_t nnbits>
	friend bool operator==(const quire<nnbits>& lhs, const quire<nnbits>& rhs);
	template<size_t nnbits>
	friend bool operator!=(const quire<nnbits>& lhs, const quire<nnbits>& rhs);
	template<size_t nnbitss>
	friend bool operator< (const quire<nnbits>>& lhs, const quire<nnbits>& rhs);
	template<size_t nnbits>
	friend bool operator> (const quire<nnbits>>& lhs, const quire<nnbits>& rhs);
	template<size_t nnbits>
	friend bool operator<=(const quire<nnbits>>& lhs, const quire<nnbits>& rhs);
	template<size_t nnbits>
	friend bool operator>=(const quire<nnbits>& lhs, const quire<nnbits>>& rhs);
};
