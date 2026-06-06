# elreal: converging on online (lazy) arithmetic, and removing the eager scaffolding

## TL;DR

LFPERA (McCleeary's Lazy Floating-Point Exact Real Arithmetic) is **online by
definition**. There is no "eager mode" in the mathematics. The eager,
depth-budgeted `mul`/`div`/`sum` in `elreal` are implementation scaffolding from
the incremental build-out, not part of the design. They are now **deprecated**
and slated for removal in favor of the streaming `mul_online`/`div_online`/
`infsum`. This note records why the eager path exists, why it is the wrong
long-term shape, and the concrete steps to delete it.

## LFPERA is online; there is no eager mode

The dissertation (`lfpera.pdf`) is unambiguous. A full-text scan finds **zero**
occurrences of `eager`, `depth`, `budget`, `bounded precision`, or
`finite precision`, against 30 of `lazy` and 13 of `co-list`. The title is
*Lazy* Floating Point Exact Real Arithmetic. Reals are "lazy lists of floating
point numbers"; arithmetic is defined on "possibly infinite" co-lists (ZBCL)
that emit one `block` at a time on demand, with "no guarantee of reaching the
least significant bits since they are infinite lists." `infSum` "takes in a
possibly infinite list."

The only place a computation is bounded is at the **consumer**: a caller asks for
`take N` blocks (N digits), and that demand pulls exactly as much work as needed
up the operand chain. That pull *is* the laziness. It is not an internal
precision budget threaded through the operators.

## Where the eager path came from

The eager operators are placeholders, and the headers say so in their own words:

- `multiply.hpp`: "Phase 6 model: FINITE-PREFIX multiplication ... a fully
  streaming product is deferred."
- `sum.hpp`: "This makes sum() eager over the budgeted prefix ... a future
  fully-lazy summation would need a Phase 4 add() that [preserves 0-overlap]."
  -- i.e. eager `sum` was a **workaround for the `add()` lazy-composition
  0-overlap bug**, which is now fixed (#1057).
- `divide.hpp`: depth-budgeted long-division refinement.

So `depth` exists because, in Phases 5-6, the streaming versions were not yet
correct (chiefly the lazy-tower 0-overlap bug). That reason is gone for sum and
multiply; for divide it is partially gone (single-block and sparse multi-block
divisors stream correctly; general dense multi-block division is still open).

## Why two modes is the wrong shape

- It is not LFPERA. `depth` truncates *intermediate operands* prematurely, which
  the lazy design never does -- bounding belongs at the output, driven by the
  consumer's `take`.
- It muddles the codebase: the math suite (`constants`, `exponent`,
  `trigonometry`, `sqrt`, `hypot`, `hyperbolic`) threads a `depth` argument
  through every `mul`/`div`/`sum` call, encoding an internal precision budget
  that the online design replaces with pull-driven evaluation.
- It invites treating "eager vs online" as a legitimate design choice. It is not.
  Online is the design; eager is debt.

## Current state

| operator | canonical (online) | status |
|----------|--------------------|--------|
| infinite sum | `infsum` / `infinitesum` (infsum.hpp) | complete; `== sum()` validated |
| multiply | `mul_online` (online_multiply.hpp) | complete; exact-product validated |
| divide | `div_online` (online_divide.hpp) | complete for single-block + sparse (power-of-two) multi-block divisors; **general dense multi-block open** |

Validated by `el_arith_online_muldiv`. The wide block exponent (`integer<256>`,
#1066) was the prerequisite that let streaming division reach full depth without
int32 overflow.

## Removal plan

1. **Finish streaming division for general dense multi-block divisors.** Two open
   problems the int32 overflow used to mask: a 0-overlap correctness bug in the
   quotient fold, and a cost explosion (the running divisor `g0*divisor` grows in
   block count per level). Needs an algorithm fix that keeps the running divisor
   sparse / bounds its block count and carry-arrests the fold. (Tracked separately
   as the hard remaining phase-1 item.) This is the only blocker that is not just
   a mechanical migration.

2. **Relocate the shared helper `zbcl_from_blocks` out of `sum.hpp`.** It is not
   eager (it just builds a ZBCL from a block vector) but it currently lives in the
   deprecated header and `online_divide.hpp` depends on it. Move it to
   `zbcl_helpers.hpp` so `sum.hpp` can be deleted cleanly.

3. **Migrate the math suite to the streaming ops.** Replace
   `mul(x,y,depth)` -> `mul_online(x,y)`, `div(x,y,depth)` -> `div_online(x,y)`,
   `sum(series,depth)` -> `infsum(series)`, and let the consumer drive precision
   with `take`. Migration caveat: the math series were *tuned against eager's
   depth-budget cost/precision profile*, so each function needs re-validation of
   result precision and run time after the switch (the high-precision
   constants/exponent tests already run ~100 s). Do this function by function,
   keeping the regression suite green.

4. **Delete the eager bodies and the `depth` parameters.** Remove
   `multiply.hpp`'s `mul`, `divide.hpp`'s `div`, and `sum.hpp` once no caller
   remains. `mul_scalar` (a single block times a stream) is lazy and stays.

## Acceptance

The end state has one arithmetic per operation, all online, with precision
controlled solely by the consumer's `take`. No `depth` parameter survives on any
`elreal` arithmetic entry point. The dissertation's design and the implementation
match one-to-one.
