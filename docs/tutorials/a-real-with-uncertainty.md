# A real with uncertainty bit

In the "The End of Error", John Gustafson introduces a floating-point type with an uncertainty bit so that we have information about exact or approximated values. Here are some canonical examples that demonstrate what ubit arithmetic can do that IEEE floats cannot:

---
## Key Examples from the Book

  1. Rump's Royal Pain (Chapter 14)

  f(a,b) = 333.75b⁶ + a²(11a²b² - b⁶ - 121b⁴ - 2) + 5.5b⁸ + a/(2b)
  a = 77617, b = 33096
  - IEEE result (single, double, AND quad): ≈ 1.18 × 10²¹
  - Correct answer: −0.827396059946821...
  - The problem: IEEE floats don't even get the sign right, off by 21 orders of magnitude
  - Unum advantage: With ubit, you get either the correct answer OR an interval that tells you "I don't know" rather than a confidently wrong answer

  2. Muller's Converging Sequence

  v₁ = 2, v₂ = -4
  vₙ = 111 - 1130/vₙ₋₁ + 3000/(vₙ₋₁ × vₙ₋₂)
  - IEEE result: Converges to 100
  - Correct answer: Converges to 6
  - Unum advantage: The ubit would indicate growing uncertainty, warning you the result is unreliable

  3. The Chaotic Bank Society

  Balance₀ = e - 1
  Balanceₙ = Balanceₙ₋₁ × n - 1  (for 25 years)
  - IEEE result: Goes negative (impossible for a bank balance)
  - Correct answer: ≈ $0.0399...
  - Unum advantage: Tracks uncertainty propagation through iterations

  4. Bailey's Numerical Nightmare (Chapter 14)

  A system of two equations in two unknowns that looks innocent but causes IEEE to fail due to:
  - Conversion of decimal inputs to binary
  - Inadequate precision for intermediate results

  5. The Quadratic Formula (Chapter 14)

  Shows that unums handle the formula naturally without needing algebraic rearrangement to avoid catastrophic cancellation in b² - 4ac.

  6. Pendulum/Two-Body Problem (Chapters 19-20)

  Differential equation solving where IEEE silently accumulates errors while ubit-based intervals provide guaranteed bounds on the solution.

  ---

## What Ubit Can Do That IEEE Cannot

|            Capability            |         IEEE Float         |        Areal with Ubit         |
|----------------------------------|----------------------------|--------------------------------|
| Detect precision loss            | Silent rounding            | ubit=1 warns you               |
| Distinguish exact vs approximate | Everything looks the same  | ubit=0 means exact             |
| Provide guaranteed bounds        | Point estimate only        | Interval contains true value   |
| Avoid confident wrong answers    | Returns 1.18×10²¹ for Rump | Returns "uncertain" or correct |
| Track error accumulation         | Hidden                     | Explicit via ubit propagation  |

  ---
## Suggested Test Cases for areal<>

Based on this research, here are 6 examples that would truly demonstrate areal's ubit functionality:

  1. Rump's polynomial - Show IEEE gets wrong sign, areal flags uncertainty or gets correct answer
  2. Muller's recurrence - Show ubit growing with iterations, warning of instability
  3. Chaotic bank - Track uncertainty through 25 iterations
  4. Quadratic with near-zero discriminant - Show graceful degradation vs catastrophic cancellation
  5. Kahan's thin triangle - Area calculation where IEEE fails
  6. Newton iteration convergence - Show ubit stabilizing when converged vs growing when diverging

  The current tests compare error magnitude between areal and cfloat. The real demonstration should be: "IEEE gave a confident wrong answer; areal correctly
  indicated uncertainty."

  ---
  Sources:
  - https://rosettacode.org/wiki/Pathological_floating_point_problems
  - https://www.johndcook.com/blog/2019/11/12/rump-floating-point/
  - https://archive.org/details/endoferrorunumco0000gust
  - https://ubiquity.acm.org/article.cfm?id=2913029
  - https://people.eecs.berkeley.edu/~wkahan/EndErErs.pdf

