# TwoSum and TwoProd properties

## TwoSum/TwoProd validity in subnormal regime

**TwoSum perfectly preserves its exact error-free transformation even with subnormals**, provided there is no overflow. **TwoProduct, however, fails** when the exact product underflows the subnormal range.

Here is a breakdown of why they behave differently and how to resolve this for your proofs.

## Why TwoSum Survives Subnormals

In IEEE 754, every finite floating-point number $x$ can be viewed as an exact integer multiple of the smallest representable subnormal number. In binary64 (double precision), this minimum quantum is **$2^{-1074}$**.

Because of this property, when you add two floating-point numbers $a$ and $b$:

1. The exact mathematical sum $a + b$ is also guaranteed to be a multiple of **$2^{-1074}$**.
2. When the sum is rounded to form $s = \text{fl}(a + b)$, the bits that are shifted out and discarded (which form your error $e$) are just a truncated chunk of the smaller operand.
3. Because the error $e$ is merely a subset of bits from an already-representable floating-point number, $e$ itself must fit perfectly within the precision and exponent limits of the format.

In fact, if the addition occurs entirely within the subnormal range, no bits are ever discarded. Subnormals share a fixed exponent, meaning floating-point addition of two subnormals acts like exact integer arithmetic. Thus, $s = a + b$ exactly, and $e = 0$.

**Conclusion for TwoSum:** As long as $a + b$ does not overflow to infinity, $a + b = s + e$ is mathematically exact and $e$ is always representable in IEEE 754.

---

## Why TwoProduct Fails on Underflow

Multiplication behaves entirely differently because it adds the exponents. If you multiply two very small floating-point numbers, their exact product can fall below the absolute minimum threshold of the format, creating an error term that is physically too small to be represented.

### A Concrete Example (binary64)

Let's define our variables in double precision:

* Let $a$ be the smallest possible subnormal number: **$2^{-1074}$**.
* Let $b$ be **0.5** (which is **$2^{-1}$**).

If we compute $a \times b$:

1. The **exact mathematical product** is **$2^{-1075}$**.
2. The number **$2^{-1075}$** is exactly halfway between **0** and the smallest representable number (**$2^{-1074}$**).
3. Under the default IEEE 754 "round ties to even" rule, **$2^{-1075}$** rounds to **0**. So, $p = \text{fl}(a \times b) = 0$.
4. The **exact error** is $e = (a \times b) - p = 2^{-1075} - 0 = 2^{-1075}$.

Here lies the failure: **$2^{-1075}$** is strictly smaller than the smallest representable floating-point number. You physically cannot represent $e$ in binary64. Whether you use the classic Veltkamp/Dekker algorithm or a modern hardware Fused Multiply-Add (`fma(a, b, -p)`), the calculation for $e$ will simply underflow to **0**, incorrectly claiming that the product was exactly **0** with no error.

**Conclusion for TwoProduct:** If $a \times b$ underflows, $a \times b = p + e$ is mathematically violated because $e$ falls into the unrepresentable gap below the smallest subnormal.

---

## How to Handle the Discrepancy in Your Proofs

If you are proving properties for numerical software (like a double-double arithmetic library or a compensated summation algorithm), you resolve this discrepancy by treating the two operations distinctly:

* **For Addition:** You can rely on TwoSum and FastTwoSum unconditionally. You do not need to add caveats for subnormal numbers in your proofs as long as overflow is bounded.
* **For Multiplication:** You must explicitly bound the inputs or add underflow-detection logic. A common proof condition for TwoProduct is explicitly requiring that $\vert{}a \times b\vert{} \ge N_{min}$ (where $N_{min}$ is the smallest normal number, **$2^{-1022}$** in binary64), ensuring that subnormals are never generated as the result of the operation.

## TwoSum in the subnormal regime

When you add or subtract in the subnormal range, floating-point math actually becomes entirely exact. Here is the step-by-step trace of your scenario in binary64 (double precision).

### 1. Defining the Variables

* Let $a$ be the smallest positive subnormal:
**$a = 2^{-1074}$**
* Let $b$ be the next subnormal value up, but negative:
**$b = -2 \times 2^{-1074}$** (which is **$-2^{-1073}$**)

### 2. The Exact Mathematical Sum

If we add them together mathematically:


$$a + b = 2^{-1074} - (2 \times 2^{-1074})$$

$$a + b = -2^{-1074}$$

### 3. The Floating-Point Sum ($s$)

The value **$-2^{-1074}$** is exactly representable in IEEE 754. It is simply the negative version of the smallest subnormal number.

Because the exact mathematical sum lands perfectly on a representable floating-point number, the machine performs no rounding.

* Computed sum: **$s = -2^{-1074}$**
* Actual error: **$e = 0$**

### 4. Running the TwoSum Algorithm

If we run the standard Knuth TwoSum algorithm ($s = a + b$, $a' = s - b$, $b' = s - a'$, $\delta_a = a - a'$, $\delta_b = b - b'$, $e = \delta_a + \delta_b$) on this exact test case:

1. $s = \text{fl}(2^{-1074} - 2^{-1073}) = -2^{-1074}$
2. $a' = \text{fl}(-2^{-1074} - (-2^{-1073})) = 2^{-1074}$
3. $b' = \text{fl}(-2^{-1074} - 2^{-1074}) = -2^{-1073}$
4. $\delta_a = \text{fl}(2^{-1074} - 2^{-1074}) = 0$
5. $\delta_b = \text{fl}(-2^{-1073} - (-2^{-1073})) = 0$
6. $e = \text{fl}(0 + 0) = 0$

TwoSum perfectly returns $s = -2^{-1074}$ and $e = 0$. Since $0$ is a representable floating-point number, the TwoSum property ($a + b = s + e$) is preserved perfectly.

---

### The Secret of Subnormals

The reason addition and subtraction can never generate an unrepresentable error in the subnormal range is that **subnormal floating-point numbers act exactly like fixed-point integers**.

In the normal range, the spacing between floating-point numbers scales up and down with the exponent. But in the subnormal range, the exponent is frozen at its absolute minimum. The spacing between *every single subnormal number* is identical: exactly $2^{-1074}$.

Because the grid spacing is constant, adding or subtracting any two subnormals is just like adding or subtracting integers. The result will always land perfectly on another grid point (assuming no overflow into the normal range). Therefore, the error $e$ for *any* subnormal addition or subtraction is always exactly $0$.

## Interval arithmetic and smallest containment interval

Interval arithmetic avoids the `TwoProduct` underflow trap entirely because it uses a completely different mathematical mechanism to guarantee containment: **directed rounding**.

Instead of trying to compute an exact result and an exact error term ($a \times b = p + e$), interval arithmetic changes the rules of floating-point rounding so that the hardware itself pushes the boundaries outward to the nearest representable numbers.

Here is exactly how interval arithmetic handles the subnormal underflow gap to create the tightest possible containment.

## The Mechanism: Directed Rounding

Standard IEEE 754 arithmetic uses "Round to Nearest, ties to Even" (RN). Interval arithmetic instead computes the lower bound using **Round Toward Negative Infinity (RD)** and the upper bound using **Round Toward Positive Infinity (RU)**.

Let’s revisit our failing `TwoProduct` example to see how directed rounding creates perfect containment.

* Let $a = 2^{-1074}$ (the smallest subnormal).
* Let $b = 0.5$.
* The exact mathematical product is $2^{-1075}$.

### 1. Computing the Lower Bound

When the CPU calculates the lower bound, it uses Round Toward Negative Infinity.
The exact product $2^{-1075}$ sits exactly between **$0$** and **$2^{-1074}$**. Because the CPU is instructed to round *down*, it selects the lower of the two bounding floating-point numbers.

* Lower bound = **$0$**

### 2. Computing the Upper Bound

When the CPU calculates the upper bound, it uses Round Toward Positive Infinity.
Again, the exact product $2^{-1075}$ sits between **$0$** and **$2^{-1074}$**. Because the CPU is instructed to round *up*, it selects the higher of the two bounding floating-point numbers.

* Upper bound = **$2^{-1074}$**

### 3. The Resulting Interval

The resulting interval for $a \times b$ is **$[0, 2^{-1074}]$**.

Does this contain the exact true mathematical answer? Yes, because $0 < 2^{-1075} < 2^{-1074}$.

Is it the "smallest possible containment"? Yes. In double precision, there are zero representable numbers between $0$ and $2^{-1074}$. The lower and upper bounds are exactly one Unit in the Last Place (ULP) apart. You physically cannot construct a tighter bounding box in this format.

## Modern Software Workarounds (The FMA trick)

While changing hardware rounding modes mathematically solves the problem, doing so in code (e.g., calling `fesetround()`) empties the CPU pipeline and severely hurts performance.

To keep speeds fast, many modern interval libraries stay in default "Round to Nearest" mode and use Fused Multiply-Add (FMA) to compute the bounds. Because of the `TwoProduct` subnormal failure we discussed, these libraries have to implement specific safety nets:

1. They compute $p = a \times b$ in round-to-nearest.
2. They use FMA to compute the error: $e = \text{fma}(a, b, -p)$.
3. They use $p$ and $e$ to adjust the interval endpoints.
4. **The Subnormal Safeguard:** The library explicitly checks if $p$ is in the subnormal range (or zero). If it is, the code knows $e$ might have underflowed and silently failed. To guarantee containment, the library artificially widens the interval by adding/subtracting **$2^{-1074}$** to the bounds.

Whether using hardware directed rounding or software FMA adjustments, interval arithmetic sacrifices *exactness* in favor of *containment*. By stepping outward to the next available floating-point grid lines, it never has to worry about storing the unrepresentable subnormal gap.