#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include <universal/number/poxel/core.hpp>
#include <universal/number/valid/exceptions.hpp>
#include <universal/number/valid/valid_fwd.hpp>

#if !defined(VALID_THROW_ARITHMETIC_EXCEPTION)
#	define VALID_THROW_ARITHMETIC_EXCEPTION 0
#endif

namespace sw {
namespace universal {

enum class valid_sign {
	negative,
	zero,
	positive,
	undecidable
};

// A valid is a contiguous run of poxels. Each exact poxel includes its
// endpoint; each inexact poxel excludes its outside endpoint.
template<unsigned _nbits, unsigned _es, typename bt>
class valid {
  public:
	static constexpr unsigned nbits = _nbits;
	static constexpr unsigned es    = _es;

	using block_type = bt;
	using poxel_type = poxel<nbits, es, bt>;
	using posit_type = posit<nbits - 1, es, bt>;
	using key_type   = typename poxel_type::index_type;

	static constexpr key_type kmax = []() {
		if constexpr (nbits <= 64)
			return static_cast<key_type>((std::uint64_t(1) << (nbits - 1)) - 1u);
		else
			return (key_type(1) << (nbits - 1)) - key_type(1);
	}();

	valid() noexcept                        = default;
	valid(const valid&) noexcept            = default;
	valid(valid&&) noexcept                 = default;
	valid& operator=(const valid&) noexcept = default;
	valid& operator=(valid&&) noexcept      = default;

	// Enclose one poxel, preserving whether its bounds are open.
	// Example: valid<8, 1> v(poxel<8, 1>(0.1));
	valid(const poxel_type& pxl) noexcept : _loPxl(pxl), _hiPxl(pxl) {}
	valid(const posit_type& point) noexcept : valid(poxel_type(point)) {}

	// Span two poxels; reversed or NaR bounds produce NaR.
	// Example: valid<8, 1> v(poxel<8, 1>(1), poxel<8, 1>(2));
	valid(const poxel_type& lower, const poxel_type& upper) noexcept { assign(lower, upper); }
	valid(const posit_type& lower, const posit_type& upper, bool lowerOpen = false, bool upperOpen = false) noexcept {
		setinterval(lower, upper, lowerOpen, upperOpen);
	}

	template<typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
	valid(Real value) noexcept : valid(poxel_type(value)) {}

	template<typename Lower, typename Upper,
	         std::enable_if_t<std::is_arithmetic_v<Lower> && std::is_arithmetic_v<Upper>, int> = 0>
	valid(Lower lower, Upper upper) noexcept {
		if constexpr (std::is_integral_v<Lower> && std::is_integral_v<Upper>) {
			if (std::cmp_greater(lower, upper)) {
				setnar();
				return;
			}
		} else if (static_cast<double>(lower) > static_cast<double>(upper)) {
			setnar();
			return;
		}
		assign(poxel_type(lower), poxel_type(upper));
	}

	template<typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
	valid& operator=(Real value) noexcept {
		return *this = valid(value);
	}

	const poxel_type& lb() const noexcept { return _loPxl; }
	const poxel_type& ub() const noexcept { return _hiPxl; }

	bool lubit() const noexcept { return _loPxl.ubit(); }
	bool uubit() const noexcept { return _hiPxl.ubit(); }
	bool lower_open() const noexcept { return _loPxl.ubit(); }
	bool upper_open() const noexcept { return _hiPxl.ubit(); }
	bool lower_closed() const noexcept { return !lower_open(); }
	bool upper_closed() const noexcept { return !upper_open(); }

	bool isnar() const noexcept { return _loPxl.isnar() || _hiPxl.isnar(); }
	bool isexact() const noexcept { return !isnar() && _loPxl == _hiPxl && _loPxl.isexact(); }
	bool isinterval() const noexcept { return !isnar() && !isexact(); }
	bool iszero() const noexcept { return isexact() && _loPxl.iszero(); }
	bool contains_zero() const noexcept {
		return !isnar() && _loPxl.key() <= key_type(0) && _hiPxl.key() >= key_type(0);
	}

	// Report a sign only when the whole interval establishes it.
	// Example: v.sign() == valid_sign::positive;
	valid_sign sign() const noexcept {
		if (isnar())
			return valid_sign::undecidable;
		if (_loPxl.key() > key_type(0))
			return valid_sign::positive;
		if (_hiPxl.key() < key_type(0))
			return valid_sign::negative;
		if (iszero())
			return valid_sign::zero;
		return valid_sign::undecidable;
	}

	template<typename Real = double>
	Real lower() const noexcept {
		return isnar() ? std::numeric_limits<Real>::quiet_NaN() : _loPxl.template lower<Real>();
	}

	template<typename Real = double>
	Real upper() const noexcept {
		return isnar() ? std::numeric_limits<Real>::quiet_NaN() : _hiPxl.template upper<Real>();
	}

	// Test membership, respecting open endpoints.
	// Example: v.contains(1.5);
	template<typename Real>
	bool contains(Real value) const noexcept {
		if (isnar())
			return false;

		const Real l = lower<Real>();
		const Real u = upper<Real>();
		return (lower_open() ? l < value : l <= value) && (upper_open() ? value < u : value <= u);
	}

	valid& setnar() noexcept {
		_loPxl.setnar();
		_hiPxl.setnar();
		return *this;
	}

	valid& setinterval(const poxel_type& lower, const poxel_type& upper) noexcept {
		assign(lower, upper);
		return *this;
	}

	// Set posit endpoints and choose which endpoints are excluded.
	// Example: v.setinterval(posit<7, 1>(1), posit<7, 1>(2), true);
	valid& setinterval(const posit_type& lower, const posit_type& upper, bool lowerOpen = false,
	                   bool upperOpen = false) noexcept {
		if (lower.isnar() || upper.isnar() || upper < lower)
			return setnar();

		poxel_type loPxl(lower), hiPxl(upper);
		if (lowerOpen)
			loPxl.setkey(loPxl.key() + key_type(1));
		if (upperOpen)
			hiPxl.setkey(hiPxl.key() - key_type(1));

		assign(loPxl, hiPxl);
		return *this;
	}

	static valid entire() noexcept { return from_keys(-kmax, kmax); }

	static valid nar() noexcept {
		valid result;
		return result.setnar();
	}

	valid operator-() const noexcept { return isnar() ? *this : from_keys(-_hiPxl.key(), -_loPxl.key()); }

	valid& operator+=(const valid& rhs) noexcept {
		if (isnar() || rhs.isnar())
			return setnar();

		assign_keys(combine(lo_end(), rhs.lo_end(), operation::add, true),
		            combine(hi_end(), rhs.hi_end(), operation::add, false));
		return *this;
	}

	valid& operator-=(const valid& rhs) noexcept { return *this += -rhs; }

	valid& operator*=(const valid& rhs) noexcept {
		if (isnar() || rhs.isnar())
			return setnar();

		const end a[2] = {lo_end(), hi_end()};
		const end b[2] = {rhs.lo_end(), rhs.hi_end()};
		key_type  lo   = kmax;
		key_type  hi   = -kmax;

		for (const end& x : a)
			for (const end& y : b) {
				lo = std::min(lo, combine(x, y, operation::mul, true));
				hi = std::max(hi, combine(x, y, operation::mul, false));
			}

		assign_keys(lo, hi);
		return *this;
	}

	valid& operator/=(const valid& rhs) {
#if VALID_THROW_ARITHMETIC_EXCEPTION
		if (rhs.iszero())
			throw valid_divide_by_zero{};
#endif
		if (isnar() || rhs.isnar())
			return setnar();
		if (rhs.contains_zero()) {
			*this = rhs.iszero() ? nar() : entire();
			return *this;
		}

		const end a[2] = {lo_end(), hi_end()};
		const end b[2] = {rhs.lo_end(), rhs.hi_end()};
		key_type  lo   = kmax;
		key_type  hi   = -kmax;

		for (unsigned i = 0; i < 2; ++i)
			for (unsigned j = 0; j < 2; ++j) {
				const end& x           = a[i];
				const end& y           = b[j];
				const int  divisorSign = y.open && y.key == key_type(0) ? (j == 0 ? 1 : -1) : 0;
				const int  dividendSign = x.open && x.key == key_type(0) ? (i == 0 ? 1 : -1) : 0;

				lo = std::min(lo, combine(x, y, operation::div, true, divisorSign, dividendSign));
				hi = std::max(hi, combine(x, y, operation::div, false, divisorSign, dividendSign));
			}

		assign_keys(lo, hi);
		return *this;
	}

	friend bool operator==(const valid& lhs, const valid& rhs) noexcept {
		return lhs._loPxl == rhs._loPxl && lhs._hiPxl == rhs._hiPxl;
	}

	friend bool operator!=(const valid& lhs, const valid& rhs) noexcept { return !(lhs == rhs); }

	friend bool operator<(const valid& lhs, const valid& rhs) noexcept {
		if (lhs.isnar() || rhs.isnar())
			return false;
		if (lhs._hiPxl.key() == kmax || rhs._loPxl.key() == -kmax)
			return false;

		const key_type upper = lhs._hiPxl.key() + key_type(lhs._hiPxl.ubit());
		const key_type lower = rhs._loPxl.key() - key_type(rhs._loPxl.ubit());
		return upper < lower || (upper == lower && (lhs.upper_open() || rhs.lower_open()));
	}

	friend bool operator>(const valid& lhs, const valid& rhs) noexcept { return rhs < lhs; }
	friend bool operator<=(const valid& lhs, const valid& rhs) noexcept { return lhs < rhs || lhs == rhs; }
	friend bool operator>=(const valid& lhs, const valid& rhs) noexcept { return rhs <= lhs; }

	friend valid square(const valid& value) noexcept {
		valid result = value * value;
		if (result.isnar())
			return result;

		if ((value._loPxl.key() < key_type(0) && value._hiPxl.key() > key_type(0)) ||
		    result._loPxl.key() < key_type(0)) {
			return from_keys(key_type(0), result._hiPxl.key());
		}
		return result;
	}

	// Return the smallest interval enclosing both inputs; ignore a NaR input.
	// Example: auto merged = hull(valid<8, 1>(1, 2), valid<8, 1>(4, 5));
	friend valid hull(const valid& a, const valid& b) noexcept {
		if (a.isnar())
			return b;
		if (b.isnar())
			return a;

		return from_keys(std::min(a._loPxl.key(), b._loPxl.key()), std::max(a._hiPxl.key(), b._hiPxl.key()));
	}

	// Return the overlap, or NaR if disjoint; a NaR input leaves the first input.
	// Example: auto shared = intersect(valid<8, 1>(1, 3), valid<8, 1>(2, 4));
	friend valid intersect(const valid& a, const valid& b) noexcept {
		if (a.isnar() || b.isnar())
			return a;

		const key_type lo = std::max(a._loPxl.key(), b._loPxl.key());
		const key_type hi = std::min(a._hiPxl.key(), b._hiPxl.key());
		return lo > hi ? nar() : from_keys(lo, hi);
	}

	friend valid sqrt(const valid& value) noexcept {
		if (value.isnar() || value._hiPxl.key() < key_type(0))
			return nar();

		key_type lo = key_type(0);
		if (value._loPxl.key() > key_type(0)) {
			const bool     open = value.lower_open();
			const key_type key  = sqrt_key(open ? value._loPxl.key() - key_type(1) : value._loPxl.key());
			lo                  = open && !odd(key) ? key + key_type(1) : key;
		}

		key_type hi = kmax;
		if (value._hiPxl.key() < kmax) {
			const bool     open = value.upper_open();
			const key_type key  = sqrt_key(open ? value._hiPxl.key() + key_type(1) : value._hiPxl.key());
			hi                  = open && !odd(key) ? key - key_type(1) : key;
		}
		return from_keys(lo, std::max(lo, hi));
	}

	friend valid cos(const valid& value) {
		if (value.isnar())
			return value;

		constexpr int upperTerms = 14;
		const valid   s          = square(value);
		valid         coefficients[upperTerms + 2];

		coefficients[0] = valid(1);
		for (int j = 1; j <= upperTerms + 1; ++j)
			coefficients[j] = coefficients[j - 1] / valid((2 * j - 1) * (2 * j));

		auto partial = [&](int m) {
			valid result = (m % 2) ? -coefficients[m] : coefficients[m];
			for (int j = m - 1; j >= 0; --j)
				result = result * s + ((j % 2) ? -coefficients[j] : coefficients[j]);
			return result;
		};

		const valid lo = partial(upperTerms + 1);
		const valid hi = partial(upperTerms);
		return intersect(from_keys(lo._loPxl.key(), hi._hiPxl.key()), valid(poxel_type(-1), poxel_type(1)));
	}

  private:
	poxel_type _loPxl{};
	poxel_type _hiPxl{};

	struct end {
		key_type key;
		int      infinity;
		bool     open;
	};

	enum class operation {
		add,
		mul,
		div
	};

	static bool odd(const key_type& key) noexcept { return (key & key_type(1)) != key_type(0); }

	static poxel_type point_pxl(const key_type& key) noexcept {
		poxel_type pxl{};
		pxl.setkey(key);
		return pxl;
	}

	static valid from_keys(const key_type& lo, const key_type& hi) noexcept {
		valid result;
		result.assign_keys(lo, hi);
		return result;
	}

	void assign_keys(const key_type& lo, const key_type& hi) noexcept { assign(point_pxl(lo), point_pxl(hi)); }

	end lo_end() const noexcept {
		const key_type key = _loPxl.key();
		if (key == -kmax)
			return {key_type(0), -1, true};
		return {odd(key) ? key - key_type(1) : key, 0, odd(key)};
	}

	end hi_end() const noexcept {
		const key_type key = _hiPxl.key();
		if (key == kmax)
			return {key_type(0), 1, true};
		return {odd(key) ? key + key_type(1) : key, 0, odd(key)};
	}

	static int signum(const end& value) noexcept {
		return value.infinity != 0 ? value.infinity : (value.key > key_type(0)) - (value.key < key_type(0));
	}

	static key_type clamp_key(const key_type& key) noexcept { return std::clamp(key, -kmax, kmax); }

	// Enclose one endpoint operation; strict exact results move to the open poxel.
	// Example: combine(lo_end(), rhs.lo_end(), operation::add, true).
	static key_type combine(const end& x, const end& y, operation op, bool lowerBound, int divisorSign = 0, int dividendSign = 0) noexcept {
		const key_type widest = lowerBound ? -kmax : kmax;

		if (x.infinity == 0 && y.infinity == 0) {
			const poxel_type xPxl = point_pxl(x.key);
			const poxel_type yPxl = point_pxl(y.key);
			poxel_type       resultPxl{};

			switch (op) {
			case operation::add:
				resultPxl = xPxl + yPxl;
				break;
			case operation::mul:
				resultPxl = xPxl * yPxl;
				break;
			case operation::div:
				if (y.key == key_type(0)) {
                   if (x.key == key_type(0)) {
                        if (!x.open)
							return key_type(0);
						const int s = dividendSign * divisorSign;
						if (s > 0)
							return lowerBound ? key_type(1) : kmax;
						if (s < 0)
							return lowerBound ? -kmax : key_type(-1);
						return widest;
					}

                    const int quotientSign = signum(x) * divisorSign;
					return quotientSign > 0 ? kmax : -kmax;
				}
				resultPxl = xPxl / yPxl;
				break;
			}
			if (resultPxl.isnar())
				return widest;

			const key_type key    = clamp_key(resultPxl.key());
			bool           strict = false;

			switch (op) {
			case operation::add:
				strict = x.open || y.open;
				break;
			case operation::mul:
			case operation::div:
				strict = (x.open && y.key != key_type(0)) || (y.open && x.key != key_type(0));
				break;
			}
			if (strict && !odd(key))
				return lowerBound ? key + key_type(1) : key - key_type(1);
			return key;
		}

		int resultSign = 0;
		switch (op) {
		case operation::add:
			if (x.infinity != 0 && y.infinity != 0 && x.infinity != y.infinity)
				return widest;
			resultSign = x.infinity != 0 ? x.infinity : y.infinity;
			break;
		case operation::mul:
			if (signum(x) == 0 || signum(y) == 0)
				return key_type(0);
			resultSign = signum(x) * signum(y);
			break;
		case operation::div:
			if (y.infinity != 0)
				return x.infinity != 0 ? widest : key_type(0);
			if (y.key == key_type(0)) {
				const int quotientSign = signum(x) * divisorSign;
				return quotientSign > 0 ? kmax : -kmax;
			}
			resultSign = signum(x) * signum(y);
			break;
		}
		return resultSign > 0 ? kmax : -kmax;
	}
	// Locate the poxel containing the root of an exact lattice point.
	// Example: sqrt_key(key_type(8)).
	static key_type sqrt_key(const key_type& valueKey) noexcept {
		if (valueKey <= key_type(0))
			return key_type(0);

		auto square_key = [](const key_type& key) {
			const poxel_type pxl = point_pxl(key);
			return clamp_key((pxl * pxl).key());
		};

		key_type lo = key_type(0);
		key_type hi = (kmax - key_type(1)) / key_type(2);

		while (lo < hi) {
			const key_type mid = lo + (hi - lo + key_type(1)) / key_type(2);
			if (square_key(key_type(2) * mid) <= valueKey)
				lo = mid;
			else
				hi = mid - key_type(1);
		}

		const key_type key = key_type(2) * lo;
		return square_key(key) == valueKey ? key : key + key_type(1);
	}

	void assign(const poxel_type& lower, const poxel_type& upper) noexcept {
		if (lower.isnar() || upper.isnar() || lower.key() > upper.key()) {
			setnar();
		} else {
			_loPxl = lower;
			_hiPxl = upper;
		}
	}
};

// Promote one poxel to a Valid before doing enclosing arithmetic.
// Example: auto v = to_valid(poxel<8, 1>(0.1));
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> to_valid(const poxel<nbits, es, bt>& pxl) noexcept {
	return valid<nbits, es, bt>(pxl);
}

template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> operator+(valid<nbits, es, bt> lhs, const valid<nbits, es, bt>& rhs) noexcept {
	return lhs += rhs;
}
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> operator-(valid<nbits, es, bt> lhs, const valid<nbits, es, bt>& rhs) noexcept {
	return lhs -= rhs;
}
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> operator*(valid<nbits, es, bt> lhs, const valid<nbits, es, bt>& rhs) noexcept {
	return lhs *= rhs;
}
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> operator/(valid<nbits, es, bt> lhs, const valid<nbits, es, bt>& rhs) {
	return lhs /= rhs;
}

// Enclose arithmetic on poxels rather than using their sticky-ubit operators.
// Example: auto v = div_valid(poxel<8, 1>(1), poxel<8, 1>(3));
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> add_valid(const poxel<nbits, es, bt>& a, const poxel<nbits, es, bt>& b) noexcept {
	return to_valid(a) + to_valid(b);
}
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> sub_valid(const poxel<nbits, es, bt>& a, const poxel<nbits, es, bt>& b) noexcept {
	return to_valid(a) - to_valid(b);
}
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> mul_valid(const poxel<nbits, es, bt>& a, const poxel<nbits, es, bt>& b) noexcept {
	return to_valid(a) * to_valid(b);
}
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> div_valid(const poxel<nbits, es, bt>& a, const poxel<nbits, es, bt>& b) {
	return to_valid(a) / to_valid(b);
}

template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> operator+(valid<nbits, es, bt> lhs, const poxel<nbits, es, bt>& rhs) noexcept {
	return lhs += to_valid(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> operator+(const poxel<nbits, es, bt>& lhs, valid<nbits, es, bt> rhs) noexcept {
	return rhs += to_valid(lhs);
}
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> operator-(valid<nbits, es, bt> lhs, const poxel<nbits, es, bt>& rhs) noexcept {
	return lhs -= to_valid(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> operator-(const poxel<nbits, es, bt>& lhs, const valid<nbits, es, bt>& rhs) noexcept {
	return to_valid(lhs) - rhs;
}
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> operator*(valid<nbits, es, bt> lhs, const poxel<nbits, es, bt>& rhs) noexcept {
	return lhs *= to_valid(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> operator*(const poxel<nbits, es, bt>& lhs, valid<nbits, es, bt> rhs) noexcept {
	return rhs *= to_valid(lhs);
}
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> operator/(valid<nbits, es, bt> lhs, const poxel<nbits, es, bt>& rhs) {
	return lhs /= to_valid(rhs);
}
template<unsigned nbits, unsigned es, typename bt>
valid<nbits, es, bt> operator/(const poxel<nbits, es, bt>& lhs, const valid<nbits, es, bt>& rhs) {
	return to_valid(lhs) / rhs;
}

template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
valid<nbits, es, bt> operator+(valid<nbits, es, bt> lhs, Real rhs) noexcept {
	return lhs += valid<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
valid<nbits, es, bt> operator+(Real lhs, valid<nbits, es, bt> rhs) noexcept {
	return rhs += valid<nbits, es, bt>(lhs);
}
template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
valid<nbits, es, bt> operator-(valid<nbits, es, bt> lhs, Real rhs) noexcept {
	return lhs -= valid<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
valid<nbits, es, bt> operator-(Real lhs, const valid<nbits, es, bt>& rhs) noexcept {
	return valid<nbits, es, bt>(lhs) - rhs;
}
template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
valid<nbits, es, bt> operator*(valid<nbits, es, bt> lhs, Real rhs) noexcept {
	return lhs *= valid<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
valid<nbits, es, bt> operator*(Real lhs, valid<nbits, es, bt> rhs) noexcept {
	return rhs *= valid<nbits, es, bt>(lhs);
}
template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
valid<nbits, es, bt> operator/(valid<nbits, es, bt> lhs, Real rhs) {
	return lhs /= valid<nbits, es, bt>(rhs);
}
template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
valid<nbits, es, bt> operator/(Real lhs, const valid<nbits, es, bt>& rhs) {
	return valid<nbits, es, bt>(lhs) / rhs;
}

}  // namespace universal
}  // namespace sw
