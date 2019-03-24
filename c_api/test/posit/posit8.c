#include <stdlib.h>
#include <stdio.h>
#include "../c_api/posit/posit_c_api.h"

int main(int argc, char* argv[]) 
{
	posit8_t pa, pb, pc;

	pa = NAR8;
	pb = 0;
	pc = posit_add8(pa, pb);

	printf("posit value = 8.0x%xp\n", pc);

	return EXIT_SUCCESS;
}
