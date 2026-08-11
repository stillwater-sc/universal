# Takum Arithmetic

The takum specification defines two variants that share an identical
`(S, D, R, C, M)` bit layout and differ only in the value map:

| variant | base | 2404.18603 | 2408.10594 |
|---|---|---|---|
| logarithmic takum | sqrt(e) | Definition 2 | Definition 1 |
| linear takum | 2 | Definition 8 (Sec. 4.7) | Definition 2 |

`takum<nbits, rbits, bt>` implements the **linear** (floating-point) variant. The
logarithmic variant is a separate, not-yet-implemented type (`takum_log<>`).

Two naming caveats worth knowing:

- arXiv:2404.18603 Section 4.7 designates the **logarithmic** variant as the
  standard and requires implementations to state explicitly which one they
  provide -- hence this note.
- The [libtakum](https://github.com/takum-arithmetic/libtakum) reference
  implementation uses the opposite default: bare `takumN` is linear and
  `takum_logN` is logarithmic. Universal follows libtakum's naming.

See [docs/takum-design.md](../../../../../docs/takum-design.md) for the assessment
of why the two variants are two types over one shared codec.

references:
- Laslo Hunhold, "Beating Posits at Their Own Game: Takum Arithmetic",
  [arXiv:2404.18603](https://arxiv.org/abs/2404.18603) -- defines both variants:
  Definition 2 (logarithmic) and Definition 8 / Section 4.7 (linear)
- Laslo Hunhold, "Design and Implementation of a Takum Arithmetic Hardware Codec",
  [arXiv:2408.10594](https://arxiv.org/abs/2408.10594) -- restates both:
  Definition 1 (logarithmic) and Definition 2 (linear)
- [libtakum](https://github.com/takum-arithmetic/libtakum) -- C99 reference implementation
