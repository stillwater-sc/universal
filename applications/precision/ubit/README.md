# Numerically challenging problems


Here's a summary of what each demonstrates:

```text
  ┌───────────────────┬──────────────────────────────────────────────┬───────────────────────────────────────────────────────────────────────┐
  │      Example      │                IEEE Behavior                 │                            areal Behavior                             │
  ├───────────────────┼──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────┤
  │ rump.cpp          │ All IEEE types give ~10²¹ (wrong)            │ td_cascade (159 bits) needed for correct -0.827; areal flags overflow │
  ├───────────────────┼──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────┤
  │ muller.cpp        │ Converges to 100 (wrong - should be 6)       │ Flags [UNCERTAIN] at iteration 4, warning not to trust                │
  ├───────────────────┼──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────┤
  │ chaotic_bank.cpp  │ Double goes negative at year 18 (impossible) │ Could warn of accumulating error                                      │
  ├───────────────────┼──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────┤
  │ quadratic.cpp     │ Float loses discriminant precision silently  │ Flags [UNCERTAIN] when discriminant cancellation occurs               │
  ├───────────────────┼──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────┤
  │ thin_triangle.cpp │ Float has 15% error in Heron's formula       │ Flags [UNCERTAIN] on (s-a) where cancellation occurs                  │
  ├───────────────────┼──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────┤
  │ newton.cpp        │ Shows convergence without indication         │ Flags [UNCERTAIN] when rounding affects division                      │
  └───────────────────┴──────────────────────────────────────────────┴───────────────────────────────────────────────────────────────────────┘
```

The key insight from Muller's recurrence is particularly striking - IEEE confidently computes 100, but the correct limit is 6. The areal type starts
flagging [UNCERTAIN] at iteration 4, warning the programmer that something is wrong before the result becomes completely nonsensical.

The Rump polynomial now shows that:
  - IEEE float/double/long double: All wrong
  - dd_cascade (~106 bits): Still wrong
  - td_cascade (~159 bits): CORRECT
  - qd_cascade (~212 bits): CORRECT

This demonstrates the extreme precision requirements for this pathological example - roughly 140-150 bits are needed to get the correct answer.

