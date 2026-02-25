# Takum Arithmetic

- Icelandic 'takmarkao umfang', meaning 'limited range'
- Improvement over posits by more efficient representation of scale using a 3-bit regime
- Regime value is the number of exponent bits minus 1
- Exponent bits follow regime bits, implicit leading 1-bit except for regime value 0
- A separate direction bit D to store information previously stored in R0 posit bit

# Design goals for takums

- have a more effective dynamic range for general purpose computing
- Takums are a posit-like number system with bounded dynamic range

- Problem: excessive dynamic range 2^+-256, 2^+-65535, etc. wasting bits for numbers that will never be used
- At some point, additional bits should only contribute to precision, not dynamic range
- Add two new dynamic range criteria for number systems:
    - The magnitudes of the largest and smallest representable exponent should be equal
    - The dynamic range should be reasonably bounded on both ends as the bit pattern length approaches infinity
- IEEE-754 violates both conditions, posits violate the last condition
- takums: find a format with reasonably bounded dynamic range



