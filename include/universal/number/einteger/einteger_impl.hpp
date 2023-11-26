#pragma once
// einteger_impl.hpp: implementation of an adaptive precision binary integer
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>
#include <map>
#include <vector>

#include <universal/number/einteger/exceptions.hpp>
#include <universal/number/einteger/einteger_fwd.hpp>

// supporting types and functions
#include <universal/native/ieee754.hpp>

namespace sw { namespace universal {

// einteger is an adaptive precision integer type
template<typename BlockType = std::uint32_t>
class einteger {
public:
	using bt = BlockType;
	static constexpr unsigned bitsInBlock = sizeof(BlockType) * 8;
	static constexpr bt       ALL_ONES = bt(0xFFFF'FFFF'FFFF'FFFFull); // block type specific all 1's value
	static constexpr uint64_t BASE = uint64_t(ALL_ONES) + 1ull;
	static_assert(bitsInBlock <= 32, "BlockType must be one of [uint8_t, uint16_t, uint32_t]");

	einteger() : _sign(false), _block{} { }

	einteger(const einteger&) = default;
	einteger(einteger&&) = default;

	einteger& operator=(const einteger&) = default;
	einteger& operator=(einteger&&) = default;

	// initializers for native types
	einteger(short initial_value)              { *this = initial_value; }
	einteger(int initial_value)                { *this = initial_value; }
	einteger(long initial_value)               { *this = initial_value; }
	einteger(long long initial_value)          { *this = initial_value; }
	einteger(unsigned int initial_value)       { *this = initial_value; }
	einteger(unsigned long initial_value)      { *this = initial_value; }
	einteger(unsigned long long initial_value) { *this = initial_value; }
	einteger(float initial_value)              { *this = initial_value; }
	einteger(double initial_value)             { *this = initial_value; }

	// assignment operators for native types
	einteger& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	einteger& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	einteger& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	einteger& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	einteger& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	einteger& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	einteger& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	einteger& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

	// conversion operators
	explicit operator int() const noexcept         { return convert_to_native_integer<int>(); }
	explicit operator long() const noexcept        { return convert_to_native_integer<long>(); }
	explicit operator long long() const noexcept   { return convert_to_native_integer<long long>(); }
	explicit operator float() const noexcept       { return convert_to_native_ieee<float>(); }
	explicit operator double() const noexcept      { return convert_to_native_ieee<double>(); }

#if LONG_DOUBLE_SUPPORT
	einteger(long double initial_value) { *this = initial_value; }
	einteger& operator=(long double rhs)        noexcept { return convert_ieee754(rhs); }
	explicit operator long double() const noexcept { return convert_to_native_ieee<long double>(); }
#endif

	// logic shift operators
	einteger& operator<<=(int shift) {
		if (shift == 0) return *this;
		if (shift < 0) return operator>>=(-shift);

		// by default extend the limbs by 1: TODO: can this be improved?
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
			_block[0] <<= shift;
		}
		else {
			_block[0] <<= shift;
		}
		remove_leading_zeros();
		return *this;
	}
	einteger& operator>>=(int shift) {
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
	
	// unary arithmetic operators
	einteger operator-() const {
		einteger negated(*this);
		negated.setsign(!_sign);
		return negated;
	}
	einteger operator++(int) {
		einteger tmp(*this);
		operator++();
		return tmp;
	}
	einteger& operator++() {
		*this += 1;
		return *this;
	}
	einteger operator--(int) {
		einteger tmp(*this);
		operator--();
		return tmp;
	}
	einteger& operator--() {
		*this -= 1;
		return *this;
	}

	// binary arithmetic operators
	einteger& operator+=(const einteger& rhs) {
		if (sign() != rhs.sign()) {
			if (sign()) {
				einteger negated(*this);
				negated.setsign(false);
				*this = rhs - negated;
				return *this;
			}
			else {
				einteger negated(rhs);
				negated.setsign(false);
				*this -= negated;
				return *this;
			}
		}
		auto lhsSize = _block.size();
		auto rhsSize = rhs._block.size();
		if (lhsSize < rhsSize) _block.resize(rhsSize, 0);

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
	einteger& operator+=(long long rhs) {
		return *this += einteger(rhs);
	}
	einteger& operator-=(const einteger& rhs) {
		if (rhs.sign()) {
			einteger negated(rhs);
			negated.setsign(false);
			return *this += negated;
		}
		auto lhsSize = _block.size();
		if (lhsSize == 0) {
			*this = -rhs;
			return *this;
		}
		auto rhsSize = rhs._block.size();
		int magnitude = compare_magnitude(*this, rhs); // if -1 result is going to be negative

		auto overlap = (lhsSize < rhsSize ? lhsSize : rhsSize);
		auto extent = (lhsSize < rhsSize ? rhsSize : lhsSize);

		// prep storage
		if (lhsSize < rhsSize) _block.resize(rhsSize, 0);

		typename std::vector<BlockType>::const_iterator aIter, bIter;
		if (magnitude == 1) {
			aIter = _block.begin();
			bIter = rhs._block.begin();
		}
		else {
			aIter = rhs._block.begin();
			bIter = _block.begin();
		}
		std::uint64_t borrow{ 0 };
		unsigned i{ 0 };
		while (i < overlap) {
			borrow = static_cast<std::uint64_t>(*aIter) - static_cast<std::uint64_t>(*bIter) - borrow;
			_block[i] = static_cast<BlockType>(borrow);
			borrow = (borrow >> bitsInBlock) & 0x1u;
			++i; ++aIter; ++bIter;
		}
		while ((i < extent)) {
			borrow = static_cast<BlockType>(*aIter) - borrow;
			_block[i] = static_cast<BlockType>(borrow);
			borrow = (borrow >> bitsInBlock) & 0x1u;
			++i; ++aIter;
		}
		remove_leading_zeros();
		setsign(magnitude == -1);
		return *this;
	}
	einteger& operator-=(long long rhs) {
		return *this -= einteger(rhs);
	}
	einteger& operator*=(const einteger& rhs) {
		if (iszero() || rhs.iszero()) {
			clear();
			return *this;
		}
		einteger base(*this);
		bool ls = sign();
		unsigned ll = limbs();
		bool rs = rhs.sign();
		unsigned rl = rhs.limbs();

		clear();
		std::uint64_t segment(0);
		for (unsigned i = 0; i < ll; ++i) {
			for (unsigned j = 0; j < rl; ++j) {
				segment += static_cast<std::uint64_t>(base.block(i)) * static_cast<std::uint64_t>(rhs.block(j));
				segment += block(i + j);
				setblock(i + j, static_cast<bt>(segment));
				segment >>= bitsInBlock;
			}
		}
		if (segment != 0) setblock(ll + rl - 1, static_cast<bt>(segment));
		setsign(ls ^ rs);
		return *this;
	}
	einteger& operator*=(long long rhs) {
		return *this *= einteger(rhs);
	}
	einteger& operator/=(const einteger& rhs) {
		einteger q, r;
		q.reduce(*this, rhs, r);
		*this = q;
		return *this;
	}
	einteger& operator/=(long long rhs) {
		return *this /= einteger(rhs);
	}
	einteger& operator%=(const einteger& rhs) {
		einteger q, a(*this), r;
		q.reduce(a, rhs, r);
		*this = r;
		return *this;
	}
	einteger& operator%=(long long rhs) {
		return *this %= einteger(rhs);
	}
	
	// reduce returns the ratio and remainder of a and b in *this and r
	void reduce(const einteger& a, const einteger& b, einteger& r) {
		if (b.iszero()) {
#if EINTEGER_THROW_ARITHMETIC_EXCEPTION
			throw einteger_divide_by_zero{};
#else
			std::cerr << "einteger_divide_by_zero\n";
			return;
#endif // EINTEGER_THROW_ARITHMETIC_EXCEPTION
		}
		clear();
		r.clear();
		if (a.iszero()) return;

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
			einteger normalized_a;
			normalized_a.setblock(m, static_cast<BlockType>((a.block(m - 1) >> (bitsInBlock - shift))));
			for (unsigned i = m - 1; i > 0; --i) {
				normalized_a.setblock(i, static_cast<BlockType>((a.block(i) << shift) | (a.block(i - 1) >> (bitsInBlock - shift))));
			}
			normalized_a.setblock(0, static_cast<BlockType>(a.block(0) << shift));
			// normalize b
			einteger normalized_b;
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
		_sign = a.sign() ^ b.sign();
	}

	// modifiers
	void clear() noexcept { _sign = false; _block.clear(); }
	void setzero() noexcept { clear(); }
	void setsign(bool sign = true) noexcept { _sign = sign; }
	// use un-interpreted raw bits to set the bits of the einteger
	void setbits(unsigned long long value) {
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
	void setblock(unsigned i, BlockType value) noexcept {
		if (i >= _block.size()) _block.resize(i+1ull);
		_block[i] = value;
	}
	void setbyte(unsigned i, std::uint8_t byte) noexcept {
		std::cerr << "setbyte(" << i << ", " << int(byte) << ") TBD\n";
	}
	einteger& assign(const std::string& txt) {
		if (!parse(txt, *this)) {
			std::cerr << "Unable to parse: " << txt << std::endl;
		}
		return *this;
	}

	// selectors
	bool iszero() const noexcept { return (_block.size() == 0 || ((_block.size() == 1) && _block[0] == bt(0))); }
	bool isone()  const noexcept { return true; }
	bool isodd()  const noexcept { return (_block.size() > 0) ? (_block[0] & 0x1) : false; }
	bool iseven() const noexcept { return !isodd(); }
	bool ispos()  const noexcept { return !_sign; }
	bool isneg()  const noexcept { return _sign; }

	bool test(unsigned index) const noexcept {
		if (index < nbits()) {
			unsigned blockIndex = index / bitsInBlock;
			unsigned bitIndexInBlock = index % bitsInBlock;
			BlockType data = _block[blockIndex];
			BlockType mask = (0x1u << bitIndexInBlock);
			if (data & mask) return true;
		}
		return false;
	}
	bool sign()   const noexcept { return _sign; }
	int scale()   const noexcept { return findMsb(); } // TODO: when value = 0, scale returns -1 which is incorrect

	BlockType block(unsigned b) const noexcept {
		if (b < _block.size()) return _block[b];
		return static_cast<BlockType>(0u);
	}
	unsigned limbs() const noexcept { return static_cast<unsigned>(_block.size()); }

	unsigned nbits() const noexcept { return static_cast<unsigned>(_block.size() * sizeof(BlockType) * 8); }

	// findMsb takes an einteger reference and returns the position of the most significant bit, -1 if v == 0
	int findMsb() const noexcept {
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
	bool                   _sign;   // sign of the number: -1 if true, +1 if false, zero is positive
	std::vector<BlockType> _block;  // building blocks representing a 1's complement magnitude

	// HELPER methods
	// compare_magnitude returns 1 if a > b, 0 if they are equal, and -1 if a < b
	int compare_magnitude(const einteger& a, const einteger& b) {
		unsigned aLimbs = a.limbs();
		unsigned bLimbs = b.limbs();
		if (aLimbs != bLimbs) {
			return (aLimbs > bLimbs ? 1 : -1);  // return 1 if a > b, otherwise -1
		}
		for (int i = static_cast<int>(aLimbs) - 1; i >= 0; --i) {
			BlockType _a = a._block[static_cast<size_t>(i)];
			BlockType _b = b._block[static_cast<size_t>(i)];
			if ( _a != _b) {
				return (_a > _b ? 1 : -1);
			}
		}
		return 0;
	}
	void remove_leading_zeros() {
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
	einteger& convert_signed(SignedInt v) {
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
	einteger& convert_unsigned(UnsignedInt v) {
		if (0 == v) {
			setzero();
		}
		else {
			setbits(static_cast<unsigned long long>(v));
		}
		return *this;
	}

	template<typename Real>
	einteger& convert_ieee754(Real& rhs) {
		clear();
		bool s{ false };
		std::uint64_t rawExponent{ 0 };
		std::uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(rhs, s, rawExponent, rawFraction, bits);
		if (rawExponent == ieee754_parameter<Real>::eallset) { // nan and inf
			// we can't represent NaNs or Infinities
			return *this;
		}
		int exponent = static_cast<int>(rawExponent) - ieee754_parameter<Real>::bias;
		if (exponent < 0) {
			return *this; // we are zero
		}
		// normal and subnormal handling
		constexpr size_t fbits = ieee754_parameter<Real>::fbits;
		std::uint64_t hiddenBit = (0x1ull << fbits);
		rawFraction |= hiddenBit;
		setbits(rawFraction);
		setsign(s);
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
	friend inline bool operator==(const einteger<BBlockType>&, const einteger<BBlockType>&);
};

////////////////////////    einteger functions   /////////////////////////////////

template<typename BlockType>
inline einteger<BlockType> abs(const einteger<BlockType>& a) {
	return (a.isneg()  ? -a : a);
}

////////////////////////    INTEGER operators   /////////////////////////////////

/// stream operators

// read a einteger ASCII format and make a binary einteger out of it
template<typename BlockType>
bool parse(const std::string& number, einteger<BlockType>& value) {
	using Integer = einteger<BlockType>;
	bool bSuccess = false;
	value.clear();
	std::regex binary_regex("^[-+]*0b[01']+");
	// check if the txt is an integer form: [0123456789]+
	std::regex decimal_regex("^[-+]*[0-9]+");
	std::regex octal_regex("^[-+]*0[1-7][0-7]*$");
	std::regex hex_regex("^[-+]*0[xX][0-9a-fA-F']+");
	// setup associative array to map chars to nibbles
	std::map<char, int> charLookup{
		{ '0', 0 },
		{ '1', 1 },
		{ '2', 2 },
		{ '3', 3 },
		{ '4', 4 },
		{ '5', 5 },
		{ '6', 6 },
		{ '7', 7 },
		{ '8', 8 },
		{ '9', 9 },
		{ 'a', 10 },
		{ 'b', 11 },
		{ 'c', 12 },
		{ 'd', 13 },
		{ 'e', 14 },
		{ 'f', 15 },
		{ 'A', 10 },
		{ 'B', 11 },
		{ 'C', 12 },
		{ 'D', 13 },
		{ 'E', 14 },
		{ 'F', 15 },
	};
	if (std::regex_match(number, octal_regex)) {
		std::cout << "found an octal representation\n";
		for (std::string::const_reverse_iterator r = number.rbegin();
			r != number.rend();
			++r) {
			std::cout << "char = " << *r << std::endl;
		}
		bSuccess = false; // TODO
	}
	else if (std::regex_match(number, hex_regex)) {
		//std::cout << "found a hexadecimal representation\n";
		// each char is a nibble
		int byte = 0;
		int byteIndex = 0;
		bool odd = false;
		for (std::string::const_reverse_iterator r = number.rbegin();
			r != number.rend();
			++r) {
			if (*r == '\'') {
				// ignore
			}
			else if (*r == 'x' || *r == 'X') {
				if (odd) {
					// complete the most significant byte
					value.setbyte(static_cast<size_t>(byteIndex), static_cast<uint8_t>(byte));
				}
				// check that we have [-+]0[xX] format
				++r;
				if (r != number.rend()) {
					if (*r == '0') {
						// check if we have a sign
						++r;
						if (r == number.rend()) {
							// no sign, thus by definition positive
							bSuccess = true;
						}
						else if (*r == '+') {
							// optional positive sign, no further action necessary
							bSuccess = true;
						}
						else if (*r == '-') {
							// negative sign, invert
							value = -value;
							bSuccess = true;
						}
						else {
							// the regex will have filtered this out
							bSuccess = false;
						}
					}
					else {
						// we didn't find the obligatory '0', the regex should have filtered this out
						bSuccess = false;
					}
				}
				else {
					// we are missing the obligatory '0', the regex should have filtered this out
					bSuccess = false;
				}
				// we have reached the end of our parse
				break;
			}
			else {
				if (odd) {
					byte += charLookup.at(*r) << 4;
					value.setbyte(static_cast<size_t>(byteIndex), static_cast<uint8_t>(byte));
					++byteIndex;
				}
				else {
					byte = charLookup.at(*r);
				}
				odd = !odd;
			}
		}
	}
	else if (std::regex_match(number, decimal_regex)) {
		//std::cout << "found a decimal integer representation\n";
		Integer scale = 1;
		bool sign{ false };
		for (std::string::const_reverse_iterator r = number.rbegin();
			r != number.rend();
			++r) {
			if (*r == '-') {
				sign = true;;
			}
			else if (*r == '+') {
				break;
			}
			else {
				Integer digit = charLookup.at(*r);
				value += scale * digit;
				scale *= 10;
			}
		}
		value.setsign(sign);
		bSuccess = true;
	}
	else if (std::regex_match(number, binary_regex)) {
		//std::cout << "found a binary integer representation\n";
		Integer scale = 1;
		//bool sign{ false };
		unsigned byte{ 0 }; // using an unsigned to simplify accumulation, but accumulating 8-bit byte values
		unsigned bitIndex = 0;
		for (std::string::const_reverse_iterator r = number.rbegin();
			r != number.rend();
			++r) {
			if (*r == '-') {
				//sign = true;;
			}
			else if (*r == '+') {
				break;
			}
			else if (*r == '\'') {
				// ignore separator
			}
			else {
				if (*r == '1') {
					byte |= (1u << (bitIndex % 8));
				}
				if (bitIndex == 7) {
					value += scale * byte;
					scale *= 256;
					byte = 0;
				}
				++bitIndex;
			}
		}
		if (bitIndex % 8) {
			value += scale * byte;
		}
		bSuccess = true;
	}
	return bSuccess;
}

template<typename BlockType>
std::string convert_to_string(std::ios_base::fmtflags flags, const einteger<BlockType>& n) {
	using AdaptiveInteger = einteger<BlockType>;

	if (n.limbs() == 0) return std::string("0");

	// set the base of the target number system to convert to
	int base = 10;
	if ((flags & std::ios_base::oct) == std::ios_base::oct) base = 8;
	if ((flags & std::ios_base::hex) == std::ios_base::hex) base = 16;

	unsigned nbits = n.limbs() * sizeof(BlockType) * 8;

	std::string result;
	if (base == 8 || base == 16) {
		if (n.sign()) return std::string("negative value: ignored");

		size_t shift = (base == 8 ? 3ull : 4ull);
		BlockType mask = static_cast<BlockType>((1u << shift) - 1);
		AdaptiveInteger t(n);
		result.assign(nbits / shift + ((nbits % shift) ? 1 : 0), '0');
		size_t pos = result.size() - 1ull;
		for (size_t i = 0; i < nbits / shift; ++i) {
			char c = '0' + static_cast<char>(t.block(0) & mask);
			if (c > '9')
				c += 'A' - '9' - 1;
			result[pos--] = c;
			t >>= static_cast<int>(shift);
		}
		if (nbits % shift) {
			mask = static_cast<BlockType>((1u << (nbits % shift)) - 1);
			char c = '0' + static_cast<char>(t.block(0) & mask);
			if (c > '9')
				c += 'A' - '9';
			result[pos] = c;
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
			block10 = 100u;
			digits_in_block10 = 2;
		}
		else if constexpr (AdaptiveInteger::bitsInBlock == 16) {
			block10 = 10'000ul;
			digits_in_block10 = 4;
		}
		else if constexpr (AdaptiveInteger::bitsInBlock == 32) {
			block10 = 1'000'000'000ul;
			digits_in_block10 = 9;
		}
		else if constexpr (AdaptiveInteger::bitsInBlock == 64) {
			// not allowed as the whole multi-digit arithmetic
			// requires that there is a 'larger' type that
			// can receive carries and borrows.
			// If your platform does have a native 128bit
			// integer, this could be enabled
			//block10 = 1'000'000'000'000'000'000ull;
			//digits_in_block10 = 18;
		}
		result.assign(nbits / 3 + 1ull, '0');
		size_t pos = result.size() - 1ull;
		AdaptiveInteger t(n);
		while (!t.iszero()) {
			AdaptiveInteger q,r;
			q.reduce(t, block10, r);
			BlockType v = r.block(0);
//			std::cout << "v  " << uint32_t(v) << '\n';
			for (unsigned i = 0; i < digits_in_block10; ++i) {
				char c = '0' + static_cast<char>(v % 10);
				v /= 10;
				result[pos] = c;
//				std::cout << result << " pos: " << pos << '\n';
				if (pos-- == 0)	break;
			}
			t = q;
		}

		std::string::size_type firstDigit = result.find_first_not_of('0');
		result.erase(0, firstDigit);
		if (result.empty())
			result = "0";
		if (n.isneg())
			result.insert(0ull, 1ull, '-');
		else if (flags & std::ios_base::showpos)
			result.insert(0ull, 1ull, '+');
	}
	return result;
}

// generate an einteger format ASCII format
template<typename BlockType>
inline std::ostream& operator<<(std::ostream& ostr, const einteger<BlockType>& i) {
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

// read an ASCII einteger format

template<typename BlockType>
inline std::istream& operator>>(std::istream& istr, einteger<BlockType>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

////////////////// string operators

template<typename BlockType>
inline std::string to_binary(const einteger<BlockType>& a, bool nibbleMarker = true) {
	if (a.limbs() == 0) return std::string("0b0");

	std::stringstream s;
	s << "0b";
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

template<typename BlockType>
inline std::string to_hex(const einteger<BlockType>& a, bool wordMarker = true) {
	if (a.limbs() == 0) return std::string("0x0");

	std::vector<char> nibbleLookup = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	std::stringstream s;
	s << "0x";
	unsigned bitIndex = a.limbs() * a.bitsInBlock - 1u;
	for (int b = static_cast<int>(a.limbs()) - 1; b >= 0; --b) {
		BlockType limb = a.block(static_cast<size_t>(b));
		BlockType mask = (0x1u << (a.bitsInBlock - 1));
		unsigned nibble{ 0 };
		unsigned rightShift = a.bitsInBlock - 4u;
		for (int i = a.bitsInBlock - 1; i >= 0; --i) {

			nibble |= (limb & mask);
			if ((i % 4) == 0) {
				nibble >>= rightShift;
				s << nibbleLookup[nibble];
				nibble = 0;
				rightShift -= 4u;
			}
			if (bitIndex > 0 && ((bitIndex % 16) == 0) && wordMarker) s << '\'';
			mask >>= 1;
			--bitIndex;
		}
	}

	return s.str();

}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// einteger - einteger binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths

template<typename BlockType>
inline bool operator==(const einteger<BlockType>& lhs, const einteger<BlockType>& rhs) {
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
inline bool operator!=(const einteger<BlockType>& lhs, const einteger<BlockType>& rhs) {
	return !operator==(lhs, rhs);
}

template<typename BlockType>
inline bool operator< (const einteger<BlockType>& lhs, const einteger<BlockType>& rhs) {
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
inline bool operator> (const einteger<BlockType>& lhs, const einteger<BlockType>& rhs) {
	return operator< (rhs, lhs);
}

template<typename BlockType>
inline bool operator<=(const einteger<BlockType>& lhs, const einteger<BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<typename BlockType>
inline bool operator>=(const einteger<BlockType>& lhs, const einteger<BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// einteger - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths

template<typename BlockType>
inline bool operator==(const einteger<BlockType>& lhs, long long rhs) {
	return operator==(lhs, einteger(rhs));
}

template<typename BlockType>
inline bool operator!=(const einteger<BlockType>& lhs, long long rhs) {
	return !operator==(lhs, rhs);
}

template<typename BlockType>
inline bool operator< (const einteger<BlockType>& lhs, long long rhs) {
	return operator<(lhs, einteger<BlockType>(rhs));
}

template<typename BlockType>
inline bool operator> (const einteger<BlockType>& lhs, long long rhs) {
	return operator< (einteger<BlockType>(rhs), lhs);
}

template<typename BlockType>
inline bool operator<=(const einteger<BlockType>& lhs, long long rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<typename BlockType>
inline bool operator>=(const einteger<BlockType>& lhs, long long rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - einteger binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths


template<typename BlockType>
inline bool operator==(long long lhs, const einteger<BlockType>& rhs) {
	return operator==(einteger<BlockType>(lhs), rhs);
}

template<typename BlockType>
inline bool operator!=(long long lhs, const einteger<BlockType>& rhs) {
	return !operator==(lhs, rhs);
}

template<typename BlockType>
inline bool operator< (long long lhs, const einteger<BlockType>& rhs) {
	return operator<(einteger<BlockType>(lhs), rhs);
}

template<typename BlockType>
inline bool operator> (long long lhs, const einteger<BlockType>& rhs) {
	return operator< (rhs, lhs);
}

template<typename BlockType>
inline bool operator<=(long long lhs, const einteger<BlockType>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

template<typename BlockType>
inline bool operator>=(long long lhs, const einteger<BlockType>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// einteger - einteger binary arithmetic operators

template<typename BlockType>
inline einteger<BlockType> operator+(const einteger<BlockType>& lhs, const einteger<BlockType>& rhs) {
	einteger sum = lhs;
	sum += rhs;
	return sum;
}

template<typename BlockType>
inline einteger<BlockType> operator-(const einteger<BlockType>& lhs, const einteger<BlockType>& rhs) {
	einteger diff = lhs;
	diff -= rhs;
	return diff;
}

template<typename BlockType>
inline einteger<BlockType> operator*(const einteger<BlockType>& lhs, const einteger<BlockType>& rhs) {
	einteger product = lhs;
	product *= rhs;
	return product;
}

template<typename BlockType>
inline einteger<BlockType> operator/(const einteger<BlockType>& lhs, const einteger<BlockType>& rhs) {
	einteger ratio = lhs;
	ratio /= rhs;
	return ratio;
}

template<typename BlockType>
inline einteger<BlockType> operator%(const einteger<BlockType>& lhs, const einteger<BlockType>& rhs) {
	einteger remainder = lhs;
	remainder %= rhs;
	return remainder;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// einteger - literal binary arithmetic operators

template<typename BlockType>
inline einteger<BlockType> operator+(const einteger<BlockType>& lhs, long long rhs) {
	return operator+(lhs, einteger<BlockType>(rhs));
}

template<typename BlockType>
inline einteger<BlockType> operator-(const einteger<BlockType>& lhs, long long rhs) {
	return operator-(lhs, einteger<BlockType>(rhs));
}

template<typename BlockType>
inline einteger<BlockType> operator*(const einteger<BlockType>& lhs, long long rhs) {
	return operator*(lhs, einteger<BlockType>(rhs));
}

template<typename BlockType>
inline einteger<BlockType> operator/(const einteger<BlockType>& lhs, long long rhs) {
	return operator/(lhs, einteger<BlockType>(rhs));
}

template<typename BlockType>
inline einteger<BlockType> operator%(const einteger<BlockType>& lhs, long long rhs) {
	return operator/(lhs, einteger<BlockType>(rhs));
}

template<typename BlockType>
inline einteger<BlockType> operator/(const einteger<BlockType>& lhs, unsigned long long rhs) {
	return operator/(lhs, einteger<BlockType>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - einteger binary arithmetic operators

template<typename BlockType>
inline einteger<BlockType> operator+(long long lhs, const einteger<BlockType>& rhs) {
	return operator+(einteger<BlockType>(lhs), rhs);
}

template<typename BlockType>
inline einteger<BlockType> operator-(long long lhs, const einteger<BlockType>& rhs) {
	return operator-(einteger<BlockType>(lhs), rhs);
}

template<typename BlockType>
inline einteger<BlockType> operator*(long long lhs, const einteger<BlockType>& rhs) {
	return operator*(einteger<BlockType>(lhs), rhs);
}

template<typename BlockType>
inline einteger<BlockType> operator/(long long lhs, const einteger<BlockType>& rhs) {
	return operator/(einteger<BlockType>(lhs), rhs);
}

template<typename BlockType>
inline einteger<BlockType> operator%(long long lhs, const einteger<BlockType>& rhs) {
	return operator/(einteger<BlockType>(lhs), rhs);
}

}} // namespace sw::universal
