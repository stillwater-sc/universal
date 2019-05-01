After extracting the posit arithmetic operations in a C-callable api,
did a quick performance check, and found a massive slow-down. 
Posit<8,0> adds went from 100MPOP to 45MPOP.
That result was generated with perf_8bit_posit_v1.exe.

I did a quick check, substituting back the inline version of the code
for operation+=(), generating the test executable perf_8bit_posit_v2.exe.
The performance results were the same: 45MPOPS for adds.

Between the start of the C api work and this first test, I changed to
a newer version of CMake, that on Windows auto selects Win32 as the
compilation target.

The old perf tests where run using the x64 target.
Reconfigured the solution to use the x64 target, generating the
test executable perf_8bit_posit_x64.exe.
The performance results went back up to the 100MPOPS region for adds
using the new C function calls. 

However, the variability of the performance results went up.

The peak performance of multiply went down from 
167 MPOPS to 130 MPOPS
