# elreal: Lazy Incremental Precision -- Current State, the Eager-Batch Regression, and the Path Back to Online mul/div

Status: design note (no code change). Written while investigating #1058
(exp/sin/cos precision cap) and #1049 (transcendental hardening). Establishes
why a localized #1058 patch is the wrong fix and scopes the architectural work
that actually closes it.

## 1. The central requirement

elreal (the McCleeary LFPERA -- Lazy Faithful Precision-Extensible Real
Arithmetic) exists to do one thing the classical Priest/Shewchuk expansion model
cannot: **deliver precision incrementally, on demand, reusing prior work.**

The intended interaction is:

```cpp
  q = a / b            // suspend a division; compute nothing yet
  q.take(2)            // force two limbs (~32 digits); inspect
  q.take(3)            // force ONE more limb -- the third -- reusing limbs 1-2
```

Each additional limb costs only the increment, never a recomputation. This is
the whole reason for (a) a lazy evaluator over a co-list representation and
(b) solving the hard sub-problem of **arresting the carry/borrow**: in an online
producer you cannot commit an output limb until you have looked far enough into
the operands to know no lower-order carry or borrow will still perturb it. Priest
and Shewchuk sidestep this by being eager and fixed-length; McCleeary's
contribution is making it lazy and precision-extensible, which forces the
carry/borrow-arrest problem to be solved head-on.

This note documents where that property currently holds, where it was lost, and
what it would take to restore it.

## 2. What the code does today (audit)

### 2.1 The data type is lazy (the spine is intact)

`ZBCL<FpType>` (`include/sw/universal/number/elreal/zbcl.hpp:48-145`) is a genuine
lazy co-list:

- A non-empty co-list is `(head block, tail thunk)`, the thunk a
  `std::function<ZBCL()>` (zbcl.hpp:52, 55-66).
- `tail()` forces the thunk on first call and **memoizes** the result on the node
  (zbcl.hpp:106-124); `shared_ptr` nodes make values cheap to copy.
- `take(n)` deliberately does not force the (n+1)-th thunk -- `take(1)` on a
  multi-block stream touches only the first block (zbcl.hpp:129-144).

So at the representation level the laziness, memoization, and sharing McCleeary
specifies are all present.

### 2.2 add() is genuinely online (the requirement is honored here)

`add(x, y)` (`threeAdd.hpp:666-683`) is the dissertation's online add
(Algorithm 4.2.1):

- It carries a streaming state machine `addRec_state` / `addRec_step` and returns
  `cons(block, [loop]{ ... })` -- **one limb produced per `tail()` pull**, with the
  continuation resuming the state machine (threeAdd.hpp:673-682).
- It takes **no `depth` parameter.** Precision is whatever the consumer pulls.
- `addRec_step` is exactly the carry/borrow arrest: it will not emit a limb until
  the streaming state resolves whether a lower-order term can still change it.

This is the model. It is the only operation that implements it.

### 2.3 mul / div / sum / transcendentals are eager batch evaluators

Every other operation abandoned the online model for an eager, `depth`-budgeted
batch:

- `mul(x, y, depth)` (`multiply.hpp:57-73`): `x.take(depth)`, `y.take(depth)` ->
  build the full product pool -> `priestRenorm(pool)` -> `zbcl_from_blocks(...)`.
- `div(x, y, depth)` (`divide.hpp:37-118`): `x.take(depth)`, `y.take(depth)` ->
  eager long-division loop bounded by `depth` (divide.hpp:92-115) ->
  `zbcl_from_blocks(priestRenorm(qblocks))`.
- `mul_scalar` (`multiply.hpp:37`), `sum`/series, and all transcendentals
  (`math/exponent.hpp`, `math/trigonometry.hpp`, `math/hyperbolic.hpp`) inherit
  this shape.
- `zbcl_from_blocks` (`sum.hpp:75-82`) conses an **already-fully-materialized**
  `std::vector<block>` into a spine. The result's "laziness" is cosmetic: every
  limb is already computed; the thunks just hand back precomputed sublists.

Two consequences follow directly:

1. **`depth` is an eager up-front budget, not a pull count.** `div(a, b, 2)` then
   `div(a, b, 3)` recomputes the whole quotient -- the Priest/Shewchuk eager model
   elreal exists to escape.
2. **The transcendentals additionally lost memoization.** `exp(x, 4)` then
   `exp(x, 20)` re-runs the entire Taylor series from term 0.

### 2.4 Summary table

| layer | lazy co-list spine | online (pull-driven, carry-arrested) | memoized resumption |
|---|---|---|---|
| `ZBCL` type | yes | n/a | yes (`tail()`) |
| `add` | yes | **yes** (`addRec_step`) | yes |
| `mul`, `div` | spine only | no -- eager `take(depth)` + `priestRenorm` | no |
| `sum`, series | spine only | no | no |
| `exp/sin/cos/...` | spine only | no | no (recompute per call) |

The incremental-precision requirement is honored **only for addition** today.

## 3. Why it got lost: the carry-arrest was sidestepped, not solved

The "arrest the carry/borrow" problem is genuinely hard for multiplication and
division done online. To commit limb `i` of a product or quotient you must look
far enough down both operands to be sure no carry/borrow will still alter it; the
0-overlap (k-bit-gap) invariant bounds that lookahead, but the producer state
machine for mul and div is substantially more involved than for add/sub.

The implementation took the tractable route: materialize the operands to a finite
`depth`, compute the entire result, and let a single global `priestRenorm` pass
(`threeAdd.hpp:366-436`) resolve all carries at once. That is **correct** and easy
to prove per operation (value preserved + 0-overlap), which fits the phased
"proofs live with each operation as it is introduced" plan stated in
zbcl.hpp:28-30.

But batch `priestRenorm` *is* the carry-arrest done the eager way: compute
everything, then normalize. It buys soundness at the cost of incrementality and
memoized resumption. For `add` the carry was arrested **online**, limb by limb;
for `mul`/`div` the online arrest was never written -- it was replaced by
"compute `depth` limbs and renormalize the lot." That substitution, repeated at
the `sum`/series/transcendental layers, is where the central requirement was lost.

## 4. Symptoms this explains

### 4.1 #1058: the exp/sin/cos precision cap and its cost wall

- `exp`/`sin_series`/`cos_series` scale each Taylor term by a **host-double**
  reciprocal coefficient (`exponent.hpp:63`, `trigonometry.hpp:126,142`). That is
  invisible in eager low-`depth` mode but is a **hard ceiling** (~17 digits) an
  incremental consumer would hit at limb ~2.
- Fixing it the eager way (mirror `e_zbcl`: full-precision `div` per term + a
  `kSeriesGuard` working-depth) is correct (309 digits) but regresses the default
  `depth=4` path catastrophically, because each precision request re-runs ~75
  series terms with per-term general `div()`:

  | `exp(0.5, depth)` | eager host-double coeff (today) | eager full-precision `div` | factor |
  |---|---|---|---|
  | 2 | 0.09 ms | 15 ms | 163x |
  | 4 (default) | 1.5 ms | 114 ms | 75x |
  | 8 | 127 ms | 1127 ms | 8.9x |

  The regression is the eager-batch model surfacing: there is no resumption to
  amortize a precision increase against, so "more digits" means "redo everything,
  bigger."

### 4.2 #1049: which transcendentals reach 300 digits, and why

In the #1049 hardening suite, `log`/`atan`/`asin`/`acos` reach 307-320 digits
because they route through `odd_power_series`, which divides by the integer
denominator at full precision; `exp`/`sin`/`cos`/`tan`/`sinh`/`cosh`/`tanh` cap at
~17 digits because of the host-double coefficient. Both groups are still **eager**;
the split is only about the coefficient. The cost wall in 4.1 is what makes the
eager full-precision coefficient unshippable for the hot path -- which is the
signal that the fix belongs at the architecture level, not the coefficient level.

## 5. Target design: pull-based precision over memoized online operators

### 5.1 `depth` becomes a pull count, not a budget

The consumer-facing precision control should be `take(n)` / a limb iterator on a
**suspended** computation, exactly as `add` already exposes. The integer-`depth`
overloads remain as convenience wrappers (`take(n)` under the hood) for callers
that genuinely want N components in one shot, but they stop being the primitive.

```cpp
  ZBCL<double> q = div(a, b);      // suspended; no work yet (like add today)
  q.take(2);                       // force 2 limbs
  q.take(3);                       // force limb 3 only, limbs 1-2 memoized
```

`div`/`mul` lose their eager `depth` parameter the way `add` already has none;
precision is whatever is pulled.

### 5.2 Online mul / div with carry-arrest

`mul` and `div` must become streaming state machines in the shape of
`addRec_step`:

- Maintain operand cursors that pull operand limbs lazily as needed.
- Maintain the partial product/quotient and the residual carry/borrow front.
- Emit an output limb only once the 0-overlap lookahead guarantees no
  lower-order carry/borrow can still perturb it (the arrest condition), then
  `cons(limb, continuation)`.

`add`'s `addRec_state`/`addRec_step` (`threeAdd.hpp`) is the template: a small,
testable state record plus a step function that either emits the next limb or
advances internal state. The division long-division loop in `divide.hpp:92-115`
already has the right *shape* (one quotient limb per step, residual reduced by
`q_i * y`); the work is to make each step (a) pull operand limbs on demand instead
of `take(depth)` up front, (b) carry the residual as persistent state across
`tail()` pulls instead of a local vector, and (c) commit a quotient limb only when
arrested rather than running to a `depth` bound.

### 5.3 Resumable transcendental series

With online `mul`/`div`, `exp`/`sin`/`cos` become **resumable series**: the term
recurrence (`term`, running `power`, partial `sum`, series index `n`) is captured
in a state object; pulling another output limb advances the series only as far as
needed and memoizes it. The host-double coefficient bug disappears for free,
because each term's `/n` is an online full-precision division that the consumer
pays for incrementally rather than a host-double shortcut taken to stay cheap in
eager mode.

### 5.4 What stays

`block`, the EFTs (`block_two_mult`, `block_two_div_rn`, `block_two_sum`),
`priestRenorm`, the 0-overlap invariant, and `add` are unchanged. This is a
restructuring of `mul`/`div`/`sum`/transcendentals into online producers over the
same numerical kernels, not a rewrite of the kernels.

## 6. Phasing

1. **Online `mul`** as an `addRec`-style state machine; validate value + 0-overlap
   against the current eager `mul` across the existing arithmetic suite, and add a
   laziness test (pull limb k+1, assert limbs 1..k are not recomputed and the
   result matches eager `mul(.,., k+1)`).
2. **Online `div`** likewise, reusing the long-division step but with persistent
   residual state and the arrest condition; validate against eager `div` and the
   #1022 exact-dyadic oracle.
3. **Resumable series** for `exp`/`sin`/`cos`; re-enable the #1049 gated checks
   (`ELREAL_EXP_SERIES_HIGH_PRECISION`) and confirm 300+ digits with the cost now
   paid incrementally. Closes #1058.
4. Keep the integer-`depth` overloads as `take(n)` wrappers throughout for source
   compatibility.

## 7. Risks and open questions

- **Carry-arrest correctness for x and /.** The hard part. Needs the dissertation's
  online-mul/div argument and a proof obligation per operation mirroring
  zbcl.hpp:22-30; an adversarial cancellation test set (the #1057 exact-cancel case
  is one) should accompany it.
- **Cost profile of online vs batch at fixed precision.** Online production trades
  the batch `priestRenorm` for per-limb arrest bookkeeping. It should win decisively
  on *incremental* workloads (the point) and be competitive on a single eager
  `take(N)`; this must be measured, not assumed.
- **Interaction with #1057.** `add()` not renormalizing an exact leading-term
  cancellation to 0-overlap (asin(x)+acos(x)) is in the online `add` itself; it
  should be resolved in or alongside this work, since online `mul`/`div` build on
  the same arrest machinery.
- **Default-precision expectations.** Today `depth=4` is "fast, ~17 digits". Under a
  pull model the analogous default is "pull until the consumer stops"; library
  defaults and the transcendental wrappers' default pull counts need a deliberate
  choice so existing callers' cost does not silently change.

## 8. References

- McCleeary LFPERA dissertation, as cited in the headers: Cauchy-sequence
  obligation 4.1.4 (zbcl.hpp:22), add Algorithm 4.2.1 (threeAdd.hpp:666),
  sum 4.2.3 (sum.hpp:84), division 4.2.6 (divide.hpp:1).
- Code: `zbcl.hpp` (lazy co-list), `threeAdd.hpp` (online `add`, `priestRenorm`),
  `multiply.hpp`, `divide.hpp`, `sum.hpp`, `math/exponent.hpp`,
  `math/trigonometry.hpp`.
- Issues: #1058 (exp/sin/cos precision cap -- this note is its real scoping),
  #1049 (transcendental hardening that surfaced the cap), #1057 (add exact-cancel
  0-overlap), #1044/#1046 (the octant-reduction renorm-pool workaround, a prior
  instance of eager carry handling around `add`).
