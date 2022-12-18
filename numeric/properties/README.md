# Properties

This directory contains programs that test properties of a number system, such as integer cover, two sum, etc.


Kahan wrote a BASIC program to test the floating-point characteristics of a system. This program -- paranoia --
has been ported to Pascal and C but these ports are failing modern security practices, so we have removed
the code from the regression set for numerical challenges. We do want to revisit the basic idea behind
the test but generalize it to the broader set of number systems that are contained within Universal,
and write it in modern C++20.
