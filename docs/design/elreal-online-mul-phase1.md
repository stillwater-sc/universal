# elreal #1061 Phase 1: Online (pull-driven) Multiplication -- Scoping Note (v2, dissertation-grounded)

Status: scoping / design (no code change). Phase 1 of #1061. Companion to
`elreal-lazy-incremental-precision.md` (#1060).

> **Correction (v2).** The first version of this note (merged in #1062) proposed
> implementing online `mul` as a 2D "anti-diagonal frontier" with a novel
> multiplicative carry-arrest margin `M(k)`. **That was wrong** -- it was reasoned
> from first principles without the source. The McCleeary 2019 dissertation (the
> primary reference the code already translates from) does no such thing:
> multiplication and division reduce to a **streaming infinite summation `infSum`**
> over lazily-generated 2-block products, and `infSum` **reuses add's `isSafe`** --
> there is no separate mul carry-arrest to derive. This v2 replaces sections 3-4 of
> the original with the dissertation's actual algorithm (verified against FCL.hs
> Appendix A.4 and the proofs in section 4.2.3/4.2.5).

## 0. Context

The current `mul` (`multiply.hpp:55-75`) is a self-declared "Phase 6 FINITE-PREFIX"
placeholder that `take(depth)`s both operands and `priestRenorm`s the pool; its header
defers "a fully streaming anti-diagonal product" citing dissertation 4.2.5. The current
`sum()` (`sum.hpp`) is likewise a deliberate **eager** stand-in for the dissertation's
streaming `infSum` -- its header says a "fully-lazy summation would need a Phase 4 add()
that composes lazily without breaking 0-overlap." This note shows the dissertation
already specifies that streaming summation; phase 1 is to translate it (as `addRec` was
translated from FCL.hs), then mul/div fall out as thin wrappers.

## 1. The reduction: mul and div are `infSum` wrappers (FCL.hs A.4, dissertation 4.2.5/4.2.6)

```haskell
-- multiply: shift operands into [-1,1], mult, shift result back
multiply (a:as) (b:bs) =
  let ((na:nas), aShift) = shiftDown (a:as) in
  let ((nb:nbs), bShift) = shiftDown (b:bs) in
  shiftUp (aShift + bShift) (mult (na:nas) (nb:nbs))

mult as bs = infiniteSum (infSumMultHelper as bs)              -- operands in [-1,1]
infSumMultHelper (f:fs) (g:gs) = singleMult f (g:gs) : infSumMultHelper fs (g:gs)
singleMult f gs = infiniteSum (singleMultHelper f gs)
singleMultHelper f (g:gs) = let (s,e) = twoMult f g in [s,e] : singleMultHelper f gs

-- divide is the SAME shape (phase 2)
div fs gs = infiniteSum (divideHelper fs gs)
divideHelper fs (g:gs) =
  if isZero g then [g] : divideHelper fs gs
  else (singleDiv fs g) : divideHelper (negation (multiply fs gs)) (singleMult g (g:gs))
singleDiv fs g = infiniteSum (singleDivHelper fs g)
singleDivHelper (f:fs) g = (twoDivFCL f g) : singleDivHelper fs g
twoDivFCL x y = if isZero x then [createZero (getSize x) (getExp x - getExp y)]
                else let (a,b) = twoDiv x y in a : twoDivFCL b y
```

So the entire content of "online mul" is: distribute into a lazy list of 2-block
products and feed it to `infSum`. No frontier, no `M(k)`. The proofs (Lemmas
4.2.16-4.2.21) need only that the generated ZBCLs have **strictly decreasing leading
exponents**, which `singleMultHelper`/`infSumMultHelper` give for free (each `twoMult`
drops the leading exponent by >= k via the `gs` 0-overlap; each `f` in `fs` drops by k).

## 2. The one real algorithm: streaming `infSum` (dissertation 4.2.3, FCL.hs A.4)

```haskell
infiniteSum as = let sum = infSum as in            -- drop a leading zero block if present
  if null sum then sum else let (high:rest) = sum in if isZero high then rest else sum

infSum (as:bs:rest) =
  let nprevs = addition as bs
  in infSumRec rest nprevs (getExp (head as) + getSize (head as) + 1)
infSum (as:[]) = as ;  infSum [] = []

infSumRec [] prevs _              = prevs
infSumRec (as:[]) prevs _         = addition as prevs
infSumRec (as:rest) [] bound      = infSumRec rest as bound
infSumRec (as:bs:rest) (prev:prevs) bound =
  if bound > getExp prev + getSize prev + 2 then           -- cancellation region
    let zero = createZero (getSize prev) bound
    in if isSafe zero as bs bs prev
       then zero : infSumRec (as:bs:rest) (prev:prevs) (getExp zero - getSize zero)
       else infSumRec (bs:rest) (addition as (prev:prevs)) bound
  else                                                      -- normal region
    let sum = addition (prev:prevs) as
    in if null sum then infSumRec rest as bound
       else let (high:highs) = sum
            in if isZero high then infSumRec (bs:rest) highs bound
               else if isSafe high bs bs prev
                    then high : infSumRec (bs:rest) highs (getExp high - getSize high)
                    else infSumRec (bs:rest) (high:highs) bound

addition fs gs = let sum = add fs gs in                    -- add, then drop a leading zero
  if null sum then sum else let (high:rest) = sum in if isZero high then rest else high:rest
```

`infSumRec` keeps a single accumulator ZBCL `prevs`, folds each next input term into it
with `addition` (= `add` + leading-zero drop), and emits the accumulator's leading block
once add's **`isSafe`** clears it (or emits an explicit `zero` in the cancellation
region). It is proven productive (4.2.6), type-correct (4.2.7), and value-preserving
(4.2.9). This is NOT the "naive lazy foldr over add" the `sum.hpp` header warned about --
it is a careful single-accumulator state machine, the streaming summation the codebase
deferred.

## 3. Mapping to the codebase: what exists vs what to add

Everything `infSum`/mul needs as primitives already exists:

| dissertation | codebase | file |
|---|---|---|
| `add` (lazy addRec) | `add` / `addRec_step` | `threeAdd.hpp:666,520` |
| `isSafe` | `isSafe` (verbatim) | `threeAdd.hpp:441` |
| `createZero` | `createZero` | `threeAdd.hpp:99` |
| `twoSumRN`, `priestAdd`, `threeAdd` | same | `threeAdd.hpp` |
| `twoMult` | `block_two_mult` | `block_eft.hpp:208` |
| `twoDiv` | `block_two_div_rn` | `block_eft.hpp:243` |
| `getSize` | `block<FpType>::k` | `block.hpp:79` |
| `getExp` | `block::exponent()` | `block.hpp` |

To ADD (translate from FCL.hs A.4):
- `addition` (trivial wrapper: `add` then drop a leading zero block).
- **`infSum` / `infSumRec` / `infiniteSum`** -- the streaming summation state machine
  (the keystone; mirrors `addRec_step`'s structure: a `std::optional<block>` step over
  `infSumRec_state{ inputs : series<ZBCL>, prevs : ZBCL, bound : int32 }`, wrapped as
  `cons(first, thunk)` like `add()`).
- `singleMult` / `singleMultHelper`, `mult` / `infSumMultHelper`, `multiply` /
  `shiftUp` / `shiftDown` / `shiftUpToTwo` -- thin lazy generators feeding `infSum`.

Division (`div`/`divideHelper`/`singleDiv`/`singleDivHelper`/`twoDivFCL`/`divide`) is the
same shape and is **phase 2**, but the list above shows it reuses exactly the same
primitives plus online `multiply` and `negation` (already present).

## 4. Corrected phase-1 plan

1. **`addition`** wrapper (minutes).
2. **Streaming `infSum`/`infSumRec`** as an `addRec`-style state machine reusing
   `add`/`isSafe`/`createZero`. Validate against the eager `sum()` (equivalence on
   finite series), the dyadic oracle (`elreal_oracle.hpp`), and 0-overlap.
3. **`singleMult` + `mult` + `multiply`** (+ `shiftUp`/`shiftDown`) as `infSum` wrappers;
   replace the eager finite-prefix `mul`. Validate `online_mul.take(N)` == eager
   `mul(.,.,N)` across the arithmetic suite + random ZBCLs; dyadic-oracle value;
   0-overlap.
4. **Laziness/memoization test** (the new capability): `take(k)` then `take(k+1)` forces
   strictly fewer additional operand pulls than recompute, reusing the memoised prefix.
5. **Re-benchmark euler_gamma (#1053)** -- the headline win; the eager `sum()` is exactly
   why its ~700-term accumulation was slow.
6. Keep integer-`depth` overloads as `take(n)` shims for source compatibility.

This is a **translation job, not a derivation** -- the same discipline that produced
`addRec` from FCL.hs.

## 5. The real risk (replaces the bogus `M(k)` risk): #1057 and add-composition

`infSumRec` leans heavily on **repeated lazy `add` composition** into the `prevs`
accumulator. The dissertation proves this preserves 0-overlap given a correct `add`.
The live risk is therefore not a new carry-arrest -- it is that the existing `add` still
has the **exact-cancellation 0-overlap gap (#1057)** (and the class of #1034, already
fixed). Streaming `infSum` is the heaviest exerciser of `add` composition we have, so:
- #1057 should be treated as a **dependency** of phase 1, fixed in tandem.
- The equivalence + 0-overlap tests must include adversarial cancellation series (e.g.
  the asin+acos residual, and alternating-sign products) to surface any add-composition
  invariant break early.

Other risks: termination on genuinely infinite operands (each emit must lower `bound` by
k -- carry add's `safety_counter` backstop); and the `shiftUp/shiftDown` range reduction
must not perturb value (it is exact, exponent-only).

## 6. References

- Dissertation (McCleeary 2019, *Lazy exact real arithmetic using floating point
  operations*): 4.2.3 Infinite Summation (`infSum`/`infSumRec`), 4.2.5 Multiplication,
  4.2.6 Divide; FCL.hs Appendix A.4 (canonical Haskell, verified for this note).
- Code: `threeAdd.hpp` (`add`/`addRec_step`/`isSafe`/`createZero` -- the template and the
  reused primitives), `sum.hpp` (the eager `sum()` stand-in to be replaced), `multiply.hpp`
  (eager finite-prefix `mul`), `block_eft.hpp` (`block_two_mult`/`block_two_div_rn`).
- Design note: `elreal-lazy-incremental-precision.md` (#1060).
- Issues: #1061 (this work), #1057 (add exact-cancel -- now a phase-1 dependency, not a
  side note), #1053 (euler_gamma benchmark), #1040 (cost tooling).
