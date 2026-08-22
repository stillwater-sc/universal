# elreal demonstrations

Programs that show what the `elreal` adaptive-precision type does, by running and
printing rather than asserting. They are **not** registered with ctest (the
`CMakeLists.txt` here passes `"false"` to `compile_all`) because they are
demonstrations and some take tens of seconds. They are still built, so they cannot
silently rot.

| Program | What it shows | Runtime |
|---------|---------------|---------|
| `thousand_digit_sqrt` | One Newton kernel, three host types: native `double` stalls at 16 digits, `elreal<float>` and `elreal<double>` both pass 1150 | ~35s |

## thousand_digit_sqrt

The claim: for `elreal` the host floating-point type is the **size of a limb**, not a
ceiling on precision. A block is `(v: FpType, exp)` with the exponent in a wide
integer, so an expansion's reach is a budget you choose rather than a property of
`FpType`.

The kernel is ordinary templated numeric code with no `elreal` in it:

```cpp
template <typename Real>
Real newton_sqrt(const Real& a, int iterations) {
    Real x = a / Real(2.0);
    for (int i = 0; i < iterations; ++i) x = (x + a / x) / Real(2.0);
    return x;
}
```

Instantiated on `double` it stops at 16 digits no matter how many iterations it is
given. Instantiated on `elreal<float>` and `elreal<double>` it converges to the same
1150+ decimal digits of `sqrt(2)`, verified against a reference computed in exact
integer arithmetic outside the library. Float needs about 2.2x the blocks for the
same digits, its limb carrying 24 bits against double's 53 -- that is the only
difference the host type makes to the answer.

Part 3 shows the one place the demo uses an `elreal`-specific API: Newton doubles the
number of correct *bits* per step, so ramping `precision()` alongside the iterate
instead of paying the final precision from the first step costs **4s and 25s** rather
than **25s and 206s**.

### Why this was not possible before

Division by a multi-block value used to cap at ~271 decimal digits on a double host
and ~22 on float, so the Newton step could not have worked at all. Three defects had
to go: the operand-normalisation rule in v4.9.0 (#1362), a host-derived depth constant
in `div_online` (#1371), and a streaming-summation branch that dropped a term whenever
the accumulator cancelled to zero (#1373).
