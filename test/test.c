/*
*/

	#include <stdlib.h>
	#include <stdbool.h>
	#include <stdio.h>
	#include <assert.h>
	#include <limits.h>
	#include <stdint.h>
	#include <math.h>

	#include "greatest.h"
	#include "strview.h"
	#include "strnum.h"
	#include "prnf.h"


//********************************************************************************************************
// Configurable defines
//********************************************************************************************************
 
//	Float comparison tolerance factor
 	#define FLOAT_TOLERANCE	0.0000000001

 //	Some tests randomly generate different format strings and arguments this many times.
	#define ITERATIONS	50000

//********************************************************************************************************
// Local defines
//********************************************************************************************************

	#define DBG(_fmtarg, ...) printf("%s:%.4i - "_fmtarg"\n" , __FILE__, __LINE__ ,##__VA_ARGS__)

	GREATEST_MAIN_DEFS();


#define COMPARE_WITH_PRINTF(fmt, arg)						\
do{															\
	snprintf(buf_printf, BUF_SIZE, fmt, arg);				\
	snprnf(buf_prnf, BUF_SIZE, fmt, arg);					\
	if(strcmp(buf_printf, buf_prnf))						\
		printf("Failed with format string \"%s\" expected \"%s\" got \"%s\" ", fmt, buf_printf, buf_prnf);	\
	ASSERT_STR_EQ(buf_printf, buf_prnf);					\
}while(false)

//********************************************************************************************************
// Public variables
//********************************************************************************************************


//********************************************************************************************************
// Private variables
//********************************************************************************************************

	#define BUF_SIZE	200
	static char buf_prnf[BUF_SIZE];
	static char buf_printf[BUF_SIZE];
	static char buf_fmt[BUF_SIZE];
	static char buf_str[BUF_SIZE];

//********************************************************************************************************
// Private prototypes
//********************************************************************************************************

	SUITE(suite_str);
	TEST test_str(void);
	TEST test_str_center(void);

	SUITE(suite_ints);
	TEST test_i(void);
	TEST test_u(void);
	TEST test_X(void);
	TEST test_lli(void);
	TEST test_llu(void);
	TEST test_llX(void);

	SUITE(suite_floats);
	TEST test_f(void);
	TEST test_e(void);

	static void gen_rand_fmt(char* dst, int width_max, int prec_max);
	static void gen_rand_str(char* dst, int size_max);
	static double rand_dbl(double min, double max);

//********************************************************************************************************
// Public functions
//********************************************************************************************************

int main(int argc, const char* argv[])
{
	GREATEST_MAIN_BEGIN();
	RUN_SUITE(suite_str);
	RUN_SUITE(suite_ints);
	RUN_SUITE(suite_floats);
	GREATEST_MAIN_END();

	return 0;
}

//********************************************************************************************************
// Private functions
//********************************************************************************************************

SUITE(suite_str)
{
	RUN_TEST(test_str);
	RUN_TEST(test_str_center);
}

SUITE(suite_ints)
{
	RUN_TEST(test_i);
	RUN_TEST(test_u);
	RUN_TEST(test_X);
	RUN_TEST(test_lli);
	RUN_TEST(test_llu);
	RUN_TEST(test_llX);
}

SUITE(suite_floats)
{
	RUN_TEST(test_f);
	RUN_TEST(test_e);
}

TEST test_str(void)
{
	int count = ITERATIONS;
	while(count--)
	{
		gen_rand_fmt(buf_fmt, 20, 20);
		strcat(buf_fmt, "s");
		gen_rand_str(buf_str, 20);
		COMPARE_WITH_PRINTF(buf_fmt, buf_str);
	};
	PASS();
}

TEST test_str_center(void)
{
	snprnf(buf_prnf, BUF_SIZE, "[%10.0s]", "TEST");
	ASSERT_STR_EQ("[   TEST   ]", buf_prnf);
	snprnf(buf_prnf, BUF_SIZE, "[%4.0s]", "TEST");
	ASSERT_STR_EQ("[TEST]", buf_prnf);
	snprnf(buf_prnf, BUF_SIZE, "[%3.0s]", "TEST");
	ASSERT_STR_EQ("[TEST]", buf_prnf);
	PASS();
}

TEST test_i(void)
{
	int count = ITERATIONS;
	int i;
	while(count--)
	{
		gen_rand_fmt(buf_fmt, 30, 30);
		strcat(buf_fmt, "i");
		i = (int)rand_dbl(INT_MIN, INT_MAX);
		COMPARE_WITH_PRINTF(buf_fmt, i);
	};
	PASS();
}

TEST test_u(void)
{
	int count = ITERATIONS;
	unsigned int i;
	while(count--)
	{
		gen_rand_fmt(buf_fmt, 30, 30);
		strcat(buf_fmt, "u");
		i = (unsigned)rand_dbl(0, UINT_MAX);
		COMPARE_WITH_PRINTF(buf_fmt, i);
	};
	PASS();
}

TEST test_X(void)
{
	int count = ITERATIONS;
	unsigned int i;
	while(count--)
	{
		gen_rand_fmt(buf_fmt, 30, 30);
		strcat(buf_fmt, "X");
		i = (unsigned)rand_dbl(0, UINT_MAX);
		COMPARE_WITH_PRINTF(buf_fmt, i);
	};
	PASS();
}

TEST test_lli(void)
{
	int count = ITERATIONS;
	long long i;
	while(count--)
	{
		gen_rand_fmt(buf_fmt, 30, 30);
		strcat(buf_fmt, "lli");
		i = (long long)rand_dbl(LLONG_MIN, LLONG_MAX);
		COMPARE_WITH_PRINTF(buf_fmt, i);
	};
	PASS();
}

TEST test_llu(void)
{
	int count = ITERATIONS;
	unsigned long long i;
	while(count--)
	{
		gen_rand_fmt(buf_fmt, 30, 30);
		strcat(buf_fmt, "llu");
		i = (unsigned long long)rand_dbl(0, ULLONG_MAX);
		COMPARE_WITH_PRINTF(buf_fmt, i);
	};
	PASS();
}

TEST test_llX(void)
{
	int count = ITERATIONS;
	unsigned long long i;
	while(count--)
	{
		gen_rand_fmt(buf_fmt, 30, 30);
		strcat(buf_fmt, "llX");
		i = (unsigned long long)rand_dbl(0, ULLONG_MAX);
		COMPARE_WITH_PRINTF(buf_fmt, i);
	};
	PASS();
}

TEST test_f(void)
{
	int count = ITERATIONS;
	double i;
	double printf_i;
	double prnf_i;
	strview_t view_printf;
	strview_t view_prnf;
	int err;
	bool in_range;

	while(count--)
	{
		gen_rand_fmt(buf_fmt, 30, 6);
		strcat(buf_fmt, "f");
		i = rand_dbl(ULLONG_MAX/-1000000.0, ULLONG_MAX/1000000.0);

		snprintf(buf_printf, BUF_SIZE, buf_fmt, i);
		snprnf(buf_prnf, BUF_SIZE, buf_fmt, i);
		view_printf = cstr(buf_printf);
		view_prnf = cstr(buf_prnf);
		err = strnum_value(&printf_i, &view_printf, STRNUM_DEFAULT);
		if(err)
		{
			printf("Failed with format string \"%s\"\n", buf_fmt);
			printf("!! %s on printf output \"%s\"\n", strerror(err), buf_printf);
			ASSERT(false);
		};
		err = strnum_value(&prnf_i, &view_prnf, STRNUM_DEFAULT);
		if(err)
		{
			printf("Failed with format string \"%s\"\n", buf_fmt);
			printf("!! %s on prnf output \"%s\"\n", strerror(err), buf_prnf);
			ASSERT(false);
		};
		in_range = (printf_i-fabs(printf_i*FLOAT_TOLERANCE) < prnf_i && prnf_i < printf_i+fabs(printf_i*FLOAT_TOLERANCE));
		if(!in_range)
		{
			printf("Failed\n\
format string \"%s\"\n\
printf output \"%s\" evaluated as %f\n\
prnf output   \"%s\" evaluated as %f",buf_fmt, buf_printf, printf_i, buf_prnf, prnf_i);
			ASSERT(false);
		};
	};
	PASS();
}

TEST test_e(void)
{
	snprnf(buf_prnf, BUF_SIZE, "%0+15.2e", 1.23E-3);
	ASSERT_STR_EQ("+0000000001.23m", buf_prnf);
	snprnf(buf_prnf, BUF_SIZE, "%0+15.3e", 7.431E-6);
	ASSERT_STR_EQ("+000000007.431u", buf_prnf);
	snprnf(buf_prnf, BUF_SIZE, "%0+15.3e", -3.981E3);
	ASSERT_STR_EQ("-000000003.981k", buf_prnf);
	snprnf(buf_prnf, BUF_SIZE, "%0 15.3e", 3.710E6);
	ASSERT_STR_EQ(" 000000003.710M", buf_prnf);
	snprnf(buf_prnf, BUF_SIZE, "%e", 3.710E12);
	ASSERT_STR_EQ("3.710T", buf_prnf);
	PASS();
}

static void gen_rand_fmt(char* dst, int width_max, int prec_max)
{
	int width = 1+(rand()%width_max);
	int prec = 1+(rand()%width);
	prec = 1+(prec % prec_max);
	strcpy(dst, "%");
	if(rand()&1)
		strcat(dst, "-");
	if(rand()&1)
	{
		if(rand()&1)
			strcat(dst, "+");
		else
			strcat(dst, " ");
	};
	if(rand()&1)
	{
		if(rand()&1)
			strcat(dst, "0");
		sprintf(&dst[strlen(dst)], "%i", width);
	};
	if(rand()&1)
	{
		strcat(dst, ".");
		sprintf(&dst[strlen(dst)], "%i", prec);
	};

}

static void gen_rand_str(char* dst, int size_max)
{
	int len = abs(rand()%(size_max-1));
	while(len--)
		*dst++ = ' '+(rand()%96);
	*dst = 0;
}

static double rand_dbl(double min, double max)
{
	double a = (double)rand()/(double)RAND_MAX;
	return min + a*(max - min);
}