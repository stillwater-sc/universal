# Posit Refinement Visualization

Universal numbers, unums for short, are for expressing real numbers, and ranges of real numbers. 
There are two modes of operation, selectable by the programmer, _posit_ mode, and _valid_ mode.

In _posit_ mode, a unum behaves much like a floating-point number of fixed size, 
rounding to the nearest expressible value if the result of a calculation is not expressible exactly.
A posit offers more accuracy and a larger dynamic range than floats with the same number of bits.

In _valid_ mode, a unum represents a range of real numbers and can be used to rigorously bound answers 
much like interval arithmetic does.

The positive regime for a posit shows a very specific structure, as can be seen in the image blow:
![regime structure](img/positive_regimes.png)

Posit configurations have a very specific relationship to one another. When expanding a posit, the new value falls 'between' the old values of the smaller posit. The new value is the arithmetic mean of the two numbers if the expanding bit is a fraction bit, and it is the geometric mean of the two numbers if the expanding bit is a regime or exponent bit. Here is the starting progression from _posit<2,0>_ to _posit<7,1>_:

The _seed_ posit:

![seed posit](img/posit_2_0.png)

_posit<3,0>_:

![posit<3,0>](img/posit_3_0.png)

_posit<4,1>_:

![posit<4,1>](img/posit_4_1.png)

_posit<5,1>_:
![posit<5,1>](img/posit_5_1.png)

_posit<6,1>_:
![posit<6,1>](img/posit_6_1.png)

_posit<7,1>_:
![posit<7,1>](img/posit_7_1.png)

The program ".../tests/posit/posit_tables" will generate the posit encodings for reference. 
These tables are a great aid in understanding posit arithmetic and rounding.

```
>:~/dev/universal/build$ tests/posit/posit_tables
Generate posit configurations
-128         100101         111011 Sign : -1 Regime :   1 Exponent :     8 Fraction :        1 Value :             -128 11
Generate Posit Lookup table for a POSIT<3,0>
   #           Binary         Decoded       k    sign   scale          regime        exponent        fraction                         value
   0:              000             000      -2       1      -2               00               ~               ~                             0
   1:              001             001      -1       1      -1               01               ~               ~                           0.5
   2:              010             010       0       1       0               10               ~               ~                             1
   3:              011             011       1       1       1               11               ~               ~                             2
   4:              100             100       2      -1       2               00               ~               ~                           NaR
   5:              101             111       1      -1       1               11               ~               ~                            -2
   6:              110             110       0      -1       0               10               ~               ~                            -1
   7:              111             101      -1      -1      -1               01               ~               ~                          -0.5
Generate Posit Lookup table for a POSIT<4,0>
   #           Binary         Decoded       k    sign   scale          regime        exponent        fraction                         value
   0:             0000            0000      -3       1      -3               000               ~               -                             0
   1:             0001            0001      -2       1      -2               001               ~               -                          0.25
   2:             0010            0010      -1       1      -1               01-               ~               0                           0.5
   3:             0011            0011      -1       1      -1               01-               ~               1                          0.75
   4:             0100            0100       0       1       0               10-               ~               0                             1
   5:             0101            0101       0       1       0               10-               ~               1                           1.5
   6:             0110            0110       1       1       1               110               ~               -                             2
   7:             0111            0111       2       1       2               111               ~               -                             4
   8:             1000            1000       3      -1       3               000               ~               -                           NaR
   9:             1001            1111       2      -1       2               111               ~               -                            -4
  10:             1010            1110       1      -1       1               110               ~               -                            -2
  11:             1011            1101       0      -1       0               10-               ~               1                          -1.5
  12:             1100            1100       0      -1       0               10-               ~               0                            -1
  13:             1101            1011      -1      -1      -1               01-               ~               1                         -0.75
  14:             1110            1010      -1      -1      -1               01-               ~               0                          -0.5
  15:             1111            1001      -2      -1      -2               001               ~               -                         -0.25
Generate Posit Lookup table for a POSIT<4,1>
   #           Binary         Decoded       k    sign   scale          regime        exponent        fraction                         value
   0:             0000            0000      -3       1      -6               000               -               ~                             0
   1:             0001            0001      -2       1      -4               001               -               ~                        0.0625
   2:             0010            0010      -1       1      -2               01-               0               ~                          0.25
   3:             0011            0011      -1       1      -1               01-               1               ~                           0.5
   4:             0100            0100       0       1       0               10-               0               ~                             1
   5:             0101            0101       0       1       1               10-               1               ~                             2
   6:             0110            0110       1       1       2               110               -               ~                             4
   7:             0111            0111       2       1       4               111               -               ~                            16
   8:             1000            1000       3      -1       6               000               -               ~                           NaR
   9:             1001            1111       2      -1       4               111               -               ~                           -16
  10:             1010            1110       1      -1       2               110               -               ~                            -4
  11:             1011            1101       0      -1       1               10-               1               ~                            -2
  12:             1100            1100       0      -1       0               10-               0               ~                            -1
  13:             1101            1011      -1      -1      -1               01-               1               ~                          -0.5
  14:             1110            1010      -1      -1      -2               01-               0               ~                         -0.25
  15:             1111            1001      -2      -1      -4               001               -               ~                       -0.0625
Generate Posit Lookup table for a POSIT<5,0>
   #           Binary         Decoded       k    sign   scale          regime        exponent        fraction                         value
   0:            00000           00000      -4       1      -4               0000               ~               --                             0
   1:            00001           00001      -3       1      -3               0001               ~               --                         0.125
   2:            00010           00010      -2       1      -2               001-               ~               0-                          0.25
   3:            00011           00011      -2       1      -2               001-               ~               1-                         0.375
   4:            00100           00100      -1       1      -1               01--               ~               00                           0.5
   5:            00101           00101      -1       1      -1               01--               ~               01                         0.625
   6:            00110           00110      -1       1      -1               01--               ~               10                          0.75
   7:            00111           00111      -1       1      -1               01--               ~               11                         0.875
   8:            01000           01000       0       1       0               10--               ~               00                             1
   9:            01001           01001       0       1       0               10--               ~               01                          1.25
  10:            01010           01010       0       1       0               10--               ~               10                           1.5
  11:            01011           01011       0       1       0               10--               ~               11                          1.75
  12:            01100           01100       1       1       1               110-               ~               0-                             2
  13:            01101           01101       1       1       1               110-               ~               1-                             3
  14:            01110           01110       2       1       2               1110               ~               --                             4
  15:            01111           01111       3       1       3               1111               ~               --                             8
  16:            10000           10000       4      -1       4               0000               ~               --                           NaR
  17:            10001           11111       3      -1       3               1111               ~               --                            -8
  18:            10010           11110       2      -1       2               1110               ~               --                            -4
  19:            10011           11101       1      -1       1               110-               ~               1-                            -3
  20:            10100           11100       1      -1       1               110-               ~               0-                            -2
  21:            10101           11011       0      -1       0               10--               ~               11                         -1.75
  22:            10110           11010       0      -1       0               10--               ~               10                          -1.5
  23:            10111           11001       0      -1       0               10--               ~               01                         -1.25
  24:            11000           11000       0      -1       0               10--               ~               00                            -1
  25:            11001           10111      -1      -1      -1               01--               ~               11                        -0.875
  26:            11010           10110      -1      -1      -1               01--               ~               10                         -0.75
  27:            11011           10101      -1      -1      -1               01--               ~               01                        -0.625
  28:            11100           10100      -1      -1      -1               01--               ~               00                          -0.5
  29:            11101           10011      -2      -1      -2               001-               ~               1-                        -0.375
  30:            11110           10010      -2      -1      -2               001-               ~               0-                         -0.25
  31:            11111           10001      -3      -1      -3               0001               ~               --                        -0.125
Generate Posit Lookup table for a POSIT<5,1>
   #           Binary         Decoded       k    sign   scale          regime        exponent        fraction                         value
   0:            00000           00000      -4       1      -8               0000               -               -                             0
   1:            00001           00001      -3       1      -6               0001               -               -                      0.015625
   2:            00010           00010      -2       1      -4               001-               0               -                        0.0625
   3:            00011           00011      -2       1      -3               001-               1               -                         0.125
   4:            00100           00100      -1       1      -2               01--               0               0                          0.25
   5:            00101           00101      -1       1      -2               01--               0               1                         0.375
   6:            00110           00110      -1       1      -1               01--               1               0                           0.5
   7:            00111           00111      -1       1      -1               01--               1               1                          0.75
   8:            01000           01000       0       1       0               10--               0               0                             1
   9:            01001           01001       0       1       0               10--               0               1                           1.5
  10:            01010           01010       0       1       1               10--               1               0                             2
  11:            01011           01011       0       1       1               10--               1               1                             3
  12:            01100           01100       1       1       2               110-               0               -                             4
  13:            01101           01101       1       1       3               110-               1               -                             8
  14:            01110           01110       2       1       4               1110               -               -                            16
  15:            01111           01111       3       1       6               1111               -               -                            64
  16:            10000           10000       4      -1       8               0000               -               -                           NaR
  17:            10001           11111       3      -1       6               1111               -               -                           -64
  18:            10010           11110       2      -1       4               1110               -               -                           -16
  19:            10011           11101       1      -1       3               110-               1               -                            -8
  20:            10100           11100       1      -1       2               110-               0               -                            -4
  21:            10101           11011       0      -1       1               10--               1               1                            -3
  22:            10110           11010       0      -1       1               10--               1               0                            -2
  23:            10111           11001       0      -1       0               10--               0               1                          -1.5
  24:            11000           11000       0      -1       0               10--               0               0                            -1
  25:            11001           10111      -1      -1      -1               01--               1               1                         -0.75
  26:            11010           10110      -1      -1      -1               01--               1               0                          -0.5
  27:            11011           10101      -1      -1      -2               01--               0               1                        -0.375
  28:            11100           10100      -1      -1      -2               01--               0               0                         -0.25
  29:            11101           10011      -2      -1      -3               001-               1               -                        -0.125
  30:            11110           10010      -2      -1      -4               001-               0               -                       -0.0625
  31:            11111           10001      -3      -1      -6               0001               -               -                     -0.015625
Generate Posit Lookup table for a POSIT<5,2>
   #           Binary         Decoded       k    sign   scale          regime        exponent        fraction                         value
   0:            00000           00000      -4       1     -16               0000               --               ~                             0
   1:            00001           00001      -3       1     -12               0001               --               ~                0.000244140625
   2:            00010           00010      -2       1      -8               001-               0-               ~                    0.00390625
   3:            00011           00011      -2       1      -6               001-               1-               ~                      0.015625
   4:            00100           00100      -1       1      -4               01--               00               ~                        0.0625
   5:            00101           00101      -1       1      -3               01--               01               ~                         0.125
   6:            00110           00110      -1       1      -2               01--               10               ~                          0.25
   7:            00111           00111      -1       1      -1               01--               11               ~                           0.5
   8:            01000           01000       0       1       0               10--               00               ~                             1
   9:            01001           01001       0       1       1               10--               01               ~                             2
  10:            01010           01010       0       1       2               10--               10               ~                             4
  11:            01011           01011       0       1       3               10--               11               ~                             8
  12:            01100           01100       1       1       4               110-               0-               ~                            16
  13:            01101           01101       1       1       6               110-               1-               ~                            64
  14:            01110           01110       2       1       8               1110               --               ~                           256
  15:            01111           01111       3       1      12               1111               --               ~                          4096
  16:            10000           10000       4      -1      16               0000               --               ~                           NaR
  17:            10001           11111       3      -1      12               1111               --               ~                         -4096
  18:            10010           11110       2      -1       8               1110               --               ~                          -256
  19:            10011           11101       1      -1       6               110-               1-               ~                           -64
  20:            10100           11100       1      -1       4               110-               0-               ~                           -16
  21:            10101           11011       0      -1       3               10--               11               ~                            -8
  22:            10110           11010       0      -1       2               10--               10               ~                            -4
  23:            10111           11001       0      -1       1               10--               01               ~                            -2
  24:            11000           11000       0      -1       0               10--               00               ~                            -1
  25:            11001           10111      -1      -1      -1               01--               11               ~                          -0.5
  26:            11010           10110      -1      -1      -2               01--               10               ~                         -0.25
  27:            11011           10101      -1      -1      -3               01--               01               ~                        -0.125
  28:            11100           10100      -1      -1      -4               01--               00               ~                       -0.0625
  29:            11101           10011      -2      -1      -6               001-               1-               ~                     -0.015625
  30:            11110           10010      -2      -1      -8               001-               0-               ~                   -0.00390625
  31:            11111           10001      -3      -1     -12               0001               --               ~               -0.000244140625
```


