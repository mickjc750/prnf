/*
	prnf() extensions

	This source file is an example only.
	It is expected to be extended with various prext_mything(mything_t mything) functions specific to your application.

*/

	#include <stdbool.h>
	#include <stddef.h>
	#include <stdarg.h>
	#include <string.h>
	#include <inttypes.h>
	#include <time.h>
	#include <complex.h>

	#include "prnf.h"

	#include <stdlib.h>
	#include <assert.h>
	#define prext_malloc(arg) 	malloc(arg)
	#define PREXT_ASSERT(arg) 	assert(arg)

//********************************************************************************************************
// Public functions
//********************************************************************************************************

int* prext_cplex_rec(complex float c)
{
	#define TXT_SIZE sizeof("(1234567890.1234567890, 1234567890.1234567890)")

	char* txt = prext_malloc(TXT_SIZE);
	snprnf(txt, TXT_SIZE, "(%f, %f)", creal(c), cimag(c));

	#undef TXT_SIZE
	return (int*)txt;
}

int* prext_cplex_pol(complex float c)
{
	#define TXT_SIZE sizeof("(1234567890.1234567890 @1234567890.1234567890)")
	
	char* txt = prext_malloc(TXT_SIZE);
	snprnf(txt, TXT_SIZE, "(%f @%f)", cabsf(c), cargf(c));

	#undef TXT_SIZE
	return (int*)txt;
}

// fmt example "%y.%m.%d-%H:%M:%S"
int* prext_tstamp(const char* fmt)
{
	#define TXT_SIZE 100

	time_t tmr = time(NULL);
	char* txt = prext_malloc(TXT_SIZE);
	PREXT_ASSERT(txt);
	txt[0] = 0;
	strftime(txt, TXT_SIZE, fmt, localtime(&tmr));

	#undef TXT_SIZE
	return (int*)txt;
}

int* prext_period(uint32_t seconds)
{
	#define TXT_SIZE (sizeof("XXy XXXd XXh XXm XXs "))
	char* txt;
	txt = prext_malloc(TXT_SIZE);
	PREXT_ASSERT(txt);
	txt[0] = 0;

	if(seconds >= 31536000)
	{
		snappf_SL(txt, TXT_SIZE, "%"PRIu32"y ", seconds/31536000);
		seconds %= 31536000;
	};
	if(seconds >= 86400)
	{
		snappf_SL(txt, TXT_SIZE, "%"PRIu32"d ", seconds/86400);
		seconds %= 86400;
	};
	if(seconds >= 3600)
	{
		snappf_SL(txt, TXT_SIZE, "%"PRIu32"h ", seconds/3600);
		seconds %= 3600;
	};
	if(seconds >= 60)
	{
		snappf_SL(txt, TXT_SIZE, "%"PRIu32"m ", seconds/60);
		seconds %= 60;
	};
	if(seconds)
		snappf_SL(txt, TXT_SIZE, "%"PRIu32"s ", seconds);
	else if(!txt[0])
		snappf_SL(txt, TXT_SIZE, "0s ");
	#undef TXT_SIZE

	txt[strlen(txt)-1] = 0;	//remove trailing ' '

	return (int*)txt;
}

//********************************************************************************************************
// Private functions
//********************************************************************************************************
