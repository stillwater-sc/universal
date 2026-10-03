#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/number/posit/core.hpp>
#include <universal/number/valid/exceptions.hpp>
#include <universal/number/valid/valid_fwd.hpp>

#if !defined(VALID_THROW_ARITHMETIC_EXCEPTION)
#	define VALID_THROW_ARITHMETIC_EXCEPTION 0
#endif

namespace sw {
namespace universal {

template<unsigned _nbits, unsigned _es, typename bt>
class valid {
  public:
	static_assert(_nbits >= 2, "a valid bound requires a posit of at least two bits");
	static_assert(_es <= _nbits - 2, "the posit exponent field does not fit");

	static constexpr unsigned nbits = _nbits;
	static constexpr unsigned es    = _es;
	using block_type                = bt;
	using posit_type                = posit<nbits, es, bt>;

	constexpr valid() noexcept                        = default;
	constexpr valid(const valid&) noexcept            = default;
	constexpr valid(valid&&) noexcept                 = default;
	constexpr valid& operator=(const valid&) noexcept = default;
	constexpr valid& operator=(valid&&) noexcept      = default;

	constexpr valid(const posit_type& value) noexcept : _lb(value), _ub(value) {
		if (value.isnar())
			setnar();
	}

	constexpr valid(const posit_type& lower, const posit_type& upper, bool lowerOpen = false,
	                bool upperOpen = false) noexcept {
		assign(lower, upper, lowerOpen, upperOpen);
	}

	template<typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
	valid(Real value) noexcept {
		const enclosure e = enclose_native(value);
		assign(e.lower, e.upper, !e.exact, !e.exact);
	}

	template<typename Lower, typename Upper,
	         std::enable_if_t<std::is_arithmetic_v<Lower> && std::is_arithmetic_v<Upper>, int> = 0>
	valid(Lower lower, Upper upper) noexcept {
		const enclosure l = enclose_native(lower);
		const enclosure u = enclose_native(upper);
		assign(l.lower, u.upper, !l.exact, !u.exact);
	}

	template<typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
	valid& operator=(Real value) noexcept {
		return *this = valid(value);
	}

	constexpr const posit_type& lb() const noexcept { return _lb; }
	constexpr const posit_type& ub() const noexcept { return _ub; }
	constexpr bool              lubit() const noexcept { return _lubit; }
	constexpr bool              uubit() const noexcept { return _uubit; }
	constexpr bool              lower_open() const noexcept { return _lubit; }
	constexpr bool              upper_open() const noexcept { return _uubit; }
	constexpr bool              lower_closed() const noexcept { return !_lubit; }
	constexpr bool              upper_closed() const noexcept { return !_uubit; }
	constexpr bool              isexact() const noexcept { return !isnar() && !_lubit && !_uubit && _lb == _ub; }
	constexpr bool              isinterval() const noexcept { return !isexact() && !isnar(); }
	constexpr bool              isnar() const noexcept { return _lb.isnar() || _ub.isnar(); }
	constexpr bool              iszero() const noexcept { return isexact() && _lb.iszero(); }
	constexpr bool              contains_zero() const noexcept {
		if (isnar())
			return false;
		const posit_type zero{};
		if (_lb < zero && _ub > zero)
			return true;
		if (_lb == zero && !_lubit)
			return true;
		return _ub == zero && !_uubit;
	}

	constexpr valid& setnar() noexcept {
		_lb.setnar();
		_ub.setnar();
		_lubit = false;
		_uubit = false;
		return *this;
	}

	constexpr valid& setinterval(const posit_type& lower, const posit_type& upper, bool lowerOpen = false,
	                             bool upperOpen = false) noexcept {
		assign(lower, upper, lowerOpen, upperOpen);
		return *this;
	}

	constexpr valid operator-() const noexcept {
		if (isnar())
			return *this;
		return valid(-_ub, -_lb, _uubit, _lubit);
	}

	constexpr valid& operator+=(const valid& rhs) noexcept {
		if (isnar() || rhs.isnar())
			return setnar();
		const enclosure lower = add_enclosure(_lb, rhs._lb);
		const enclosure upper = add_enclosure(_ub, rhs._ub);
		assign(lower.lower, upper.upper, !lower.exact || _lubit || rhs._lubit, !upper.exact || _uubit || rhs._uubit);
		return *this;
	}

	constexpr valid& operator-=(const valid& rhs) noexcept { return *this += -rhs; }

	constexpr valid& operator*=(const valid& rhs) noexcept {
		if (isnar() || rhs.isnar())
			return setnar();

		product_candidate candidates[4] = {multiply_candidate(_lb, rhs._lb, _lubit || rhs._lubit),
		                                   multiply_candidate(_lb, rhs._ub, _lubit || rhs._uubit),
		                                   multiply_candidate(_ub, rhs._lb, _uubit || rhs._lubit),
		                                   multiply_candidate(_ub, rhs._ub, _uubit || rhs._uubit)};

		posit_type lower     = candidates[0].value.lower;
		posit_type upper     = candidates[0].value.upper;
		bool       lowerOpen = !candidates[0].value.exact || candidates[0].inputOpen;
		bool       upperOpen = lowerOpen;

		for (unsigned i = 1; i < 4; ++i) {
			const bool candidateLowerOpen = !candidates[i].value.exact || candidates[i].inputOpen;
			const bool candidateUpperOpen = candidateLowerOpen;
			if (candidates[i].value.lower < lower) {
				lower     = candidates[i].value.lower;
				lowerOpen = candidateLowerOpen;
			} else if (candidates[i].value.lower == lower) {
				lowerOpen = lowerOpen && candidateLowerOpen;
			}
			if (candidates[i].value.upper > upper) {
				upper     = candidates[i].value.upper;
				upperOpen = candidateUpperOpen;
			} else if (candidates[i].value.upper == upper) {
				upperOpen = upperOpen && candidateUpperOpen;
			}
		}
		assign(lower, upper, lowerOpen, upperOpen);
		return *this;
	}

	constexpr valid& operator/=(const valid& rhs) {
		if (isnar() || rhs.isnar())
			return setnar();
		const posit_type zero{};
		if (!(rhs._ub < zero) && !(rhs._lb > zero)) {
#if VALID_THROW_ARITHMETIC_EXCEPTION
			throw valid_divide_by_zero{};
#else
			return setnar();
#endif
		}

		quotient_candidate candidates[4] = {
		    divide_candidate(_lb, rhs._lb, _lubit || rhs._lubit), divide_candidate(_lb, rhs._ub, _lubit || rhs._uubit),
		    divide_candidate(_ub, rhs._lb, _uubit || rhs._lubit), divide_candidate(_ub, rhs._ub, _uubit || rhs._uubit)};

		posit_type lower     = candidates[0].value.lower;
		posit_type upper     = candidates[0].value.upper;
		bool       lowerOpen = !candidates[0].value.exact || candidates[0].inputOpen;
		bool       upperOpen = lowerOpen;
		for (unsigned i = 1; i < 4; ++i) {
			const bool candidateOpen = !candidates[i].value.exact || candidates[i].inputOpen;
			if (candidates[i].value.lower < lower) {
				lower     = candidates[i].value.lower;
				lowerOpen = candidateOpen;
			} else if (candidates[i].value.lower == lower) {
				lowerOpen = lowerOpen && candidateOpen;
			}
			if (candidates[i].value.upper > upper) {
				upper     = candidates[i].value.upper;
				upperOpen = candidateOpen;
			} else if (candidates[i].value.upper == upper) {
				upperOpen = upperOpen && candidateOpen;
			}
		}
		assign(lower, upper, lowerOpen, upperOpen);
		return *this;
	}

	friend constexpr bool operator==(const valid& lhs, const valid& rhs) noexcept {
		return lhs._lb == rhs._lb && lhs._ub == rhs._ub && lhs._lubit == rhs._lubit && lhs._uubit == rhs._uubit;
	}
	friend constexpr bool operator!=(const valid& lhs, const valid& rhs) noexcept { return !(lhs == rhs); }
	friend constexpr bool operator<(const valid& lhs, const valid& rhs) noexcept {
		if (lhs.isnar() || rhs.isnar())
			return false;
		return lhs._ub < rhs._lb || (lhs._ub == rhs._lb && (lhs._uubit || rhs._lubit));
	}
	friend constexpr bool operator>(const valid& lhs, const valid& rhs) noexcept { return rhs < lhs; }
	friend constexpr bool operator<=(const valid& lhs, const valid& rhs) noexcept { return lhs < rhs || lhs == rhs; }
	friend constexpr bool operator>=(const valid& lhs, const valid& rhs) noexcept { return rhs <= lhs; }

  private:
	static constexpr unsigned fbits = posit_type::fbits;

	struct enclosure {
		posit_type lower{};
		posit_type upper{};
		bool       exact{true};
	};
	struct product_candidate {
		enclosure value{};
		bool      inputOpen{false};
	};
	using quotient_candidate = product_candidate;

	posit_type _lb{};
	posit_type _ub{};
	bool       _lubit{false};
	bool       _uubit{false};

	constexpr void assign(const posit_type& lower, const posit_type& upper, bool lowerOpen, bool upperOpen) noexcept {
		if (lower.isnar() || upper.isnar() || upper < lower) {
			setnar();
			return;
		}
		_lb    = lower;
		_ub    = upper;
		_lubit = lowerOpen;
		_uubit = upperOpen;
		if (_lb == _ub && (_lubit || _uubit)) {
			setnar();
		}
	}

	static constexpr posit_type next_up(posit_type value) noexcept {
		posit_type candidate(value);
		++candidate;
		return candidate.isnar() ? value : candidate;
	}

	static constexpr posit_type next_down(posit_type value) noexcept {
		posit_type candidate(value);
		--candidate;
		return candidate.isnar() ? value : candidate;
	}

	template<typename Real>
	static enclosure enclose_native(Real value) noexcept {
		const posit_type rounded(value);
		if (rounded.isnar())
			return {rounded, rounded, false};
		const long double source        = static_cast<long double>(value);
		const long double approximation = static_cast<long double>(rounded);
		if (source == approximation)
			return {rounded, rounded, true};
		if (approximation < source) {
			return {rounded, next_up(rounded), false};
		}
		return {next_down(rounded), rounded, false};
	}

	template<BlockTripleOperator op>
	static constexpr void normalize_for(const posit_type& value, blocktriple<fbits, op, bt>& target) noexcept {
		if constexpr (op == BlockTripleOperator::ADD) {
			value.normalizeAddition(target);
		} else if constexpr (op == BlockTripleOperator::MUL) {
			value.normalizeMultiplication(target);
		} else {
			value.normalizeDivision(target);
		}
	}

	template<BlockTripleOperator op>
	static constexpr enclosure enclose_triple(const blocktriple<fbits, op, bt>& exact) noexcept {
		posit_type rounded;
		convert(exact, rounded);
		if (rounded.isnar())
			return {rounded, rounded, false};
		if (exact.iszero())
			return {rounded, rounded, true};

		blocktriple<fbits, op, bt> represented;
		if constexpr (op == BlockTripleOperator::MUL) {
			blocktriple<fbits, op, bt> candidate, one;
			rounded.normalizeMultiplication(candidate);
			posit_type(1).normalizeMultiplication(one);
			represented.mul(candidate, one);
		} else {
			normalize_for<op>(rounded, represented);
		}
		const int relation = compare_triples(represented, exact);
		if (relation == 0)
			return {rounded, rounded, true};
		if (relation < 0) {
			return {rounded, next_up(rounded), false};
		}
		return {next_down(rounded), rounded, false};
	}

	template<BlockTripleOperator op>
	static constexpr int compare_triples(const blocktriple<fbits, op, bt>& lhs,
	                                     const blocktriple<fbits, op, bt>& rhs) noexcept {
		if (lhs.iszero())
			return rhs.iszero() ? 0 : (rhs.sign() ? 1 : -1);
		if (rhs.iszero())
			return lhs.sign() ? -1 : 1;

		const int lhsSignificandScale    = lhs.significandscale();
		const int rhsSignificandScale    = rhs.significandscale();
		const int commonSignificandScale = std::max(lhsSignificandScale, rhsSignificandScale);

		auto alignedLhs(lhs);
		auto alignedRhs(rhs);
		alignedLhs.bitShift(commonSignificandScale - lhsSignificandScale);
		alignedRhs.bitShift(commonSignificandScale - rhsSignificandScale);
		alignedLhs.setscale(lhs.scale() + lhsSignificandScale - commonSignificandScale);
		alignedRhs.setscale(rhs.scale() + rhsSignificandScale - commonSignificandScale);
		if (alignedLhs == alignedRhs)
			return 0;
		return alignedLhs < alignedRhs ? -1 : 1;
	}

	static constexpr enclosure add_enclosure(const posit_type& lhs, const posit_type& rhs) noexcept {
		blocktriple<fbits, BlockTripleOperator::ADD, bt> a, b, sum;
		lhs.normalizeAddition(a);
		rhs.normalizeAddition(b);
		sum.add(a, b);
		return enclose_triple(sum);
	}

	static constexpr enclosure multiply_enclosure(const posit_type& lhs, const posit_type& rhs) noexcept {
		blocktriple<fbits, BlockTripleOperator::MUL, bt> a, b, product;
		lhs.normalizeMultiplication(a);
		rhs.normalizeMultiplication(b);
		product.mul(a, b);
		return enclose_triple(product);
	}

	static constexpr product_candidate multiply_candidate(const posit_type& lhs, const posit_type& rhs,
	                                                      bool inputOpen) noexcept {
		return {multiply_enclosure(lhs, rhs), inputOpen};
	}

	static constexpr enclosure divide_enclosure(const posit_type& numerator, const posit_type& denominator) noexcept {
		if (numerator.iszero()) {
			const posit_type zero{};
			return {zero, zero, true};
		}
		blocktriple<fbits, BlockTripleOperator::DIV, bt> a, b, ratio;
		numerator.normalizeDivision(a);
		denominator.normalizeDivision(b);
		ratio.div(a, b);

		posit_type rounded;
		convert(ratio, rounded);
		if (rounded.isnar())
			return {rounded, rounded, false};

		// Compare q*d with n using an exact blocktriple multiplication. This also
		// detects division remainders that fall below the quotient buffer.
		blocktriple<fbits, BlockTripleOperator::MUL, bt> q, d, qd;
		blocktriple<fbits, BlockTripleOperator::MUL, bt> n, one, nAligned;
		rounded.normalizeMultiplication(q);
		denominator.normalizeMultiplication(d);
		qd.mul(q, d);
		numerator.normalizeMultiplication(n);
		posit_type(1).normalizeMultiplication(one);
		nAligned.mul(n, one);

		const int productRelation = compare_triples(qd, nAligned);
		if (productRelation == 0)
			return {rounded, rounded, true};
		const bool roundedBelowExact = denominator > posit_type{} ? productRelation < 0 : productRelation > 0;
		if (roundedBelowExact) {
			return {rounded, next_up(rounded), false};
		}
		return {next_down(rounded), rounded, false};
	}

	static constexpr quotient_candidate divide_candidate(const posit_type& lhs, const posit_type& rhs,
	                                                     bool inputOpen) noexcept {
		return {divide_enclosure(lhs, rhs), inputOpen};
	}
};

template<unsigned nbits, unsigned es, typename bt>
constexpr valid<nbits, es, bt> operator+(valid<nbits, es, bt> lhs, const valid<nbits, es, bt>& rhs) noexcept {
	return lhs += rhs;
}

template<unsigned nbits, unsigned es, typename bt>
constexpr valid<nbits, es, bt> operator-(valid<nbits, es, bt> lhs, const valid<nbits, es, bt>& rhs) noexcept {
	return lhs -= rhs;
}

template<unsigned nbits, unsigned es, typename bt>
constexpr valid<nbits, es, bt> operator*(valid<nbits, es, bt> lhs, const valid<nbits, es, bt>& rhs) noexcept {
	return lhs *= rhs;
}

template<unsigned nbits, unsigned es, typename bt>
constexpr valid<nbits, es, bt> operator/(valid<nbits, es, bt> lhs, const valid<nbits, es, bt>& rhs) {
	return lhs /= rhs;
}

template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
constexpr valid<nbits, es, bt> operator+(valid<nbits, es, bt> lhs, Real rhs) noexcept {
	return lhs += valid<nbits, es, bt>(rhs);
}

template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
constexpr valid<nbits, es, bt> operator+(Real lhs, valid<nbits, es, bt> rhs) noexcept {
	return rhs += valid<nbits, es, bt>(lhs);
}

template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
constexpr valid<nbits, es, bt> operator-(valid<nbits, es, bt> lhs, Real rhs) noexcept {
	return lhs -= valid<nbits, es, bt>(rhs);
}

template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
constexpr valid<nbits, es, bt> operator-(Real lhs, const valid<nbits, es, bt>& rhs) noexcept {
	return valid<nbits, es, bt>(lhs) - rhs;
}

template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
constexpr valid<nbits, es, bt> operator*(valid<nbits, es, bt> lhs, Real rhs) noexcept {
	return lhs *= valid<nbits, es, bt>(rhs);
}

template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
constexpr valid<nbits, es, bt> operator*(Real lhs, valid<nbits, es, bt> rhs) noexcept {
	return rhs *= valid<nbits, es, bt>(lhs);
}

template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
constexpr valid<nbits, es, bt> operator/(valid<nbits, es, bt> lhs, Real rhs) {
	return lhs /= valid<nbits, es, bt>(rhs);
}

template<unsigned nbits, unsigned es, typename bt, typename Real, std::enable_if_t<std::is_arithmetic_v<Real>, int> = 0>
constexpr valid<nbits, es, bt> operator/(Real lhs, const valid<nbits, es, bt>& rhs) {
	return valid<nbits, es, bt>(lhs) / rhs;
}

}  // namespace universal
}  // namespace sw
