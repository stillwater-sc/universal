#pragma once
// adaptiveint_impl.hpp: implementation of an adaptive precision binary integer
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>
#include <vector>
#include <map>

#include <universal/number/adaptiveint/exceptions.hpp>

namespace sw { namespace universal {

// forward references
class adaptiveint;
inline adaptiveint& convert(int64_t v, adaptiveint& result);
inline adaptiveint& convert_unsigned(uint64_t v, adaptiveint& result);
bool parse(const std::string& number, adaptiveint& v);

// adaptiveint is an adaptive precision integer type
class adaptiveint {
	using BlockType = uint32_t;
	static constexpr unsigned bitsInBlock = 32;
public:
	adaptiveint() : _sign(false), _blocks(0) { }

	adaptiveint(const adaptiveint&) = default;
	adaptiveint(adaptiveint&&) = default;

	adaptiveint& operator=(const adaptiveint&) = default;
	adaptiveint& operator=(adaptiveint&&) = default;

	// initializers for native types
	adaptiveint(int initial_value)                { *this = initial_value; }
	adaptiveint(long initial_value)               { *this = initial_value; }
	adaptiveint(long long initial_value)          { *this = initial_value; }
	adaptiveint(unsigned long long initial_value) { *this = initial_value; }
	adaptiveint(long double initial_value)        { *this = initial_value; }

	// assignment operators for native types
	adaptiveint& operator=(int rhs)                noexcept { return convert(rhs, *this); }
	adaptiveint& operator=(long rhs)               noexcept { return convert(rhs, *this); }
	adaptiveint& operator=(long long rhs)          noexcept { return convert(rhs, *this); }
	adaptiveint& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs, *this); }
	adaptiveint& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs, *this); }
	adaptiveint& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs, *this); }
	adaptiveint& operator=(long double rhs)        noexcept { return float_assign(rhs); }

	// prefix operators
	adaptiveint operator-() const {
		adaptiveint negated(*this);
		negated.setsign(!_sign);
		return negated;
	}

	// conversion operators
	explicit operator float() const { return float(toNativeFloatingPoint()); }
	explicit operator double() const { return float(toNativeFloatingPoint()); }
	explicit operator long double() const { return toNativeFloatingPoint(); }

	// logic shift operators
	adaptiveint& operator<<=(int shift) {
		if (shift == 0) return *this;
		if (shift < 0) return operator>>=(-shift);

		size_t MSU = _blocks.size() - 1;
		if (shift >= static_cast<int>(bitsInBlock)) {
			int blockShift = shift / static_cast<int>(bitsInBlock);
			if (blockShift > 0) _blocks.resize(_blocks.size() + blockShift, 0ul);
			for (int i = static_cast<int>(MSU); i >= blockShift; --i) {
				_blocks[static_cast<size_t>(i)] = _blocks[static_cast<size_t>(i) - static_cast<size_t>(blockShift)];
			}
			for (int i = blockShift - 1; i >= 0; --i) {
				_blocks[static_cast<size_t>(i)] = BlockType(0);
			}
			// adjust the shift
			shift -= static_cast<int>(blockShift * bitsInBlock);
			if (shift == 0) return *this;
		}
		if (MSU > 0) {
			// construct the mask for the upper bits in the block that needs to move to the higher word
			BlockType mask = 0xFFFF'FFFFul << (bitsInBlock - shift);
			for (size_t i = MSU; i > 0; --i) {
				_blocks[static_cast<size_t>(i)] <<= shift;
				// mix in the bits from the right
				BlockType bits = BlockType(mask & _blocks[i - 1]);
				_blocks[static_cast<size_t>(i)] |= (bits >> (bitsInBlock - shift));
			}
		}
	}
	adaptiveint& operator>>=(int shift) {
		if (shift == 0) return *this;
		if (shift < 0) return operator>>=(-shift);
		if (shift > static_cast<int>(nbits())) {
			setzero();
			return *this;
		}
		size_t MSU = _blocks.size() - 1;
		size_t blockShift = 0;
		if (shift >= static_cast<int>(bitsInBlock)) {
			blockShift = shift / bitsInBlock;
			if (MSU >= blockShift) {
				// shift by blocks
				for (size_t i = 0; i <= MSU - blockShift; ++i) {
					_blocks[i] = _blocks[i + blockShift];
				}
			}
			// adjust the shift
			shift -= static_cast<int>(blockShift * bitsInBlock);
			if (shift == 0) {
				// clean up the blocks we have shifted clean
//				shift += static_cast<int>(blockShift * bitsInBlock);
//				for (unsigned i = nbits - shift; i < nbits; ++i) {
//					this->setbit(i, false);
//				}
				return *this;
			}
		}
		if (MSU > 0) {
			BlockType mask = 0xFFFF'FFFFul;
			mask >>= (bitsInBlock - shift); // this is a mask for the lower bits in the block that need to move to the lower word
			for (size_t i = 0; i < MSU; ++i) {  // TODO: can this be improved? we should not have to work on the upper blocks in case we block shifted
				_blocks[i] >>= shift;
				// mix in the bits from the left
				BlockType bits = BlockType(mask & _blocks[i + 1]);
				_blocks[i] |= (bits << (bitsInBlock - shift));
			}
		}
		_blocks[MSU] >>= shift;
		// remove leading 0 blocks
		// TODO
	}
	// arithmetic operators
	adaptiveint& operator+=(const adaptiveint& rhs) {
		auto lhsSize = _blocks.size();
		auto rhsSize = rhs._blocks.size();
//		auto minLimbs = (lhsSize < rhsSize) ? lhsSize : rhsSize;

		std::uint64_t carry{ 0 };
		std::vector<BlockType>::iterator li = _blocks.begin();
		std::vector<BlockType>::const_iterator ri = rhs._blocks.begin();
		while (li != _blocks.end()) {
			carry += static_cast<std::uint64_t>(*li) + static_cast<std::uint64_t>(*ri);
			*li = static_cast<BlockType>(carry);
			carry >>= bitsInBlock;
			++li; ++ri;
		}
		if (carry == 0x1ull) {
			_blocks.push_back(static_cast<BlockType>(carry));
		}
		return *this;
	}
	adaptiveint& operator+=(long long rhs) {
		return *this += adaptiveint(rhs);
	}
	adaptiveint& operator-=(const adaptiveint& rhs) {
		return *this;
	}
	adaptiveint& operator-=(long long rhs) {
		return *this -= adaptiveint(rhs);
	}
	adaptiveint& operator*=(const adaptiveint& rhs) {
		return *this;
	}
	adaptiveint& operator*=(long long rhs) {
		return *this *= adaptiveint(rhs);
	}
	adaptiveint& operator/=(const adaptiveint& rhs) {
		return *this;
	}
	adaptiveint& operator/=(long long rhs) {
		return *this /= adaptiveint(rhs);
	}
	adaptiveint& operator%=(const adaptiveint& rhs) {
		return *this;
	}
	adaptiveint& operator%=(long long rhs) {
		return *this %= adaptiveint(rhs);
	}
	// modifiers
	inline void clear() noexcept { _sign = false; _blocks.clear(); }
	inline void setzero() noexcept { clear(); }
	inline void setsign(bool sign = true) noexcept { _sign = sign; }
	// use un-interpreted raw bits to set the bits of the adaptiveint
	inline void setbits(unsigned long long value) {
		clear();
		std::uint32_t low  = static_cast<std::uint32_t>(value & 0x0000'0000'FFFF'FFFF);
		std::uint32_t high = static_cast<std::uint32_t>((value & 0xFFFF'FFFF'0000'0000) >> bitsInBlock);
		if (high > 0) {
			_blocks.push_back(high);
			_blocks.push_back(low);
		}
		else if (low > 0) {
			_blocks.push_back(low);
		}
	}
	inline adaptiveint& assign(const std::string& txt) {
		return *this;
	}

	// selectors
	inline bool iszero() const noexcept { return !_sign && _blocks.size() == 0; }
	inline bool isone()  const noexcept { return true; }
	inline bool isodd()  const noexcept { return (_blocks.size() > 0) ? (_blocks[0] & 0x1) : false; }
	inline bool iseven() const noexcept { return !isodd(); }
	inline bool ispos()  const noexcept { return !_sign; }
	inline bool isneg()  const noexcept { return _sign; }

	inline bool sign()   const noexcept { return _sign; }
	inline int scale()   const noexcept { return findMsb(); }

	inline std::uint32_t block(unsigned b) const noexcept {
		if (b < _blocks.size()) {
			return _blocks[b];
		}
		return 0u;
	}
	inline unsigned limbs() const noexcept { return static_cast<unsigned>(_blocks.size()); }

	inline unsigned nbits() const noexcept { return static_cast<unsigned>(_blocks.size() * sizeof(BlockType) * 8); }

	// findMsb takes an adaptiveint reference and returns the position of the most significant bit, -1 if v == 0
	inline int findMsb() const noexcept {
		int nrBlocks = static_cast<int>(_blocks.size());
		if (nrBlocks == 0) return -1; // no significant bit found, all bits are zero
		int msb = nrBlocks * static_cast<int>(bitsInBlock);
		for (int b = nrBlocks - 1; b >= 0; --b) {
			std::uint32_t segment = _blocks[static_cast<size_t>(b)];
			std::uint32_t mask = 0x8000'0000ul;
			for (int i = bitsInBlock - 1; i >= 0; --i) {
				--msb;
				if (segment & mask) return msb;
				mask >>= 1;
			}
		}
		return -1; // no significant bit found, all bits are zero
	}

	// convert to string containing digits number of digits
	std::string str(size_t nrDigits = 0) const {

		return std::string("tbd");
	}

protected:
	bool                   _sign;    // sign of the number: -1 if true, +1 if false, zero is positive
	std::vector<BlockType> _blocks;  // building blocks representing a 1's complement magnitude

	// HELPER methods



	// convert to native floating-point, use conversion rules to cast down to float and double
	long double toNativeFloatingPoint() const {
		long double ld = 0;
		return ld;
	}

	template<typename Ty>
	adaptiveint& float_assign(Ty& rhs) {
		clear();
		long long base = (long long)rhs;
		*this = base;
		return *this;
	}

private:

	// adaptiveint - adaptiveint logic comparisons
	friend bool operator==(const adaptiveint& lhs, const adaptiveint& rhs);

	// adaptiveint - literal logic comparisons
	friend bool operator==(const adaptiveint& lhs, const long long rhs);

	// literal - adaptiveint logic comparisons
	friend bool operator==(const long long lhs, const adaptiveint& rhs);

	// find the most significant bit set
	friend signed findMsb(const adaptiveint& v);
};

inline adaptiveint& convert(long long v, adaptiveint& result) {
	if (0 == v) {
		result.setzero();
	}
	else {
		result.clear();
		if (v < 0) {
			result.setsign(true);
			result.setbits(static_cast<unsigned long long>(-v));
		}
		else {
			result.setbits(static_cast<unsigned long long>(v)); // TODO: what about -2^(n-1)
		}
	}
	return result;
}

inline adaptiveint& convert_unsigned(unsigned long long v, adaptiveint& result) {
	if (0 == v) {
		result.setzero();
	}
	else {
		result.setbits(static_cast<unsigned long long>(v));
	}
	return result;
}

////////////////////////    adaptiveint functions   /////////////////////////////////


inline adaptiveint abs(const adaptiveint& a) {
	return a; // (a < 0 ? -a : a);
}

////////////////////////    INTEGER operators   /////////////////////////////////

/// stream operators

// read a adaptiveint ASCII format and make a binary adaptiveint out of it

bool parse(const std::string& number, adaptiveint& value) {
	bool bSuccess = false;

	return bSuccess;
}

std::string convert_to_string(std::ios_base::fmtflags flags, const adaptiveint& n) {
	using BlockType = std::uint32_t; // TODO: how to associate this to the configuration of adaptiveint?

	if (n.limbs() == 0) return std::string("0");

	// set the base of the target number system to convert to
	int base = 10;
	if ((flags & std::ios_base::oct) == std::ios_base::oct) base = 8;
	if ((flags & std::ios_base::hex) == std::ios_base::hex) base = 16;

	unsigned nbits = n.limbs() * sizeof(BlockType) * 8;

	std::string result;
	if (base == 8 || base == 16) {
		if (n.sign()) return std::string("negative value: ignored");

		int shift = (base == 8 ? 3 : 4);
		BlockType mask = static_cast<BlockType>((1u << shift) - 1);
		adaptiveint t(n);
		result.assign(nbits / shift + ((nbits % shift) ? 1 : 0), '0');
		std::string::difference_type pos = static_cast<int>(result.size()) - 1;
		for (size_t i = 0; i < nbits / static_cast<size_t>(shift); ++i) {
			char c = '0' + static_cast<char>(t.block(0) & mask);
			if (c > '9')
				c += 'A' - '9' - 1;
			result[static_cast<size_t>(pos--)] = c;
			t >>= shift;
		}
		if (nbits % shift) {
			mask = static_cast<BlockType>((1u << (nbits % shift)) - 1);
			char c = '0' + static_cast<char>(t.block(0) & mask);
			if (c > '9')
				c += 'A' - '9';
			result[static_cast<size_t>(pos)] = c;
		}
		//
		// Get rid of leading zeros:
		//
		std::string::size_type n = result.find_first_not_of('0');
		if (!result.empty() && (n == std::string::npos))
			n = result.size() - 1;
		result.erase(0, n);
		if (flags & std::ios_base::showbase) {
			const char* pp = base == 8 ? "0" : "0x";
			result.insert(static_cast<std::string::size_type>(0), pp);
		}
	}
	else {
		result.assign(nbits / 3 + 1u, '0');
		std::string::difference_type pos = static_cast<int>(result.size()) - 1;
		adaptiveint t(n);

		unsigned block10{ 1'000'000'000 };
		unsigned digits_in_block10 = 9;
		while (!t.iszero()) {

			//adaptiveint t2 = t / block10;
			//adaptiveint r  = t % block10;
			adaptiveint t2(t); t2 /= block10;
			adaptiveint r(t); r %= block10;
			//			std::cout << "t  " << long(t) << '\n';
			//			std::cout << "t2 " << long(t2) << '\n';
			//			std::cout << "r  " << long(r) << '\n';
			t = t2;
			BlockType v = r.block(0);
			//			std::cout << "v  " << uint32_t(v) << '\n';
			for (unsigned i = 0; i < digits_in_block10; ++i) {
				//				std::cout << i << " " << (v / 10) << " " << (v % 10) << '\n';
				char c = '0' + static_cast<char>(v % 10);
				v /= 10;
				result[static_cast<size_t>(pos)] = c;
				//				std::cout << result << '\n';
				if (pos-- == 0)
					break;
			}
		}

		std::string::size_type firstDigit = result.find_first_not_of('0');
		result.erase(0, firstDigit);
		if (result.empty())
			result = "0";
		if (n.isneg())
			result.insert(static_cast<std::string::size_type>(0), 1, '-');
		else if (flags & std::ios_base::showpos)
			result.insert(static_cast<std::string::size_type>(0), 1, '+');
	}
	return result;
}

// generate an adaptiveint format ASCII format
inline std::ostream& operator<<(std::ostream& ostr, const adaptiveint& i) {
	std::streamsize prec = ostr.precision();
	std::string s = convert_to_string(ostr.flags(), i);
	std::streamsize width = ostr.width();
	if (width > static_cast<std::streamsize>(s.size())) {
		char fill = ostr.fill();
		if ((ostr.flags() & std::ios_base::left) == std::ios_base::left)
			s.append(static_cast<std::string::size_type>(width - s.size()), fill);
		else
			s.insert(static_cast<std::string::size_type>(0), static_cast<std::string::size_type>(width - s.size()), fill);
	}
	return ostr << s;
}

// read an ASCII adaptiveint format

inline std::istream& operator>>(std::istream& istr, adaptiveint& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators

inline std::string to_binary(const adaptiveint& a, bool nibbleMarker = true) {
	if (a.limbs() == 0) return std::string("0x0");

	std::stringstream s;
	s << "0x";
	for (int b = static_cast<int>(a.limbs()) - 1; b >= 0; --b) {
		std::uint32_t segment = a.block(static_cast<size_t>(b));
		std::uint32_t mask = 0x8000'0000;
		for (int i = 31; i >= 0; --i) {
			s << ((segment & mask) ? '1' : '0');
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
			mask >>= 1;
		}
	}

	return s.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// adaptiveint - adaptiveint binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths

inline bool operator==(const adaptiveint& lhs, const adaptiveint& rhs) {
	return true;
}

inline bool operator!=(const adaptiveint& lhs, const adaptiveint& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const adaptiveint& lhs, const adaptiveint& rhs) {
	return false; // lhs and rhs are the same
}

inline bool operator> (const adaptiveint& lhs, const adaptiveint& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(const adaptiveint& lhs, const adaptiveint& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const adaptiveint& lhs, const adaptiveint& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// adaptiveint - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths

inline bool operator==(const adaptiveint& lhs, long long rhs) {
	return operator==(lhs, adaptiveint(rhs));
}

inline bool operator!=(const adaptiveint& lhs, long long rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (const adaptiveint& lhs, long long rhs) {
	return operator<(lhs, adaptiveint(rhs));
}

inline bool operator> (const adaptiveint& lhs, long long rhs) {
	return operator< (adaptiveint(rhs), lhs);
}

inline bool operator<=(const adaptiveint& lhs, long long rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(const adaptiveint& lhs, long long rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - adaptiveint binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths


inline bool operator==(long long lhs, const adaptiveint& rhs) {
	return operator==(adaptiveint(lhs), rhs);
}

inline bool operator!=(long long lhs, const adaptiveint& rhs) {
	return !operator==(lhs, rhs);
}

inline bool operator< (long long lhs, const adaptiveint& rhs) {
	return operator<(adaptiveint(lhs), rhs);
}

inline bool operator> (long long lhs, const adaptiveint& rhs) {
	return operator< (rhs, lhs);
}

inline bool operator<=(long long lhs, const adaptiveint& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

inline bool operator>=(long long lhs, const adaptiveint& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// adaptiveint - adaptiveint binary arithmetic operators

inline adaptiveint operator+(const adaptiveint& lhs, const adaptiveint& rhs) {
	adaptiveint sum = lhs;
	sum += rhs;
	return sum;
}

inline adaptiveint operator-(const adaptiveint& lhs, const adaptiveint& rhs) {
	adaptiveint diff = lhs;
	diff -= rhs;
	return diff;
}

inline adaptiveint operator*(const adaptiveint& lhs, const adaptiveint& rhs) {
	adaptiveint product = lhs;
	product *= rhs;
	return product;
}

inline adaptiveint operator/(const adaptiveint& lhs, const adaptiveint& rhs) {
	adaptiveint ratio = lhs;
	ratio /= rhs;
	return ratio;
}

inline adaptiveint operator%(const adaptiveint& lhs, const adaptiveint& rhs) {
	adaptiveint remainder = lhs;
	remainder %= rhs;
	return remainder;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// adaptiveint - literal binary arithmetic operators

inline adaptiveint operator+(const adaptiveint& lhs, long long rhs) {
	return operator+(lhs, adaptiveint(rhs));
}

inline adaptiveint operator-(const adaptiveint& lhs, long long rhs) {
	return operator-(lhs, adaptiveint(rhs));
}

inline adaptiveint operator*(const adaptiveint& lhs, long long rhs) {
	return operator*(lhs, adaptiveint(rhs));
}

inline adaptiveint operator/(const adaptiveint& lhs, long long rhs) {
	return operator/(lhs, adaptiveint(rhs));
}

inline adaptiveint operator%(const adaptiveint& lhs, long long rhs) {
	return operator/(lhs, adaptiveint(rhs));
}

inline adaptiveint operator/(const adaptiveint& lhs, unsigned long long rhs) {
	return operator/(lhs, adaptiveint(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - adaptiveint binary arithmetic operators

inline adaptiveint operator+(long long lhs, const adaptiveint& rhs) {
	return operator+(adaptiveint(lhs), rhs);
}

inline adaptiveint operator-(long long lhs, const adaptiveint& rhs) {
	return operator-(adaptiveint(lhs), rhs);
}

inline adaptiveint operator*(long long lhs, const adaptiveint& rhs) {
	return operator*(adaptiveint(lhs), rhs);
}

inline adaptiveint operator/(long long lhs, const adaptiveint& rhs) {
	return operator/(adaptiveint(lhs), rhs);
}

inline adaptiveint operator%(long long lhs, const adaptiveint& rhs) {
	return operator/(adaptiveint(lhs), rhs);
}

}} // namespace sw::universal
