July 19, 2022 ETLO

conversion_algorithm_development.cpp_ contains code experiments to develop and test the conversion functions for posits.
Testing these functions based on first principles turned out to be a bust, but there is interesting notes in this
file that I wanted to preserve. I have given it the .cpp_ extension to have it ignored in the cmake process, but still
carry the information that this was a regression test file for the posit number.

I tried to clean up the warnings yesterday, but was not entirely successful, and introduced a segfault for gcc. As this
is not a working regression test, I removed it from the testing set.