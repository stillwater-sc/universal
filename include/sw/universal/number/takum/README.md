# Takum Arithmetic

`takum<nbits, rbits, bt>` implements the **linear** (floating-point) takum,
Definition 2 of the hardware codec paper. The **logarithmic** takum (Definition 1
of the original paper) is a separate, not-yet-implemented type; see
[docs/takum-design.md](../../../../../docs/takum-design.md) for the assessment of
why the two variants are two types over one shared codec.

references:
- Laslo Hunhold, "Beating Posits at Their Own Game: Takum Arithmetic",
  [arXiv:2404.18603](https://arxiv.org/abs/2404.18603) -- logarithmic takum (Definition 1)
- Laslo Hunhold, "Design and Implementation of a Takum Arithmetic Hardware Codec",
  [arXiv:2408.10594](https://arxiv.org/abs/2408.10594) -- linear takum (Definition 2)
- [libtakum](https://github.com/takum-arithmetic/libtakum) -- C99 reference implementation
