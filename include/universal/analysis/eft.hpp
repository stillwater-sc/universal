#pragma once


namespace sw { namespace universal {

    // TwoSum error free transform for a sum: a + b = sum + residual
    // returns residual
    // WARNING requires rounding to nearest (see Priest)
    // NOTE the volatile keyword is required to avoid the operation being optimized 
    // away by a compiler using associativity rules
    template <typename Real>
    inline const Real TwoSum(Real a, Real b, const volatile Real result)
    {
        const Real bAprox = result - a;
        const Real aAprox = result - bAprox;
        const volatile Real aEpsilon = a - aAprox;
        const volatile Real bEpsilon = b - bAprox;
        const Real error = aEpsilon + bEpsilon;
        return error;
    }

    // FastTwoProd error free transform for a multiplication
    // see also dekker's multiplication algorithm (rounding to nearest) when an FMA is unavailable
    // NOTE proof for rounding toward zero in "Error-Free Transformation in Rounding Mode toward Zero"
    // WARNING proven only for rounding to nearest and toward zero
    template <typename Real>
    inline const Real FastTwoProd(Real a, Real b, Real result)
    {
        const Real error = std::fma(a, b, -result);
        return error;
    }

    // RemainderDiv computes the remainder of a division
    // see the Handbook of floating point arithmetic
    template <typename Real>
    inline const Real RemainderDiv(Real a, Real b, Real result)
    {
        Real remainder = -std::fma(b, result, -a);
        return remainder;
    }


}} // namespace sw::universal
