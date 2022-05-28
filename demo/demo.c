/*
*/

	#include <stdint.h>
	#include <stddef.h>
	#include <stdio.h>
	#include <string.h>
	#include <time.h>
	#include <complex.h>

	#include "prnf.h"
	#include "prext.h"

//********************************************************************************************************
// Configurable defines
//********************************************************************************************************
	
//********************************************************************************************************
// Local defines
//********************************************************************************************************

	#define DBG(_fmtarg, ...)		prnf("\e[32m%n %s:%.4i(%s)\v55 : "_fmtarg"\e[m\n", prext_tstamp("%y.%m.%d-%H:%M:%S"), __FILE__, __LINE__ , __func__,##__VA_ARGS__)
	#define DBG_ERR(_fmtarg, ...)	fptrprnf(demo_fputch, (void*)stderr, "\e[31m%n %s:%.4i(%s) **ERROR**\v55 : "_fmtarg"\e[m\n", prext_tstamp("%y.%m.%d-%H:%M:%S"), __FILE__, __LINE__, __func__,##__VA_ARGS__)

//********************************************************************************************************
// Public variables
//********************************************************************************************************

//********************************************************************************************************
// Private variables
//********************************************************************************************************

//********************************************************************************************************
// Private prototypes
//********************************************************************************************************

	static void demo_fputch(void* dst, char c);

//********************************************************************************************************
// Public functions
//********************************************************************************************************

// prepend with module name to avoid conflicts
int main(int argc, const char* argv[])
{
	complex float ca = 23.471 + 41.231*I;
	complex float cb = -18.19 + 9.473*I;
	complex float cc = ca * cb;

	prnf_out_fptr = &demo_fputch;

	printf("Hello from regular printf()\n");
	prnf("Hello from prnf()\n\n");

	DBG("Debug macro to stdout %i %i %i", 1, 2, 3);
	DBG_ERR("Something bad to stderr %i %i %i", 1, 2, 3);

	prnf("\nCenter a string within a width of 40, using regular dynamic width [%%*s]\n");
	prnf(" ----------------------------------------\n");
	prnf("[%*s%*s]\n\n", (int)(40+strlen("test"))/2, "test", (int)(40-strlen("test"))/2, "");

	prnf("\nCenter a string within a width of 40, using prnf's 0 string precision [%%40.0s]\n");
	prnf(" ----------------------------------------\n");
	prnf("[%40.0s]\n\n\n", "test");

	prnf("%%e for engineering notation, example voltages : %eV %eV %eV %eV %eV\n\n\n", 75E-6, 981E-3, 34.0, 281E3, 48E6);

	prnf("The current time is %n, which is around %n after the unix epoc\n\n", prext_tstamp("%y.%m.%d-%H:%M:%S"), prext_period((uint32_t)time(NULL)));

	prnf("\v10-Column 10\n\v20*Column 20\n\v30~Column 30\n\n\n");

	prnf("\v40*\n Example banner\n\v40*\n\n");

	prnf("Complex multiplication in rectangular form:\n%n * %n = %n\n\n", prext_cplex_rec(ca), prext_cplex_rec(cb), prext_cplex_rec(cc));
	prnf("Complex multiplication in polar form:\n%n * %n = %n\n\n", prext_cplex_pol(ca), prext_cplex_pol(cb), prext_cplex_pol(cc));

	return 0;
}

//********************************************************************************************************
// Private functions
//********************************************************************************************************

// As this is a desktop application, lets have a character handler that can write to a FILE*
static void demo_fputch(void* dst, char c)
{
	if(dst == NULL)
		dst = stdout;
	fprintf((FILE*)dst, "%c",c);
}
