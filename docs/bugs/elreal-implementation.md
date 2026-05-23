# elreal implementation failing

and it's directly connected to why L.2.c hit the non-overlap wall. Let me check what our docs say about the choice we actually made:

● Honest answer: we built a hybrid, and the L.2.c failure is exactly the price of that hybrid.

What McCleeary's algorithm actually is

I want to flag upfront that I don't have the dissertation text in front of me — I'm reasoning from the lazy-real-arithmetic literature it builds on (Boehm,
Geuvers, Niqui) and the citations in our own docs. Treat this as a directional answer, not a verbatim summary.

McCleeary 2019 builds on the computable-real tradition where a real value x is represented as a function

f: Z → Z      such that   |x - f(n)/2^n| < 1

(call f an "approximation function"). For any precision request n, the function returns an integer m such that m/2^n is within 1 of the true value. Each
arithmetic operation op(x, y) produces a new approximation function:

(x + y)(n)  =  x(n+2) + y(n+2)             // refine inputs to enough precision, then add
(x * y)(n)  =  round(x(k) * y(k), n) for k = appropriate function of n

McCleeary's contribution: replace the arbitrary-precision integers with IEEE-754 doubles by tracking which exponent range each approximation lives in. The
"approximation" at precision n is now a double whose mantissa carries the next 53 bits past what previous approximations gave. Arithmetic operations are
still computed exactly — but exactness is preserved by tracking the residual through a sequence of EFTs and re-computing at higher precision when the
request comes in.

The key property: x(n) for any n is correct to one ulp. If you ask for n=1000 bits, the algorithm walks the EFT chain however many doubles it needs.

What we built instead

A Shewchuk-style non-overlapping multi-component expansion with a McCleeary-style lazy materialisation wrapper:

┌───────────────┬────────────────────────────────────────────┬────────────────────────────────────────────────┬───────────────────────────────────────┐
│    Aspect     │                  Shewchuk                  │                   McCleeary                    │              What we did              │
├───────────────┼────────────────────────────────────────────┼────────────────────────────────────────────────┼───────────────────────────────────────┤
│ Storage       │ array of doubles, non-overlap invariant    │ function f(n) → double with precision tracking │ array of doubles (Shewchuk)           │
├───────────────┼────────────────────────────────────────────┼────────────────────────────────────────────────┼───────────────────────────────────────┤
│ Refinement    │ eager: full expansion built upfront        │ lazy: compute next bits on demand              │ lazy materialisation (McCleeary)      │
├───────────────┼────────────────────────────────────────────┼────────────────────────────────────────────────┼───────────────────────────────────────┤
│ Arithmetic op │ exact via expansion_sum etc. (Bailey/Hida) │ exact via EFT chain with precision propagation │ closed-form depth-1 formulas (ad-hoc) │
├───────────────┼────────────────────────────────────────────┼────────────────────────────────────────────────┼───────────────────────────────────────┤
│ Per-op cost   │ O(N²) for N components, eager              │ O(1) for current depth, lazy                   │ O(1) lazy + 53-bit-per-depth          │
└───────────────┴────────────────────────────────────────────┴────────────────────────────────────────────────┴───────────────────────────────────────┘

So the type is Shewchuk storage + McCleeary laziness + closed-form depth-1 arithmetic. The third piece is the one that's neither.

Why we didn't implement the full McCleeary algorithm

Three real reasons, plus one I'm less proud of:

1. Reuse. Universal had Shewchuk-style EFT primitives (error_free_ops.hpp) and the ereal<N> type already. Building elreal on the same substrate let Phase A
ship the type skeleton in a few hundred lines instead of a full big-integer/precision-tracking infrastructure.
2. No big integers. McCleeary's algorithm in its pure form needs to track arbitrary-precision exponent-shifted residuals, which is morally a big-integer
computation. Universal commits to header-only, no-external-deps, double-only foundations. Building the precision-tracking infrastructure McCleeary's
algorithm wants is a major project of its own.
3. The depth-1 case fit cleanly. For arithmetic at depth-1 (~ 106 bits), simple closed-form formulas using EFTs give the exact answer — there's nothing for
McCleeary's precision tracking to add. So Phase C–E shipped depth-1 with closed-form formulas and worked correctly.
4. (less proud) We assumed depth-2+ would extend the same way. Phase L.2.a/b confirmed that for depth-2. We hit the wall at L.2.c.

How this connects to L.2.c

The L.2.c non-overlap violation is exactly the bug McCleeary's algorithm would have prevented.

At depth-2 the dropped EFT residuals are O(eps³), well below the depth-2 magnitude (~ eps²) — they don't affect the result. At depth-3 the dropped
residuals are O(eps⁴), comparable in magnitude to the c_3 we're computing (~ eps³). Now they dominate the discrepancy that pushes c_3 over the ulp(c_2) / 2
bound.

McCleeary's algorithm would track these residuals through the chain. Concretely, for division at depth 3:

- My closed form: c_3 = (a.at(3) - sum_terms) / b0
- McCleeary's would be: at each two_prod inside that sum, propagate prod_err into a residual-stream that becomes part of the depth-3 computation. The
leading double of the result is correct because the discarded bits live somewhere countable.

The fix #921 lists are essentially two paths back toward McCleeary:

- Option A (exact EFT tracking): track residuals through two_sum/two_prod chains. This is a small piece of McCleeary's machinery — the
precision-propagation chain for arithmetic ops.
- Option B (renormalisation): after computing tentative c_k, redistribute bits between c_{k-1} and c_k via two_sum so the non-overlap holds. This is
Priest/Bailey-style normalisation; not McCleeary specifically, but related.

A full McCleeary implementation would be Option A taken to its logical end: every operation produces a residual-stream that propagates to wherever it's
needed, regardless of depth.

Should we switch?

Honest assessment: probably not in one PR, but the direction is right.

Implementing McCleeary's algorithm properly is a multi-month effort. It would require:

- A new representation: elreal_mccleeary parallel to today's elreal, with proper precision-tracking
- All ~25 operators reimplemented (math suite included)
- New validation against MPFR / external high-precision reference (the existing oracle helper validates against the Shewchuk-style implementation, which
has the same bias)

What's pragmatic for the existing elreal:

1. Ship Option A or B from #921 as the L.2.c follow-up (smaller scope; reuses the current type)
2. Accept the depth-2 cap as a documented contract for the Shewchuk-style implementation
3. File a separate epic if someone wants the proper McCleeary algorithm — it would be a sibling type, not a replacement

The L.2.b state we shipped is honest: depth-0 and depth-1 are McCleeary-correct (~ 106 bits exact); depth-2 is approximately correct with the EFT residuals
dropped (good enough for most uses); depth-3+ would require the residual tracking to be principled.

I'd love to actually read the dissertation to verify my characterization of McCleeary's algorithm — if you have a copy of the relevant chapters, I can
sharpen this. The big-picture answer ("we built Shewchuk-style with lazy materialisation, not McCleeary-style precision tracking") is robust either way.

## McCleeary algorithm

This is a fundamentally illuminating read. Our type isn't McCleeary's algorithm at all — we mislabeled. 
Let me synthesize what McCleeary actually does, and why the L.2.c failure was inevitable given the path we took.

### McCleeary's actual algorithm

The representation is a ZBCL_k — Zero-overlap Block Co-list with k-bit blocks. Three layers:

1. Block (s, e, bv): sign in {-1, +1}, integer exponent e, and a k-bit bit-vector bv. The block's value is s * 2^e * Σ_{i=0..k-1} bv[i] * 2^-i. So a block
is a k-bit floating-point number with an explicit exponent and sign. Not a double — a parameterized-precision number where k is a free design parameter (he
uses k >= 3 in proofs; in practice he picks values like the IEEE double mantissa).
2. Co-list of blocks (BCL_k): a possibly-infinite lazy stream of blocks. The blocks at later positions in the stream contribute smaller magnitudes; the sum
of the stream is the represented real.
3. Zero-overlap property (ZBCL_k) — this is the crucial bit. For consecutive blocks at exponents e_n and e_{n+1}, McCleeary requires:

e_n >= e_{n+1} + k

3. That is, the bit-position immediately below block n and immediately above block n+1 is guaranteed to be zero. Figure 4.1 in the dissertation labels this
position with the literal name imp (impossible-to-be-1).

That imp bit (or "gap") is the entire reason the algorithm is correct. McCleeary says it explicitly (p. 45):

▎ The result is guaranteed to generate either a single bit in the lower part of the output... or there is guaranteed to be a bit space in which a 0 exists
▎ between the two blocks being outputted. ... The reason why this property is important is this is what allows us to do lazy arithmetic from left to right.
▎  This bit ... is what allows us to consume possible carry ups from below, without going into the space of the higher block.

What we built

Our lazy_component_buffer stores raw IEEE-754 doubles (k=53), and our invariant is Shewchuk's:

|c_{i+1}| <= ulp(c_i) / 2

This is a tighter bound than McCleeary's. McCleeary's e_n >= e_{n+1} + k (block-exponent gap) allows the next block to have any bit pattern below its
leading bit — including bits all the way down to the gap. Shewchuk's <= ulp/2 requires the next double's leading set bit to be at least one position below
the previous double's least significant bit.

Critically: Shewchuk has no imp bit. There's no buffer space between c_i and c_{i+1} to absorb carry-ups. That's why Shewchuk expansions need
renormalization (the Renormalize algorithm in §3.1.1 of the dissertation, the same one Bailey/Hida use for QD): they eagerly compute the full result, then
sweep back through to redistribute carries.

McCleeary's imp gap eliminates the need for the renormalization sweep — the gap is built into the representation as it's produced. That's what makes the
algorithm genuinely lazy left-to-right.

How McCleeary actually computes

Take addition (§4.2.1). His helper is threeAdd: take one block from each input co-list plus one block from a workspace (carry register), produce three
output blocks out_1, out_2, out_3 satisfying:

- Value preservation: a + b + c = out_1 + out_2 + out_3
- Gap preservation: dominate out_1 out_2 ∧ dominate out_2 out_3
- Exponent bound: (getExp a) - k <= (getExp out_1) <= (getExp a) + 2
- A "no-shrink" guarantee: if the input exponents are far enough apart, getExp out_1 = getExp a (no catastrophic shrinkage from this step)

The algorithm walks left-to-right through the input co-lists. At each step it commits exactly one output block (the leftmost one of (out_1, out_2, out_3)
that's "safe to output" — meaning the gap to whatever comes next is preserved). The other two blocks become the workspace for the next step.

And a hard-won lesson from §4.2 (p. 53): McCleeary describes his own struggle:

▎ My first attempt at the addition algorithm took and performed exact floating point additions ... and tried to output a block that was safe to output. ...
▎  Ultimately all of these attempts failed to produce a ZBCL_k. ... I was viewing the problem as a question of "How much of the input ZBCL_k do we need to
▎ see before producing an output?". I switched the problem to a simple one, "When do we know a block_k is safe to output?"

That reframing is the whole game. McCleeary's algorithm is "compute eagerly into a workspace, and only commit a block when you have proof the gap will
hold." We don't do that. We commit each c_k immediately and assume the formulas will keep the bound.

Why our L.2.c fails

The L.2.c symptom (c_3 exceeds ulp(c_2)/2 by 1.5x) is the direct, predictable consequence of three mismatches:

1. We use Shewchuk's tight bound instead of McCleeary's gap-leaving bound. Shewchuk forces every limb to honour <= ulp/2, but his algorithms enforce that
via a renormalization sweep. Ours don't.
2. We dropped EFT residuals from the depth-2+ formulas. In McCleeary, residuals would propagate into the workspace and influence whether the next block can
be safely committed. In our system, they just disappear.
3. We commit c_k greedily. The lazy framework's at(k) materialises c_k once and stores it. McCleeary's framework would carry the workspace forward and emit
a block only when the gap is provable. We don't have a workspace concept; the "result so far" is exactly the materialised prefix, no slack.

So the L.2.c violation is the predictable failure mode: we built a Shewchuk-style representation (which needs renormalization) on top of McCleeary-style
laziness (which avoids renormalization but only because of the gap). Neither half is wrong by itself; the combination is.

Three honest paths forward

The L.2.c issue (#921) said "exact EFT tracking or post-computation renormalisation". Given the dissertation, I'd reframe those:

1. Add the renormalization sweep (Shewchuk path): After computing tentative c_k, walk (c_{k-1}, c_k) through Fast2Sum (the same Renormalize algorithm
Bailey/Hida use for QD) to redistribute bits so the Shewchuk bound holds. Lossy with respect to the existing materialised c_{k-1} — we have to mutate it.
Conflicts with our "materialised components are immutable" contract; would need a small refactor.
2. Add EFT residual propagation (still Shewchuk-style): Track residuals through the formula via two_sum/two_prod chains. Each depth-k evaluation becomes
O(k) EFTs. Doesn't change the representation; just makes the formulas exact. The tightest fix that doesn't redesign the storage.
3. Switch to McCleeary's actual algorithm (proper path): Reimplement as ZBCL_53 — blocks of size 53 (matching double mantissa), with the e_n >= e_{n+1} + k
gap, with threeAdd-style primitives that maintain a workspace and commit blocks only when proven safe. This is a multi-month rewrite — essentially a
different number system. Worth filing as a separate epic if the user wants a true McCleeary implementation.

Honest assessment of what we shipped: Phases A-G built a very good Shewchuk-style multi-component type with McCleeary-style lazy materialisation grafted on
top. Depth-0 and depth-1 are correct because the EFTs preserve Shewchuk's bound exactly for one-step operations. Depth-2 (L.2.a/b) is correct because the
dropped EFT residuals are still small enough. Depth-3+ is where the architectural mismatch surfaces.

I'd recommend updating issue #921 to reference the dissertation's actual algorithm and choose path (1) or (2) as the pragmatic fix, with path (3) filed as
a longer-term aspiration if the user wants a faithful McCleeary implementation. Want me to update #921 with this finding?

