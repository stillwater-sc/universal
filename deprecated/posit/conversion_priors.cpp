#if PREVIOUS_FAILURE_INPUTS
	/*
	conversion failures for <4,1>
	no exp left : geo-dw d          0.125  result          0.0625  scale=  -4  k=  -2  exp=   -  0001 00010          0.0625     PASS
	no rounding alltaken u          0.125  result             0.5  scale=  -1  k=  -1  exp=   1  0011 00100            0.25 FAIL
	no rounding alltaken u           0.25  result               1  scale=   0  k=  -1  exp=   0  0100 00100            0.25 FAIL
	no rounding alltaken d           0.25  result            0.25  scale= - 2  k=  -1  exp=   0  0010 00100            0.25     PASS
	no rounding alltaken u          -0.25  result           -0.25  scale=  -2  k=  -1  exp=   0  1110 11100           -0.25     PASS
	no rounding alltaken d          -0.25  result              -1  scale=   0  k=  -1  exp=   0  1100 11100           -0.25 FAIL
	no rounding alltaken d         -0.125  result            -0.5  scale=  -1  k=  -1  exp=   1  1101 11100           -0.25 FAIL
	no exp left:  geo-dw u         -0.125  result         -0.0625  scale=  -4  k=  -2  exp=   -  1111 11110         -0.0625     PASS
	// incoming values and their corresponding bits
	float:            0.1249900013 Sign: 0 Scale: -4 Fraction: b11111111111101011000010
	float:                   0.125 Sign: 0 Scale: -3 Fraction: b00000000000000000000000
	float:            0.1250099987 Sign: 0 Scale: -3 Fraction: b00000000000001010011111
	float:            0.2499900013 Sign: 0 Scale: -3 Fraction: b11111111111110101100001
	float:                    0.25 Sign: 0 Scale: -2 Fraction: b00000000000000000000000
	float:            0.2500100136 Sign: 0 Scale: -2 Fraction: b00000000000000101010000
	float:           -0.2500100136 Sign: 1 Scale: -2 Fraction: b00000000000000101010000
	float:                   -0.25 Sign: 1 Scale: -2 Fraction: b00000000000000000000000
	float:           -0.2499900013 Sign: 1 Scale: -3 Fraction: b11111111111110101100001
	float:           -0.1250099987 Sign: 1 Scale: -3 Fraction: b00000000000001010011111
	float:                  -0.125 Sign: 1 Scale: -3 Fraction: b00000000000000000000000
	float:           -0.1249900013 Sign: 1 Scale: -4 Fraction: b11111111111101011000010

	conversion failures for <5,1>
	no exp left:  geo-dw d        0.03125  result        0.015625  scale=  -6  k=  -3  exp=   -  00001 000010        0.015625     PASS
	no rounding alltaken u        0.03125  result           0.125  scale=  -3  k=  -2  exp=   1  00011 000100          0.0625 FAIL
	no rounding alltaken u         0.0625  result            0.25  scale=  -2  k=  -2  exp=   0  00100 000100          0.0625 FAIL
	no rounding alltaken d         0.0625  result          0.0625  scale=  -4  k=  -2  exp=   0  00010 000100          0.0625     PASS
	no rounding alltaken u          0.125  result           0.125  scale=  -3  k=  -2  exp=   1  00011 000110           0.125     PASS
	arithmetic  rounding d          0.125  result             0.5  scale=  -1  k=  -1  exp=   1  00110 000110           0.125 FAIL
	arithmetic  rounding d         0.1875  result            0.75  scale=  -1  k=  -1  exp=   1  00111 000110           0.125 FAIL
	arithmetic  rounding u         0.1875  result            0.75  scale=  -1  k=  -1  exp=   1  00111 001000            0.25 FAIL
	arithmetic  rounding u           0.25  result               1  scale=   0  k=  -1  exp=   0  01000 001000            0.25 FAIL
	arithmetic  rounding d           0.25  result            0.25  scale=  -2  k=  -1  exp=   0  00100 001000            0.25     PASS
	arithmetic  rounding u          -0.25  result           -0.25  scale=  -2  k=  -1  exp=   0  11100 111000           -0.25     PASS
	arithmetic  rounding d          -0.25  result              -1  scale=   0  k=  -1  exp=   0  11000 111000           -0.25 FAIL
	arithmetic  rounding d        -0.1875  result           -0.75  scale=  -1  k=  -1  exp=   1  11001 111000           -0.25 FAIL
	arithmetic  rounding u        -0.1875  result           -0.75  scale=  -1  k=  -1  exp=   1  11001 111010          -0.125 FAIL
	arithmetic  rounding u         -0.125  result            -0.5  scale=  -1  k=  -1  exp=   1  11010 111010          -0.125 FAIL
	no rounding alltaken d         -0.125  result          -0.125  scale=  -3  k=  -2  exp=   1  11101 111010          -0.125     PASS
	no rounding alltaken d       -0.09375  result          -0.125  scale=  -3  k=  -2  exp=   1  11101 111010          -0.125     PASS
	no rounding alltaken u       -0.09375  result         -0.0625  scale=  -4  k=  -2  exp=   0  11110 111100         -0.0625     PASS
	no rounding alltaken u        -0.0625  result         -0.0625  scale=  -4  k=  -2  exp=   0  11110 111100         -0.0625     PASS
	no rounding alltaken d        -0.0625  result           -0.25  scale=  -2  k=  -2  exp=   0  11100 111100         -0.0625 FAIL
	no rounding alltaken d       -0.03125  result          -0.125  scale=  -3  k=  -2  exp=   1  11101 111100         -0.0625 FAIL
	no exp left:  geo-dw u       -0.03125  result       -0.015625  scale=  -6  k=  -3  exp=   -  11111 111110       -0.015625     PASS

	conversion failures for <5,2>
	no exp left:  geo-dw d    0.000244141  result     0.000244141  scale= -12  k=  -3  exp=   --  00001 000010     0.000244141     PASS
	truncated exp geo-up d    0.000976562  result          0.0625  scale=  -4  k=  -2  exp=   0-  00100 000010     0.000244141 FAIL
	truncated exp geo-dw u    0.000976563  result        0.015625  scale=  -6  k=  -2  exp=   1-  00011 000100      0.00390625 FAIL
	truncated exp geo-up u     0.00390625  result        0.015625  scale=  -6  k=  -2  exp=   1-  00011 000100      0.00390625 FAIL
	truncated exp geo-dw d     0.00390625  result      0.00390625  scale=  -8  k=  -2  exp=   0-  00010 000100      0.00390625     PASS
	truncated exp geo-dw d      0.0078125  result      0.00390625  scale=  -8  k=  -2  exp=   0-  00010 000100      0.00390625     PASS
	no rounding alltaken u      0.0078125  result             0.5  scale=  -1  k=  -1  exp=   11  00111 000110        0.015625 FAIL
	no rounding alltaken u       0.015625  result               1  scale=   0  k=  -1  exp=   00  01000 000110        0.015625 FAIL
	no rounding alltaken d       0.015625  result            0.25  scale=  -2  k=  -1  exp=   10  00110 000110        0.015625 FAIL
	no rounding alltaken d        0.03125  result             0.5  scale=  -1  k=  -1  exp=   11  00111 000110        0.015625 FAIL
	no rounding alltaken u        0.03125  result           0.125  scale=  -3  k=  -1  exp=   01  00101 001000          0.0625 FAIL
	no rounding alltaken u         0.0625  result            0.25  scale=  -2  k=  -1  exp=   10  00110 001000          0.0625 FAIL
	no rounding alltaken d         0.0625  result          0.0625  scale=  -4  k=  -1  exp=   00  00100 001000          0.0625     PASS
	no exp left:  geo-dw d            256  result             256  scale=   8  k=   2  exp=   --  01110 011100             256     PASS
	no exp left:  geo-dw d           1024  result             512  scale=   9  k=   2  exp=   --  01110 011100             256 FAIL
	no exp left:  geo-up u           1024  result            2048  scale=  11  k=   2  exp=   --  01110 011110            4096 FAIL
	no exp left:  geo-up u           4096  result            4096  scale=  12  k=   2  exp=   --  01111 011110            4096     PASS
	no exp left:  geo-up d          -4096  result           -4096  scale=  12  k=   2  exp=   --  10001 100010           -4096     PASS
	no exp left:  geo-up d          -1024  result           -2048  scale=  11  k=   2  exp=   --  10010 100010           -4096 FAIL
	no exp left:  geo-dw u          -1024  result            -512  scale=   9  k=   2  exp=   --  10010 100100            -256 FAIL
	no exp left:  geo-dw u           -256  result            -256  scale=   8  k=   2  exp=   --  10010 100100            -256     PASS
	no rounding alltaken u        -0.0625  result         -0.0625  scale=  -4  k=  -1  exp=   00  11100 111000         -0.0625     PASS
	no rounding alltaken d        -0.0625  result           -0.25  scale=  -2  k=  -1  exp=   10  11010 111000         -0.0625 FAIL
	no rounding alltaken d       -0.03125  result          -0.125  scale=  -3  k=  -1  exp=   01  11011 111000         -0.0625 FAIL
	no rounding alltaken u       -0.03125  result            -0.5  scale=  -1  k=  -1  exp=   11  11001 111010       -0.015625 FAIL
	no rounding alltaken u      -0.015625  result           -0.25  scale=  -2  k=  -1  exp=   10  11010 111010       -0.015625 FAIL
	no rounding alltaken d      -0.015625  result              -1  scale=   0  k=  -1  exp=   00  11000 111010       -0.015625 FAIL
	no rounding alltaken d     -0.0078125  result            -0.5  scale=  -1  k=  -1  exp=   11  11001 111010       -0.015625 FAIL
	truncated exp geo-dw u     -0.0078125  result     -0.00390625  scale=  -8  k=  -2  exp=   0-  11110 111100     -0.00390625     PASS
	truncated exp geo-dw u    -0.00390625  result     -0.00390625  scale=  -8  k=  -2  exp=   0-  11110 111100     -0.00390625     PASS
	truncated exp geo-up d    -0.00390625  result       -0.015625  scale=  -6  k=  -2  exp=   1-  11101 111100     -0.00390625 FAIL
	truncated exp geo-dw d   -0.000976563  result       -0.015625  scale=  -6  k=  -2  exp=   1-  11101 111100     -0.00390625 FAIL
	truncated exp geo-up u   -0.000976562  result         -0.0625  scale=  -4  k=  -2  exp=   0-  11100 111110    -0.000244141 FAIL
	no exp left:  geo-dw p   -0.000244141  result    -0.000244141  scale= -12  k=  -3  exp=   --  11111 111110    -0.000244141     PASS

	 */

	posit<4, 1> p;
	input = 0.015625; reference = 0.0625;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 0.03125; reference = 0.0625;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 0.0624; reference = 0.0625;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 0.06251; reference = 0.0625;
	p = input;
	GenerateTestCase(input, reference, p);

	input = 0.1249; reference = 0.0625;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 0.1251; reference = 0.25;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 0.125; reference = 0.25;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 0.2499; reference = 0.25;
	p = input;
	GenerateTestCase(input, reference, p);

	input = 3.99; reference = 4.0;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 4.01; reference = 4.0;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 7.99; reference = 4.0;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 8.01; reference = 16.0;
	p = input;
	GenerateTestCase(input, reference, p);
#endif
