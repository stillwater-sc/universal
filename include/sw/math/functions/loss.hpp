#pragma once
// loss.hpp: definition of different loss functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

/*
Taken from:

Robust Bi-Tempered Logistic Loss Based on Bregman Divergences
Ehsan Amid: Manfred K. Warmuth: Rohan Anil : Tomer Koren :
  Department of Computer Science, University of California, Santa Cruz
: Google Brain
{eamid, manfred, rohananil, tkoren}@google.com

The logistic loss is essentially the logarithm of the predicted class probabilities, which are computed
as the normalized exponentials of the inputs. In this paper, we tackle both shortcomings of the logistic
loss, pertaining to its convexity as well as its tail-lightness, by replacing the logarithm and exponential
functions with their corresponding tempered versions. We define the function logt : R+ -> R with
temperature parameter t >= 0 as in:

       logt(x) :=  (1/(1 - t)) * (x^(1-t)  1) .

The log sub t function is monotonically increasing and concave.The standard(natural) logarithm is
recovered at the limit t -> 1. Unlike the standard log, the logt function is bounded from below
by -1 / (1 - t) for 0 <= t < 1. This property will be used to define bounded loss functions that are
significantly more robust to outliers.Similarly, our heavy - tailed alternative for the softmax function
is based on the tempered exponential function.The function expt : R -> R+ with temperature t E R
is defined as the inverse of logt, that is,

	expt(x) : = ([1 + (1 - t) x]+) ^ (1 / (1 - t))

where [ . ]+ = max{ . , 0 }.
	
The standard exp function is again recovered at the limit t -> 1. Compared
to the exp function, a heavier tail (for negative values of x) is achieved for t > 1. We use this property
to define heavy - tailed analogues of softmax probabilities at the output layer.

The vanilla logistic loss can be viewed as a logarithmic(relative entropy) divergence that operates on
a matching exponential(softmax) probability assignment[10, 11].Its convexity then stems from
classical convex duality, using the fact that the probability assignment function is the gradient of the
dual function to the entropy on the simplex.When the logt1 and expt2 are substituted instead, this
duality still holds whenever t1 = t2, albeit with a different Bregman divergence, and the induced loss
remains convex2.However, for t1 < t2, the loss becomes non - convex in the output activations.In
particular, 0 <= t1 < 1 leads to a bounded loss, while t2 > 1 provides tail - heaviness. 

 */

namespace sw::math::function {

    // tempered logarithm logt(x) :=  (1/(1 - t)) * (x^(1 - t) -  1)
    // Unlike the standard log, the logt function is bounded from below -1/(1 - t) for 0 <= t < 1.
    // This property will be used to define bounded loss functions that are more robust to outliers.
    template<typename Scalar>
    Scalar logt(const Scalar& temp, const Scalar& x) {
	    assert(x >= 0);
	    Scalar one_minus_temp = Scalar(1) - temp;
	    return (pow(x, one_minus_temp) - Scalar(1))/one_minus_temp;
    }

    // tempered exponent expt(x) : = [1 + (1 - t) x]^(1 / (1 - t))
    template<typename Scalar>
    Scalar expt(const Scalar& temp, const Scalar& x) {
	    Scalar one_minus_temp = Scalar(1) - temp;
	    Scalar base = max(Scalar(1) + one_minus_temp * x, Scalar(0));
	    return (pow(base, (Scalar(1) / one_minus_temp)));
    }

} // namespace sw::math::function

