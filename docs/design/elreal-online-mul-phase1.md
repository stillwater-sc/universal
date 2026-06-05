# elreal #1061 Phase 1: Online (pull-driven) Multiplication -- Scoping Note

Status: scoping / design (no code change). Phase 1 of #1061 (online mul/div with
carry-arrest). Companion to `elreal-lazy-incremental-precision.md` (the architectural
design note, #1060). This note specifies what online `mul` must do, the algorithm
shape, the genuinely hard part (the multiplicative carry-arrest), the test plan, and
the external dependency that gates implementation.

## 0. Context and the deliberate placeholder

The current `mul` (`multiply.hpp:55-75`) is, by its own header, a **"Phase 6 FINITE-
PREFIX"** stand-in: it materialises `x.take(depth)` and `y.take(depth)`, forms every
pair product, and `priestRenorm`s the lot. The header explicitly states *"a fully
streaming anti-diagonal product is deferred to the Phase 7 math suite"* and cites
**dissertation 4.2.5** for the real algorithm. So online mul is a known, intended
completion, not a redesign.

`add()` already demonstrates the online model end to end (`threeAdd.hpp:666-683`,
`addRec_state`/`addRec_step`/`isSafe`). Phase 1 is to do for `mul` what `add()` does:
produce one output limb per `tail()` pull, pulling operand limbs on demand, emitting a
limb only when carry-arrest guarantees no future term can perturb it.

This is the prerequisite that makes high-precision composite computations tractable;
euler_gamma (#1053) is the motivating benchmark (123s of its 200s at depth 16 is eager
`mul` recomputing full precision per term and discarding most of it).

## 1. Value specification (unchanged)

```
x * y = sum_{i,j} (x_i * y_j)
```
where each `x_i * y_j` is the exact 2-block decomposition `block_two_mult(x_i, y_j)`
(`block_eft.hpp`). The online result must be the unique 0-overlap ZBCL equal to that
double sum, produced limb-by-limb on demand. Both the eager `mul` and the online `mul`
must agree exactly; the eager version is the equivalence oracle (section 5).

## 2. Why mul is harder than add: 1D merge vs 2D convolution

`add`'s carry-arrest (`isSafe`, `threeAdd.hpp:481-484`) emits `out` once the next
operand blocks `f, g` are `2k+3` below it:
```
expGreaterBy(2k+3, out, f) && expGreaterBy(2k+3, out, g) && !expGreater(prev, out) && expGreaterBy(k, out, e)
```
This fixed 1-block-per-operand lookahead is sufficient because each operand is itself
0-overlap: the entire remaining tail of `fs` is geometrically bounded by its leading
block (`|tail beyond f| < 2^{E(f)+1-k}/(1-2^-k) ~ 2^{E(f)+2-k}`). So if `f` is `2k+3`
below `out`, no future addition can carry into `out`.

For `mul` the contributors to a given output exponent form a **2D region** of the
`(i,j)` grid, not a 1D stream:
- Pair product `x_i * y_j` sits at exponent `~ E(x_i) + E(y_j)` (two blocks, spanning
  `[E(x_i)+E(y_j)-k+1 , E(x_i)+E(y_j)]` roughly).
- Output limb at exponent `E` receives contributions from every `(i,j)` with
  `E(x_i)+E(y_j) >= E - O(k)`.
- The count of such pairs grows with depth (an anti-diagonal band), so there is no
  fixed-size lookahead; the arrest must bound a 2D tail.

Two sub-problems follow: (A) generate pair products in ~descending exponent order, and
(B) decide when an accumulated output limb is safe against the unconsumed 2D tail.

## 3. Sub-problem A: anti-diagonal term generation

Because `x_i` and `y_j` are each strictly descending (`E(x_{i+1}) <= E(x_i) - k`), the
pair exponents `E(x_i)+E(y_j)` are monotone non-increasing along rows and columns. The
maximal unconsumed pair exponent lives on a **frontier** of the grid (the staircase
between consumed and unconsumed cells). Standard approach:

- Maintain a frontier as a priority queue keyed by `E(x_i)+E(y_j)` (max-heap).
- Seed with `(0,0)`. On popping `(i,j)`, push its grid successors `(i+1,j)` and
  `(i,j+1)` (with the usual dedup so each cell enters once -- e.g. only push `(i+1,j)`
  when `j==0`, and always `(i,j+1)`).
- Pulling `x_i`/`y_j` for a newly reachable cell forces that operand's `tail()` lazily
  -- this is where operand limbs are pulled on demand.
- Emit `block_two_mult(x_i, y_j)` (2 blocks) into the accumulation workspace.

The frontier's current maximum pair exponent is the analog of `add`'s "next operand
block exponent": it bounds the most significant unconsumed contribution. The arrest
(sub-problem B) is stated against that frontier maximum.

Note the band, not just the diagonal: cells with comparable `E(x_i)+E(y_j)` can lie off
a single anti-diagonal when the operand block gaps exceed `k`. The priority queue
handles this directly (it orders by exponent, not by `i+j`).

## 4. Sub-problem B: the multiplicative carry-arrest (the hard, novel part)

Let `Phi = max over frontier of (E(x_i)+E(y_j))` be the largest unconsumed pair
exponent after some prefix of the frontier has been consumed. The total value of all
unconsumed pair products is bounded by summing the geometric tails in both dimensions;
to first order it is `O(2^{Phi + c})` for a small constant `c` that depends on `k` and
the frontier shape. A candidate output limb `out` is safe to emit when the unconsumed
tail cannot reach `out`'s ulp, i.e. (schematically)
```
expGreaterBy(M(k), out, frontier_max_block)  && !expGreater(prev, out) && expGreaterBy(k, out, e)
```
for a margin `M(k)` that is the multiplicative analog of add's `2k+3`. Deriving `M(k)`
correctly is the crux of phase 1: too small emits a limb a later term perturbs (wrong
digits); too large stalls or never emits (non-termination). The bound must account for:
- both block sub-products of each `block_two_mult` (the low block sits `~k` below),
- the geometric pile-up of many same-magnitude pair products on a band (a factor that
  grows with band width, unlike add's two fixed operands),
- the workspace `priestAdd` already in flight.

**Do not derive `M(k)` from scratch and hope.** Dissertation 4.2.5 specifies the
streaming product; `add`'s `isSafe`/`addRec` were *translated verbatim* from FCL.hs
(`threeAdd.hpp:3,517`), not reinvented. Phase 1 step 0 is to obtain the dissertation's
streaming-mul (and FCL.hs `mulRec` if it exists) and translate its safety predicate,
exactly as add was. **FCL.hs is not in this repo** (only referenced in comments); it
must be sourced from the McCleeary 2019 dissertation materials.

## 5. State machine design (mirrors addRec)

```
struct mulRec_state<FpType> {
    ZBCL<FpType> xs, ys;                 // operands; pulled lazily via head()/tail()
    std::vector<block> x_seen, y_seen;   // materialised prefixes the frontier indexes
    PriorityFrontier frontier;           // (i,j) cells ordered by E(x_i)+E(y_j)
    std::vector<block> workspace;        // priest-renormalised pending output (front-first)
    std::int32_t bound;                  // exponent frontier, as in addRec
    bool initialised;
};

std::optional<block> mulRec_step(mulRec_state&);   // emit next limb or nullopt
ZBCL<FpType> mul(ZBCL<FpType> x, ZBCL<FpType> y);  // cons(first, thunk{loop}), NO depth param
```

`mulRec_step` loop, per the add template:
1. Pop frontier cells whose pair exponent is at/above the current `bound`; for each,
   pull the needed `x_i`/`y_j` (extending `x_seen`/`y_seen` via `tail()`), compute
   `block_two_mult`, and `priestAdd` its two blocks into `workspace`. Push grid
   successors.
2. Take the workspace leading block as candidate `out`. Test the mul `isSafe` (section
   4) against `frontier_max`, `prev`, and the workspace tail.
3. If safe: emit `out`, set `bound = E(out) - k`, drop `out` from workspace. If not:
   keep consuming frontier cells (step 1) until safe or operands exhausted.
4. Termination: when both operands and the frontier are exhausted, drain the workspace
   (finite product = exact). Carry add's `safety_counter` backstop.

`mul()` itself is the same `cons(first, [loop]{...})` wrapper as `add()`
(`threeAdd.hpp:673-682`); the `take(n)` / integer-`depth` form becomes a thin wrapper
that pulls `n` limbs.

Reused verbatim: `block_two_mult`, `twoSumRN`/`threeAdd`/`priestAdd`/`priestRenorm`,
`expGreater`/`expGreaterBy`, the cons+thunk wrapper, the `safety_counter` guard.

## 6. Test strategy

1. **Equivalence vs eager mul** -- for finite operands, `online_mul(x,y).take(N)` must
   equal `mul_eager(x,y,N)` (current `mul`) for N up to each operand's length, across
   the existing arithmetic-suite inputs plus randomised multi-block ZBCLs. Eager mul is
   the exact oracle for finite operands.
2. **Exact value** -- against the dyadic oracle (`elreal_oracle.hpp`,
   `agreed_decimal_digits`) for products of constants/known values.
3. **0-overlap invariant** -- `check_zero_overlap` on the streamed output at several N.
4. **Laziness / memoization (the whole point)** -- instrument operand pull counts;
   assert: (a) `take(k)` forces only the operand limbs the frontier needed for k output
   limbs (not all of them); (b) `take(k)` then `take(k+1)` forces strictly fewer
   additional operand pulls than recomputing `take(k+1)` from scratch, and reuses the
   memoised prefix (the ZBCL tail memoisation gives this for free if the state threads
   through the thunk correctly). This is the property that makes #1053 tractable.
5. **Cost** -- `online.take(N)` competitive with `mul_eager(.,.,N)` on a single shot;
   incremental refinement `take(k)`->`take(k+1)` strictly cheaper than recompute. Use
   the #1040 characterization tooling once available.
6. **Adversarial** -- operands with block gaps `> k` (off-diagonal bands), near-equal
   magnitudes (frontier ties), and a deep operand x a short one.

## 7. Risks and open questions

- **The margin `M(k)` derivation is the make-or-break.** Must come from dissertation
  4.2.5 / FCL.hs, not be guessed. Sourcing those materials is step 0 and blocks the
  rest.
- **Frontier management cost.** The priority queue adds per-pair overhead; it must not
  dominate. An array-of-active-rows staircase may beat a generic heap.
- **#1057 interaction.** The workspace uses `priestAdd`/`threeAdd`, which carry the
  exact-cancellation 0-overlap gap (#1057). Products rarely cancel exactly, but the
  arrest machinery is shared; resolve #1057 in tandem.
- **Termination on infinite operands.** Online mul of two genuinely infinite reals must
  still make monotone progress (each emit lowers `bound` by `k`); the `safety_counter`
  guards bugs, not legitimate non-termination -- verify the frontier always advances.
- **Division (phase 2) depends on this.** `div`'s long-division step reduces the
  remainder by `q_i * y` -- a `mul_scalar`; online div reuses online mul's arrest. Keep
  the mul interfaces division-friendly.

## 8. Phase 1 work breakdown

0. **Source** dissertation 4.2.5 streaming mul + FCL.hs `mulRec`/`isSafe_mul`; confirm
   the margin and the frontier order. (Blocks everything; FCL.hs not in repo.)
1. Implement `mulRec_state` + `mulRec_step` + anti-diagonal frontier; `mul()` becomes
   the cons+thunk wrapper, integer-`depth` form a `take(n)` shim.
2. Equivalence + 0-overlap + value tests (section 6.1-6.3) green against eager mul.
3. Laziness/memoization tests (section 6.4) -- the new capability.
4. Swap call sites: nothing changes for callers passing `depth` (the shim), but verify
   the full arithmetic + math + summation suites pass unchanged.
5. Re-benchmark euler_gamma (#1053) on online mul as the headline win.

## 9. References

- Code: `multiply.hpp` (eager mul + the deferred-streaming note, dissertation 4.2.5),
  `threeAdd.hpp:441-683` (`isSafe`, `addRec_state`, `addRec_step`, `add` -- the online
  template), `block_eft.hpp` (`block_two_mult`), `zbcl.hpp` (lazy co-list).
- Design note: `elreal-lazy-incremental-precision.md` (#1060).
- Issues: #1061 (this work), #1058 (closed; coefficient symptom), #1053 (euler_gamma
  benchmark, sequenced behind this), #1057 (shared arrest machinery), #1040 (cost
  tooling).
- External (NOT in repo): McCleeary 2019 dissertation 4.2.5; FCL.hs reference.
