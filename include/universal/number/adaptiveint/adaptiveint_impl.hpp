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

#include <universal/number/adaptiveint/exceptions.hpp>

// supporting types and functions
#include <universal/native/ieee754.hpp>

namespace sw { namespace universal {

// forward references
template<typename BlockType> class adaptiveint;
template<typename BlockType> bool parse(const std::string& number, adaptiveint<BlockType>& v);

// adaptiveint is an adaptive precision integer type
template<typename BlockType = std::uint32_t>
class adaptiveint {
public:
	using bt = BlockType;
	static constexpr unsigned bitsInBlock = sizeof(BlockType) * 8;
	static constexpr bt       ALL_ONES = bt(~0); // block type specific all 1's value
	static constexpr uint64_t BASE = (ALL_ONES + 1ull);
	static_assert(bitsInBlock <= 32, "BlockType must be one of [uint8_t, uint16_t, uint32_t]");

	adaptiveint() : _sign(false), _block(0) { }

	adaptiveint(const adaptiveint&) = default;
	adaptiveint(adaptiveint&&) = default;

	adaptiveint& operator=(const adaptiveint&) = default;
	adaptiveint& operator=(adaptiveint&&) = default;

	// initializers for native types
	adaptiveint(short initial_value)              { *this = initial_value; }
	adaptiveint(int initial_value)                { *this = initial_value; }
	adaptiveint(long initial_value)               { *this = initial_value; }
	adaptiveint(long long initial_value)          { *this = initial_value; }
	adaptiveint(unsigned int initial_value)       { *this = initial_value; }
	adaptiveint(unsigned long initial_value)      { *this = initial_value; }
	adaptiveint(unsigned long long initial_value) { *this = initial_value; }
	adaptiveint(float initial_value)              { *this = initial_value; }
	adaptiveint(double initial_value)             { *this = initial_value; }

	// assignment operators for native types
	adaptiveint& operator=(int rhs)                noexcept { return assign_signed(rhs); }
	adaptiveint& operator=(long rhs)               noexcept { return assign_signed(rhs); }
	adaptiveint& operator=(long long rhs)          noexcept { return assign_signed(rhs); }
	adaptiveint& operator=(unsigned int rhs)       noexcept { return assign_unsigned(rhs); }
	adaptiveint& operator=(unsigned long rhs)      noexcept { return assign_unsigned(rhs); }
	adaptiveint& operator=(unsigned long long rhs) noexcept { return assign_unsigned(rhs); }
	adaptiveint& operator=(float rhs)              noexcept { return assign_native_ieee(rhs); }
	adaptiveint& operator=(double rhs)             noexcept { return assign_native_ieee(rhs); }

	// prefix operators
	adaptiveint operator-() const {
		adaptiveint negated(*this);
		negated.setsign(!_sign);
		return negated;
	}

	// conversion operators
	explicit operator int() const noexcept         { return convert_to_native_integer<int>(); }
	explicit operator long() const noexcept        { return convert_to_native_integer<long>(); }
	explicit operator long long() const noexcept   { return convert_to_native_integer<long long>(); }
	explicit operator float() const noexcept       { return convert_to_native_ieee<float>(); }
	explicit operator double() const noexcept      { return convert_to_native_ieee<double>(); }

#if LONG_DOUBLE_SUPPORT
	adaptiveint(long double initial_value) { *this = initial_value; }
	adaptiveint& operator=(long double rhs)        noexcept { return assign_native_ieee(rhs); }
	explicit operator long double() const noexcept { return convert_to_native_ieee<long double>(); }
#endif

	// logic shift operators
	adaptiveint& operator<<=(int shift) {
		if (shift == 0) return *this;
		if (shift < 0) return operator>>=(-shift);

		// by default extend the limbs by 1
		_block.push_back(0);
		size_t MSU = _block.size() - 1;
		if (shift >= static_cast<int>(bitsInBlock)) {
			int blockShift = shift / static_cast<int>(bitsInBlock);
			if (blockShift > 0) _block.resize(_block.size() + blockShift, 0ul);
			MSU = _block.size() - 1;
			for (int i = static_cast<int>(MSU); i >= blockShift; --i) {
				_block[static_cast<size_t>(i)] = _block[static_cast<size_t>(i) - static_cast<size_t>(blockShift)];
			}
			for (int i = blockShift - 1; i >= 0; --i) {
				_block[static_cast<size_t>(i)] = BlockType(0);
			}
			// adjust the shift
			shift -= static_cast<int>(blockShift * bitsInBlock);
			if (shift == 0) return *this;
		}
		if (MSU > 0) {
			// construct the mask for the upper bits in the block that needs to move to the higher word
			BlockType mask = static_cast<BlockType>(0xFFFF'FFFFul << (bitsInBlock - shift));
			for (size_t i = MSU; i > 0; --i) {
				_block[static_cast<size_t>(i)] <<= shift;
				// mix in the bits from the right
				BlockType bits = BlockType(mask & _block[i - 1]);
				_block[static_cast<size_t>(i)] |= (bits >> (bitsInBlock - shift));
			}
		}
		return *this;
	}
	adaptiveint& operator>>=(int shift) {
		if (shift == 0) return *this;
		if (shift < 0) return operator>>=(-shift);
		if (shift > static_cast<int>(nbits())) {
			setzero();
			return *this;
		}
		size_t MSU = _block.size() - 1;
		size_t blockShift = 0;
		if (shift >= static_cast<int>(bitsInBlock)) {
			blockShift = shift / bitsInBlock;
			if (MSU >= blockShift) {
				// shift by blocks
				for (size_t i = 0; i <= MSU - blockShift; ++i) {
					_block[i] = _block[i + blockShift];
					_block[i + blockShift] = 0; // null the upper block
				}
			}
			// adjust the shift
			shift -= static_cast<int>(blockShift * bitsInBlock);
			if (shift == 0) {
				remove_leading_zeros();
				return *this;
			}
		}
		if (MSU > 0) {
			BlockType mask = static_cast<BlockType>(0xFFFF'FFFFul);
			mask >>= (bitsInBlock - shift); // this is a mask for the lower bits in the block that need to move to the lower word
			for (size_t i = 0; i < MSU; ++i) {
				_block[i] >>= shift;
				// mix in the bits from the left
				BlockType bits = BlockType(mask & _block[i + 1]);
				_block[i] |= (bits << (bitsInBlock - shift));
			}
			_block[MSU] >>= shift;
		}
		else {
			_block[0] >>= shift;
		}
		remove_leading_zeros();
		return *this;
	}
	
	// arithmetic operators
	adaptiveint& operator+=(const adaptiveint& rhs) {
		if (sign() != rhs.sign()) {
			if (sign()) {
				adaptiveint negated(*this);
				negated.setsign(false);
				*this = rhs - negated;
				return *this;
			}
			else {
				adaptiveint negated(rhs);
				negated.setsign(false);
				*this -= negated;
				return *this;
			}
		}
		auto lhsSize = _block.size();
		auto rhsSize = rhs._block.size();
		if (lhsSize < rhsSize) {
			_block.resize(rhsSize, 0);
		}
		std::uint64_t carry{ 0 };
		typename std::vector<BlockType>::iterator li = _block.begin();
		typename std::vector<BlockType>::const_iterator ri = rhs._block.begin();
		while (li != _block.end()) {
			if (ri != rhs._block.end()) {
				carry += static_cast<std::uint64_t>(*li) + static_cast<std::uint64_t>(*ri);
				++ri;
			}
			else {
				carry += static_cast<std::uint64_t>(*li);
			}
			*li = static_cast<BlockType>(carry);
			carry >>= bitsInBlock;
			++li; 
		}
		if (carry == 0x1ull) {
			_block.push_back(static_cast<BlockType>(carry));
		}
		return *this;
	}
	adaptiveint& operator+=(long long rhs) {
		return *this += adaptiveint(rhs);
	}
	adaptiveint& operator-=(const adaptiveint& rhs) {
		if (rhs.sign()) {
			adaptiveint negated(rhs);
			negated.setsign(false);
			return *this += negated;
		}
		int magnitude = compare_magnitude(*this, rhs); // if -1 result is going to be negative
		auto lhsSize = _block.size();
		auto rhsSize = rhs._block.size();

		auto overlap = (lhsSize < rhsSize ? lhsSize : rhsSize);
		auto extent = (lhsSize < rhsSize ? rhsSize : lhsSize);
		
		if (lhsSize < rhsSize) _block.resize(rhsSize + 1);
		std::uint64_t borrow{ 0 };
		
		typename std::vector<BlockType>::const_iterator aIter, bIter;
		if (magnitude == 1) {
			aIter = _block.begin();
			bIter = rhs._block.begin();
		}
		else {
			aIter = rhs._block.begin();
			bIter = _block.begin();
		}
		unsigned i{ 0 };
		while (i < overlap) {
			borrow = static_cast<std::uint64_t>(*aIter) - static_cast<std::uint64_t>(*bIter) + borrow;
			_block[i] = static_cast<BlockType>(borrow);
			borrow = (borrow >> bitsInBlock) & 0x1u;
			++i; ++aIter; ++bIter;
		}
		while (borrow && (i < extent)) {
			borrow = static_cast<BlockType>(*aIter) - borrow;
			_block[i] = static_cast<BlockType>(borrow);
			borrow = (borrow >> bitsInBlock) & 1u;
			++i; ++aIter;
		}
		remove_leading_zeros();
		setsign(magnitude == -1);
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
		adaptiveint q, r;
		q.reduce(*this, rhs, r);
		*this = q;
		return *this;
	}
	adaptiveint& operator/=(long long rhs) {
		return *this /= adaptiveint(rhs);
	}
	adaptiveint& operator%=(const adaptiveint& rhs) {
		adaptiveint q, a(*this), r;
		q.reduce(a, rhs, r);
		*this = r;
		return *this;
	}
	adaptiveint& operator%=(long long rhs) {
		return *this %= adaptiveint(rhs);
	}
	
	// reduce returns the ratio and remainder of a and b in *this and r
	void reduce(const adaptiveint& a, const adaptiveint& b, adaptiveint& r) {
		if (b.iszero()) {
#if ADAPTIVEINT_THROW_ARITHMETIC_EXCEPTION
			throw adaptiveint_divide_by_zero{};
#else
			std::cerr << "adaptiveint_divide_by_zero\n";
			return;
#endif // ADAPTIVEINT_THROW_ARITHMETIC_EXCEPTION
		}
		clear();
		r.clear();
		if (a.iszero()) return;

		_sign = a.sign() ^ b.sign();
		size_t aBlocks = a.limbs();
		size_t bBlocks = b.limbs();
		if (aBlocks == 1 && aBlocks == bBlocks) { // completely reduce this to native div and rem
			std::uint64_t a0 = a._block[0];
			std::uint64_t b0 = b._block[0];
			*this = static_cast<BlockType>(a0 / b0);
			r = static_cast<BlockType>(a0 % b0);
		}
		else {
			// filter out the easy stuff
			if (a < b) { r = a; clear(); return; }

			// determine first non-zero limbs
			unsigned m{ 0 }, n{ 0 };
			for (size_t i = aBlocks; i > 0; --i) {
				if (a._block[i - 1] != 0) {
					m = static_cast<unsigned>(i);
					break;
				}
			}
			for (size_t i = bBlocks; i > 0; --i) {
				if (b._block[i - 1] != 0) {
					n = static_cast<unsigned>(i);
					break;
				}
			}

			// single limb divisor
			if (n == 1) {
				_block.resize(m);
				std::uint64_t remainder{ 0 };
				auto divisor = b.block(0);
				for (unsigned j = m; j > 0; --j) {
					std::uint64_t dividend = remainder * BASE + static_cast<std::uint64_t>(a.block(j - 1));
					std::uint64_t limbQuotient = dividend / divisor;
					_block[j - 1] = static_cast<BlockType>(limbQuotient);
					remainder = dividend - limbQuotient * divisor;
				}
				remove_leading_zeros();
				r.setblock(0, static_cast<BlockType>(remainder));
				return;
			}

			// Knuth's algorithm calculates a normalization factor d
			// that perfectly aligns b so that b0 >= floor(BASE/2),
			// a requirement for the relationship: (qHat - 2) <= q <= qHat

			int shift = nlz(b.block(n - 1));
			adaptiveint normalized_a;
			normalized_a.setblock(m, static_cast<BlockType>((a.block(m - 1) >> (bitsInBlock - shift))));
			for (unsigned i = m - 1; i > 0; --i) {
				normalized_a.setblock(i, static_cast<BlockType>((a.block(i) << shift) | (a.block(i - 1) >> (bitsInBlock - shift))));
			}
			normalized_a.setblock(0, static_cast<BlockType>(a.block(0) << shift));
			// normalize b
			adaptiveint normalized_b;
			unsigned n_minus_1 = n - 1;
			for (unsigned i = n_minus_1; i > 0; --i) {
				normalized_b.setblock(i, static_cast<BlockType>((b.block(i) << shift) | (b.block(i - 1) >> (bitsInBlock - shift))));
			}
			normalized_b.setblock(0, static_cast<BlockType>(b.block(0) << shift));

			//std::cout << "normalized a : " << normalized_a.showLimbs() << " : " << normalized_a.showLimbValues() << '\n';
			//std::cout << "normalized b :             " << normalized_b.showLimbs() << " : " << normalized_b.showLimbValues() << '\n';

			// divide by limb
			std::uint64_t divisor = normalized_b._block[n - 1];
			std::uint64_t v_nminus2 = normalized_b._block[n - 2]; // n > 1 at this point
			for (int j = static_cast<int>(m - n); j >= 0; --j) {
				std::uint64_t dividend = normalized_a.block(j + n) * BASE + normalized_a.block(j + n - 1);
				std::uint64_t qhat = dividend / divisor;
				std::uint64_t rhat = dividend - qhat * divisor;

				while (qhat >= BASE || qhat * v_nminus2 > BASE * rhat + normalized_a.block(j + n - 2)) {
					--qhat;
					rhat += divisor;
					if (rhat < BASE) continue;
				}
				std::uint64_t borrow{ 0 };
				std::uint64_t diff{ 0 };
				for (unsigned i = 0; i < n; ++i) {
					std::uint64_t p = qhat * normalized_b.block(i);
					diff = normalized_a.block(i + j) - static_cast<BlockType>(p) - borrow;
					normalized_a.setblock(i + j, static_cast<BlockType>(diff));
					borrow = (p >> bitsInBlock) - (diff >> bitsInBlock);
				}
				std::int64_t signedBorrow = static_cast<int64_t>(normalized_a.block(j + n) - borrow);
				normalized_a.setblock(j + n, static_cast<BlockType>(signedBorrow));

				//std::cout << "   updated a : " << normalized_a.showLimbs() << " : " << normalized_a.showLimbValues() << '\n';

				setblock(static_cast<unsigned>(j), static_cast<BlockType>(qhat));
				if (signedBorrow < 0) { // subtracted too much, add back
					std::cout << "subtracted too much, add back\n";
					setblock(static_cast<size_t>(j), static_cast<BlockType>(_block[static_cast<size_t>(j)] - 1));
					std::uint64_t carry{ 0 };
					for (unsigned i = 0; i < n; ++i) {
						carry += static_cast<std::uint64_t>(normalized_a.block(i + j)) + static_cast<std::uint64_t>(normalized_b.block(i));
						normalized_a.setblock(i + j, static_cast<BlockType>(carry));
						carry >>= 32;
					}
					BlockType rectified = static_cast<BlockType>(normalized_a.block(j + n) + carry);
					normalized_a.setblock(j + n, rectified);
				}
				//std::cout << "   updated a : " << normalized_a.showLimbs() << " : " << normalized_a.showLimbValues() << '\n';
			}

			// remainder needs to be normalized
			for (unsigned i = 0; i < n - 1; ++i) {
				std::uint64_t remainder = static_cast<std::uint64_t>(normalized_a.block(i) >> shift);
				remainder |= (static_cast<std::uint64_t>(normalized_a.block(i + 1)) << (32 - shift));
				r.setblock(i, static_cast<BlockType>(remainder));
			}
			r.setblock(n - 1, static_cast<BlockType>(normalized_a.block(n - 1) >> shift));
		}
		remove_leading_zeros();
	}

	// modifiers
	inline void clear() noexcept { _sign = false; _block.clear(); }
	inline void setzero() noexcept { clear(); }
	inline void setsign(bool sign = true) noexcept { _sign = sign; }
	// use un-interpreted raw bits to set the bits of the adaptiveint
	inline void setbits(unsigned long long value) {
		clear();
		if constexpr (bitsInBlock == 8) {
			std::uint8_t byte0 = static_cast<std::uint8_t>(value & 0x0000'0000'0000'00FF);
			std::uint8_t byte1 = static_cast<std::uint8_t>((value & 0x0000'0000'0000'FF00) >> 8);
			std::uint8_t byte2 = static_cast<std::uint8_t>((value & 0x0000'0000'00FF'0000) >> 16);
			std::uint8_t byte3 = static_cast<std::uint8_t>((value & 0x0000'0000'FF00'0000) >> 24);
			std::uint8_t byte4 = static_cast<std::uint8_t>((value & 0x0000'00FF'0000'0000) >> 32);
			std::uint8_t byte5 = static_cast<std::uint8_t>((value & 0x0000'FF00'0000'0000) >> 40);
			std::uint8_t byte6 = static_cast<std::uint8_t>((value & 0x00FF'0000'0000'0000) >> 48);
			std::uint8_t byte7 = static_cast<std::uint8_t>((value & 0xFF00'0000'0000'0000) >> 56);
			if (byte7 > 0) {
				_block.push_back(byte0);
				_block.push_back(byte1);
				_block.push_back(byte2);
				_block.push_back(byte3);
				_block.push_back(byte4);
				_block.push_back(byte5);
				_block.push_back(byte6);
				_block.push_back(byte7);
			}
			else if (byte6 > 0) {
				_block.push_back(byte0);
				_block.push_back(byte1);
				_block.push_back(byte2);
				_block.push_back(byte3);
				_block.push_back(byte4);
				_block.push_back(byte5);
				_block.push_back(byte6);
			}
			else if (byte5 > 0) {
				_block.push_back(byte0);
				_block.push_back(byte1);
				_block.push_back(byte2);
				_block.push_back(byte3);
				_block.push_back(byte4);
				_block.push_back(byte5);
			}
			else if (byte4 > 0) {
				_block.push_back(byte0);
				_block.push_back(byte1);
				_block.push_back(byte2);
				_block.push_back(byte3);
				_block.push_back(byte4);
			}
			else if (byte3 > 0) {
				_block.push_back(byte0);
				_block.push_back(byte1);
				_block.push_back(byte2);
				_block.push_back(byte3);
			}
			else if (byte2 > 0) {
				_block.push_back(byte0);
				_block.push_back(byte1);
				_block.push_back(byte2);
			}
			else if (byte1 > 0) {
				_block.push_back(byte0);
				_block.push_back(byte1);
			}
			else if (byte0 > 0) {
				_block.push_back(byte0);
			}
			else {
				_block.clear();
			}
		}
		else if constexpr (bitsInBlock == 16) {
			std::uint16_t word0 = static_cast<std::uint16_t>( value & 0x0000'0000'0000'FFFF);
			std::uint16_t word1 = static_cast<std::uint16_t>((value & 0x0000'0000'FFFF'0000) >> 16);
			std::uint16_t word2 = static_cast<std::uint16_t>((value & 0x0000'FFFF'0000'0000) >> 32);
			std::uint16_t word3 = static_cast<std::uint16_t>((value & 0xFFFF'0000'0000'0000) >> 48);
			if (word3 > 0) {
				_block.push_back(word0);
				_block.push_back(word1);
				_block.push_back(word2);
				_block.push_back(word3);
			}
			else if (word2 > 0) {
				_block.push_back(word0);
				_block.push_back(word1);
				_block.push_back(word2);
			}
			else if (word1 > 0) {
				_block.push_back(word0);
				_block.push_back(word1);
			}
			else if (word0 > 0) {
				_block.push_back(word0);
			}
			else {
				_block.clear();
			}
		}
		else if constexpr (bitsInBlock == 32) {
			std::uint32_t low  = static_cast<std::uint32_t>(value & 0x0000'0000'FFFF'FFFF);
			std::uint32_t high = static_cast<std::uint32_t>((value & 0xFFFF'FFFF'0000'0000) >> bitsInBlock);
			if (high > 0) {
				_block.push_back(low);
				_block.push_back(high);
			}
			else if (low > 0) {
				_block.push_back(low);
			}
			else {
				_block.clear();
			}
		}
	}
	inline void setblock(unsigned i, BlockType value) noexcept {
		if (i >= _block.size()) _block.resize(i+1);
		_block[i] = value;
	}
	inline adaptiveint& assign(const std::string& txt) {
		return *this;
	}

	// selectors
	inline bool iszero() const noexcept { return !_sign && (_block.size() == 0 || ((_block.size() == 1) && _block[0] == bt(0))); }
	inline bool isone()  const noexcept { return true; }
	inline bool isodd()  const noexcept { return (_block.size() > 0) ? (_block[0] & 0x1) : false; }
	inline bool iseven() const noexcept { return !isodd(); }
	inline bool ispos()  const noexcept { return !_sign; }
	inline bool isneg()  const noexcept { return _sign; }

	inline bool test(unsigned index) const noexcept {
		if (index < nbits()) {
			unsigned blockIndex = index / bitsInBlock;
			unsigned bitIndexInBlock = index % bitsInBlock;
			BlockType data = _block[blockIndex];
			BlockType mask = (0x1u << bitIndexInBlock);
			if (data & mask) return true;
		}
		return false;
	}
	inline bool sign()   const noexcept { return _sign; }
	inline int scale()   const noexcept { return findMsb(); } // TODO: when value = 0, scale returns -1 which is incorrect

	inline BlockType block(unsigned b) const noexcept {
		if (b < _block.size()) {
			return _block[b];
		}
		return 0u;
	}
	inline unsigned limbs() const noexcept { return static_cast<unsigned>(_block.size()); }

	inline unsigned nbits() const noexcept { return static_cast<unsigned>(_block.size() * sizeof(BlockType) * 8); }

	// findMsb takes an adaptiveint reference and returns the position of the most significant bit, -1 if v == 0
	inline int findMsb() const noexcept {
		int nrBlocks = static_cast<int>(_block.size());
		if (nrBlocks == 0) return -1; // no significant bit found, all bits are zero
		int msb = nrBlocks * static_cast<int>(bitsInBlock);
		for (int b = nrBlocks - 1; b >= 0; --b) {
			std::uint32_t segment = _block[static_cast<size_t>(b)];
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

	// show the binary encodings of the limbs
	std::string showLimbs() const {
		if (_block.empty()) return "no limbs";
		std::stringstream s;
		size_t i = _block.size() - 1;
		while (i > 0) {
			s << to_binary(_block[i], sizeof(BlockType) * 8, true) << ' ';
			--i;
		}
		s << to_binary(_block[0], sizeof(BlockType) * 8, true);
		return s.str();
	}
	// show the values of the limbs as a radix-BlockType number
	std::string showLimbValues() const {
		if (_block.empty()) return "no limbs";
		std::stringstream s;
		size_t i = _block.size() - 1;
		while (i > 0) {
			s << std::setw(5) << unsigned(_block[i]) << ", ";
			--i;
		}
		s << std::setw(5) << unsigned(_block[0]);
		return s.str();
	}

protected:
	bool                   _sign;    // sign of the number: -1 if true, +1 if false, zero is positive
	std::vector<BlockType> _block;  // building blocks representing a 1's complement magnitude

	// HELPER methods
	inline int compare_magnitude(const adaptiveint& a, const adaptiveint& b) {
		unsigned aLimbs = a.limbs();
		unsigned bLimbs = b.limbs();
		if (aLimbs != bLimbs) {
			return (aLimbs > bLimbs ? 1 : -1);  // return 1 if a > b, otherwise -1
		}
		for (int i = aLimbs - 1; i >= 0; --i) {
			BlockType _a = a._block[static_cast<size_t>(i)];
			BlockType _b = b._block[static_cast<size_t>(i)];
			if ( _a != _b) {
				return (_a > _b ? 1 : -1);
			}
		}
		return 0;
	}
	inline void remove_leading_zeros() {
		unsigned leadingZeroBlocks{ 0 };
		typename std::vector<BlockType>::reverse_iterator rit = _block.rbegin();
		while (rit != _block.rend()) {
			if (*rit == 0) {
				++leadingZeroBlocks;
			}
			else {
				break;
			}
			++rit;
		}
		_block.resize(_block.size() - leadingZeroBlocks);
	}
	
	template<typename SignedInt>
	inline adaptiveint& assign_signed(SignedInt v) {
		clear();
		if (v != 0) {
			if (v < 0) {
				setbits(static_cast<unsigned long long>(-v));
				setsign(true);
			}
			else {
				setbits(static_cast<unsigned long long>(v)); // TODO: what about -2^63
			}
		}
		return *this;
	}

	template<typename UnsignedInt>
	inline adaptiveint& assign_unsigned(UnsignedInt v) {
		if (0 == v) {
			setzero();
		}
		else {
			setbits(static_cast<unsigned long long>(v));
		}
		return *this;
	}

	template<typename Real>
	adaptiveint& assign_native_ieee(Real& rhs) {
		clear();
		bool s{ false };
		std::uint64_t rawExponent{ 0 };
		std::uint64_t rawFraction{ 0 };
		// use native conversion
		extractFields(rhs, s, rawExponent, rawFraction);
		if (rawExponent == ieee754_parameter<Real>::eallset) { // nan and inf
			// we can't represent NaNs or Infinities
			return *this;
		}
		int exponent = static_cast<int>(rawExponent) - ieee754_parameter<Real>::bias;
		if (exponent < 0) {
			return *this; // we are zero
		}
		// normal and subnormal handling
		setsign(s);
		constexpr size_t fbits = ieee754_parameter<Real>::fbits;
		std::uint64_t hiddenBit = (0x1ull << fbits);
		rawFraction |= hiddenBit;
		setbits(rawFraction);
		// scale the fraction bits
		*this <<= static_cast<int>(exponent - fbits);
		return *this;
	}

	template<typename Integer>
	Integer convert_to_native_integer() const noexcept {
		Integer v{ 0 };
		Integer m{ 1 };
		for (unsigned i = 0; i < nbits(); ++i) {
			if (test(i)) {
				v += m;
			}
			m *= 2;
		}
		return (sign() ? -v : v);
	}
	template<typename Real>
	Real convert_to_native_ieee() const noexcept {
		Real v{ 0 };
		Real m{ 1.0 };
		for (unsigned i = 0; i < nbits(); ++i) {
			if (test(i)) {
				v += m;
			}
			m *= Real(2.0);
		}
		return (sign() ? -v : v);
	}

private:

	template<typename BBlockType>
	friend inline bool operator==(const adaptiveint<BBlockType>&, const adaptiveint<BBlockType>&);
};

////////////////////////    adaptiveint functions   /////////////////////////////////

template<typename BlockType>
inline adaptiveint<BlockType> abs(const adaptiveint<BlockType>& a) {
	return (a.isneg()  ? -a : a);
}

////////////////////////    INTEGER operators   /////////////////////////////////

/// stream operators

// read a adaptiveint ASCII format and make a binary adaptiveint out of it
template<typename BlockType>
bool parse(const std::string& number, adaptiveint<BlockType>& value) {
	bool bSuccess = false;

	return bSuccess;
}

template<typename BlockType>
std::string convert_to_string(std::ios_base::fmtflags flags, const adaptiveint<BlockType>& n) {
	using AdaptiveInteger = adaptiveint<BlockType>;

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
		AdaptiveInteger t(n);
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
		std::string::size_type fnz = result.find_first_not_of('0');
		if (!result.empty() && (fnz == std::string::npos)) fnz = result.size() - 1;
		result.erase(0, fnz);
		if (flags & std::ios_base::showbase) {
			const char* pp = base == 8 ? "0" : "0x";
			result.insert(static_cast<std::string::size_type>(0), pp);
		}
	}
	else {
		unsigned block10;
		unsigned digits_in_block10;
		if constexpr (AdaptiveInteger::bitsInBlock == 8) {
			block10 = 100;
			digits_in_block10 = 2;
		}
		else if constexpr (AdaptiveInteger::bitsInBlock == 16) {
			block10 = 10'000;
			digits_in_block10 = 4;
		}
		else if constexpr (AdaptiveInteger::bitsInBlock == 32) {
			block10 = 1'000'000'000;
			digits_in_block10 = 9;
		}
		else if constexpr (AdaptiveInteger::bitsInBlock == 64) {
			block10 = 1'000'000'000'000'000'000;
			digits_in_block10 = 18;
		}
		result.assign(nbits / 3 + 1u, '0');
		std::string::difference_type pos = static_cast<int>(result.size()) - 1;
		AdaptiveInteger t(n);
		while (!t.iszero()) {
			AdaptiveInteger q,r;
			q.reduce(t, block10, r);
			BlockType v = r.block(0);
//			std::cout << "v  " << uint32_t(v) << '\n';
			for (unsigned i = 0; i < digits_in_block10; ++i) {
				char c = '0' + static_cast<char>(v % 10);
				v /= 10;
				result[static_cast<size_t>(pos)] = c;
//				std::cout << result << " pos: " << pos << '\n';
				if (pos-- == 0)
					break;
			}
			t = q;
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
template<typename BlockType>
inline std::ostream& operator<<(std::ostream& ostr, const adaptiveint<BlockType>& i) {
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

template<typename BlockType>
inline std::istream& operator>>(std::istream& istr, adaptiveint<BlockType>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators

template<typename BlockType>
inline std::string to_binary(const adaptiveint<BlockType>& a, bool nibbleMarker = true) {
	if (a.limbs() == 0) return std::string("0x0");

	std::stringstream s;
	s << "0x";
	for (int b = static_cast<int>(a.limbs()) - 1; b >= 0; --b) {
		BlockType segment = a.block(static_cast<size_t>(b));
		BlockType mask = (0x1u << (a.bitsInBlock - 1));
		for (int i = a.bitsInBlock - 1; i >= 0; --i) {
			s << ((segment & mask) ? '1' : '0');
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
			if (b > 0 && i == 0 && nibbleMarker) s << '\'';
			mask >>= 1;
		}
	}

	return s.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// adaptiveint - adaptiveint binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths

template<typename BlockType>
inline bool operator==(const adaptiveint<BlockType>& lhs, const adaptiveint<BlockType>& rhs) {
	if (lhs.limbs() != rhs.limbs()) {
		return false;
	}
	typename std::vector<BlockType>::const_iterator li = lhs._block.begin();
	typename std::vector<BlockType>::const_iterator ri = rhs._block.begin();
	while (li != lhs._block.end()) {
		if (*li != *ri) return false;
		++li; ++ri;
	}
	return true;
}

template<typename BlockType>
inline bool operator!=(const adaptiveint<BlockType>& lhs, const adaptiveint<BlockType>& rhs) {
	return !operator==(lhs, rhs);
}

template<typename BlockType>
inline bool operator< (const adaptiveint<BlockType>& lhs, const adaptiveint<BlockType>& rhs) {
	unsigned ll = lhs.limbs();
	unsigned rl = rhs.limbs();
	if (ll < rl) return true;
	if (ll > rl) return false;
	for (unsigned b = ll - 1; b > 0; --b) {
		BlockType l = lhs.block(b);
		BlockType r = rhs.block(b);
		if (l < r) return true;
		else if (l == r) continue;
		else return false;
	}
	BlockType l = lhs.block(0);
	BlockType r = rhs.block(0);
	if (l < r) return true;
	return false; // lhs and rhs are the same
}

template<typename BlockType>
inline bool operator> (const adaptiveint<BlockType>& lhs, const adaptiveint<BlockType>& rhs) {
	return operator< (rhs, lhs);
}

template<typename BlockType>
inline bool operator<=(const adaptiveint<BlockType>& lhs, const adaptiveint<BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<typename BlockType>
inline bool operator>=(const adaptiveint<BlockType>& lhs, const adaptiveint<BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// adaptiveint - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths

template<typename BlockType>
inline bool operator==(const adaptiveint<BlockType>& lhs, long long rhs) {
	return operator==(lhs, adaptiveint(rhs));
}

template<typename BlockType>
inline bool operator!=(const adaptiveint<BlockType>& lhs, long long rhs) {
	return !operator==(lhs, rhs);
}

template<typename BlockType>
inline bool operator< (const adaptiveint<BlockType>& lhs, long long rhs) {
	return operator<(lhs, adaptiveint<BlockType>(rhs));
}

template<typename BlockType>
inline bool operator> (const adaptiveint<BlockType>& lhs, long long rhs) {
	return operator< (adaptiveint<BlockType>(rhs), lhs);
}

template<typename BlockType>
inline bool operator<=(const adaptiveint<BlockType>& lhs, long long rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<typename BlockType>
inline bool operator>=(const adaptiveint<BlockType>& lhs, long long rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - adaptiveint binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths


template<typename BlockType>
inline bool operator==(long long lhs, const adaptiveint<BlockType>& rhs) {
	return operator==(adaptiveint<BlockType>(lhs), rhs);
}

template<typename BlockType>
inline bool operator!=(long long lhs, const adaptiveint<BlockType>& rhs) {
	return !operator==(lhs, rhs);
}

template<typename BlockType>
inline bool operator< (long long lhs, const adaptiveint<BlockType>& rhs) {
	return operator<(adaptiveint<BlockType>(lhs), rhs);
}

template<typename BlockType>
inline bool operator> (long long lhs, const adaptiveint<BlockType>& rhs) {
	return operator< (rhs, lhs);
}

template<typename BlockType>
inline bool operator<=(long long lhs, const adaptiveint<BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<typename BlockType>
inline bool operator>=(long long lhs, const adaptiveint<BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// adaptiveint - adaptiveint binary arithmetic operators

template<typename BlockType>
inline adaptiveint<BlockType> operator+(const adaptiveint<BlockType>& lhs, const adaptiveint<BlockType>& rhs) {
	adaptiveint sum = lhs;
	sum += rhs;
	return sum;
}

template<typename BlockType>
inline adaptiveint<BlockType> operator-(const adaptiveint<BlockType>& lhs, const adaptiveint<BlockType>& rhs) {
	adaptiveint diff = lhs;
	diff -= rhs;
	return diff;
}

template<typename BlockType>
inline adaptiveint<BlockType> operator*(const adaptiveint<BlockType>& lhs, const adaptiveint<BlockType>& rhs) {
	adaptiveint product = lhs;
	product *= rhs;
	return product;
}

template<typename BlockType>
inline adaptiveint<BlockType> operator/(const adaptiveint<BlockType>& lhs, const adaptiveint<BlockType>& rhs) {
	adaptiveint ratio = lhs;
	ratio /= rhs;
	return ratio;
}

template<typename BlockType>
inline adaptiveint<BlockType> operator%(const adaptiveint<BlockType>& lhs, const adaptiveint<BlockType>& rhs) {
	adaptiveint remainder = lhs;
	remainder %= rhs;
	return remainder;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// adaptiveint - literal binary arithmetic operators

template<typename BlockType>
inline adaptiveint<BlockType> operator+(const adaptiveint<BlockType>& lhs, long long rhs) {
	return operator+(lhs, adaptiveint<BlockType>(rhs));
}

template<typename BlockType>
inline adaptiveint<BlockType> operator-(const adaptiveint<BlockType>& lhs, long long rhs) {
	return operator-(lhs, adaptiveint<BlockType>(rhs));
}

template<typename BlockType>
inline adaptiveint<BlockType> operator*(const adaptiveint<BlockType>& lhs, long long rhs) {
	return operator*(lhs, adaptiveint<BlockType>(rhs));
}

template<typename BlockType>
inline adaptiveint<BlockType> operator/(const adaptiveint<BlockType>& lhs, long long rhs) {
	return operator/(lhs, adaptiveint<BlockType>(rhs));
}

template<typename BlockType>
inline adaptiveint<BlockType> operator%(const adaptiveint<BlockType>& lhs, long long rhs) {
	return operator/(lhs, adaptiveint<BlockType>(rhs));
}

template<typename BlockType>
inline adaptiveint<BlockType> operator/(const adaptiveint<BlockType>& lhs, unsigned long long rhs) {
	return operator/(lhs, adaptiveint<BlockType>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - adaptiveint binary arithmetic operators

template<typename BlockType>
inline adaptiveint<BlockType> operator+(long long lhs, const adaptiveint<BlockType>& rhs) {
	return operator+(adaptiveint<BlockType>(lhs), rhs);
}

template<typename BlockType>
inline adaptiveint<BlockType> operator-(long long lhs, const adaptiveint<BlockType>& rhs) {
	return operator-(adaptiveint<BlockType>(lhs), rhs);
}

template<typename BlockType>
inline adaptiveint<BlockType> operator*(long long lhs, const adaptiveint<BlockType>& rhs) {
	return operator*(adaptiveint<BlockType>(lhs), rhs);
}

template<typename BlockType>
inline adaptiveint<BlockType> operator/(long long lhs, const adaptiveint<BlockType>& rhs) {
	return operator/(adaptiveint<BlockType>(lhs), rhs);
}

template<typename BlockType>
inline adaptiveint<BlockType> operator%(long long lhs, const adaptiveint<BlockType>& rhs) {
	return operator/(adaptiveint<BlockType>(lhs), rhs);
}

}} // namespace sw::universal
