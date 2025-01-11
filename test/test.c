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
	#include "prext.h"

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


#define COMPARE_WITH_PRINTF(fmt, arg)																		\
do{																											\
	printf_retval = snprintf(buf_printf, BUF_SIZE, fmt, arg);												\
	prnf_retval   = snprnf(buf_prnf, BUF_SIZE, fmt, arg);													\
	failed = (prnf_retval != printf_retval);																\
	failed |= !!strcmp(buf_printf, buf_prnf);																\
	if(failed)																								\
	{																										\
		printf("\n******* FAIL *******\n" );																\
		printf("Format string = \"%s\"\n", fmt);															\
		printf("printf output (expected) = \"%s\" returned %i\n", buf_printf, printf_retval);				\
		printf("prnf output   (got)      = \"%s\" returned %i\n", buf_prnf, prnf_retval);					\
	};																										\
	ASSERT(!failed);																						\
}while(false)


#define COMPARE_WITH_PRINTF_DYN(fmt, arg, dwidth, dprec)													\
do{																											\
	printf_retval = snprintf(buf_printf, BUF_SIZE, fmt, dwidth, dprec, arg);								\
	prnf_retval   = snprnf(buf_prnf, BUF_SIZE, fmt, dwidth, dprec, arg);									\
	failed = (prnf_retval != printf_retval);																\
	failed |= !!strcmp(buf_printf, buf_prnf);																\
	if(failed)																								\
	{																										\
		printf("\n******* FAIL *******\n" );																\
		printf("Dynamic width = %i, Dynamic precision = %i\n", dwidth, dprec);								\
		printf("Format string = \"%s\"\n", fmt);															\
		printf("printf output (expected) = \"%s\" returned %i\n", buf_printf, printf_retval);				\
		printf("prnf output   (got)      = \"%s\" returned %i\n", buf_prnf, prnf_retval);					\
	};																										\
	ASSERT(!failed);																						\
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

//	used by the default character handler to write characters
	static char* default_prnf_out_ptr;

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
	TEST test_o(void);

	SUITE(suite_floats);
	TEST test_f(void);
	TEST test_e(void);

	SUITE(output_types);
	TEST test_def_out(void);
	TEST test_fptr_out(void);
	TEST test_snprnf_limit(void);
	TEST test_snappf(void);

	SUITE(dynamic_width_prec);
	TEST test_str_dyn(void);
	TEST test_i_dyn(void);

	SUITE(special);
	TEST test_col_align(void);
	TEST test_ext(void);

	static void gen_rand_fmt(char* dst, int width_max, int prec_max);
	static void gen_rand_fmt_dyn(char* dst);
	static void gen_rand_str(char* dst, int size_max);
	static double rand_dbl(double min, double max);

	static void prnf_custom_putch(void* dst, char c);

//********************************************************************************************************
// Public functions
//********************************************************************************************************

int main(int argc, const char* argv[])
{
	GREATEST_MAIN_BEGIN();
	RUN_SUITE(suite_str);
	RUN_SUITE(suite_ints);
	RUN_SUITE(suite_floats);
	RUN_SUITE(output_types);
	RUN_SUITE(dynamic_width_prec);
	RUN_SUITE(special);
	GREATEST_MAIN_END();

	return 0;
}

void prnf_putch(void* dst, char c)
{
	(void)dst;
	*default_prnf_out_ptr++ = c;
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
	RUN_TEST(test_o);
}

SUITE(suite_floats)
{
	RUN_TEST(test_f);
	RUN_TEST(test_e);
}

SUITE(output_types)
{
	RUN_TEST(test_def_out);
	RUN_TEST(test_fptr_out);
	RUN_TEST(test_snprnf_limit);
	RUN_TEST(test_snappf);
}

SUITE(dynamic_width_prec)
{
	RUN_TEST(test_str_dyn);
	RUN_TEST(test_i_dyn);
}

SUITE(special)
{
	RUN_TEST(test_col_align);
	RUN_TEST(test_ext);
}

TEST test_str(void)
{
	int count = ITERATIONS;
	int printf_retval;
	int prnf_retval;
	bool failed;
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
	int printf_retval;
	int prnf_retval;
	bool failed;
	while(count--)
	{
		gen_rand_fmt(buf_fmt, 30, 30);
		strcat(buf_fmt, "i");
		i = (int)rand_dbl(INT_MIN, INT_MAX);
		COMPARE_WITH_PRINTF(buf_fmt, i);
	};
	PASS();
}

TEST test_o(void)
{
	uint32_t i;
	i = 0b00111010011101011100101110010110;
	snprnf(buf_prnf, BUF_SIZE, "%o", i);
	ASSERT_STR_EQ("111010011101011100101110010110", buf_prnf);
	PASS();
}

TEST test_u(void)
{
	int count = ITERATIONS;
	unsigned int i;
	int printf_retval;
	int prnf_retval;
	bool failed;
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
	int printf_retval;
	int prnf_retval;
	bool failed;
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
	int printf_retval;
	int prnf_retval;
	bool failed;
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
	int printf_retval;
	int prnf_retval;
	bool failed;
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
	int printf_retval;
	int prnf_retval;
	bool failed;
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

TEST test_def_out(void)
{
	int i;
	default_prnf_out_ptr = buf_prnf;
	memset(buf_prnf, 0x7F, BUF_SIZE);
	i = prnf("0123456789");
	ASSERT_EQ(10, i);
	ASSERT(!memcmp(buf_prnf, "0123456789", 10));
	ASSERT(buf_prnf[10] == 0x7F);	// check 0 terminator was NOT written
	PASS();
}

TEST test_fptr_out(void)
{
	char* ptr = buf_str;
	int i;
	memset(buf_str, 0x7F, BUF_SIZE);
	i = fptrprnf(prnf_custom_putch, &ptr, ".0987654321.");
	ASSERT_EQ(12, i);
	ASSERT(!memcmp(buf_str, ".0987654321.", 12));
	ASSERT(buf_str[12] == 0x7F);	// check 0 terminator was NOT written
	PASS();
}

TEST test_snprnf_limit(void)
{
	int i;
	memset(buf_prnf, 0x7F, BUF_SIZE);
	i = snprnf(buf_prnf, 5, "1234567890");
	ASSERT_EQ(10, i);
	ASSERT_STR_EQ("1234", buf_prnf);
	ASSERT_EQ(0x7F, buf_prnf[5]);
	buf_prnf[0] = 0x7E;
	i = snprnf(buf_prnf, 0, "123456789");
	ASSERT_EQ(9, i);
	ASSERT_EQ(0x7E, buf_prnf[0]);
	PASS();
}

TEST test_snappf(void)
{
	int i;
	strcpy(buf_prnf, "Hello");
	i = snappf(buf_prnf, BUF_SIZE, " Test");
	ASSERT_EQ(5, i);
	ASSERT_STR_EQ("Hello Test", buf_prnf);
	memset(buf_prnf, 0x7F, BUF_SIZE);
	strcpy(buf_prnf, "123");
	i = snappf(buf_prnf, 5, "456");
	ASSERT_EQ(3, i);
	ASSERT_STR_EQ("1234", buf_prnf);
	strcpy(buf_prnf, "123456");
	i = snappf(buf_prnf, 3, "456");
	ASSERT_EQ(3, i);
	ASSERT_STR_EQ("12", buf_prnf);
	PASS();
}

TEST test_str_dyn(void)
{
	int count = ITERATIONS;
	int printf_retval;
	int prnf_retval;
	bool failed;
	int width, prec;
	while(count--)
	{
		width = rand()%20;
		prec = rand()%(width+1);
		gen_rand_fmt_dyn(buf_fmt);
		strcat(buf_fmt, "s");
		gen_rand_str(buf_str, 20);
		COMPARE_WITH_PRINTF_DYN(buf_fmt, buf_str, width, prec);
	};
	PASS();
}

TEST test_i_dyn(void)
{
	int count = ITERATIONS;
	int i;
	int printf_retval;
	int prnf_retval;
	bool failed;
	int width, prec;
	while(count--)
	{
		width = rand()%30;
		prec = rand()%(width+1);
		gen_rand_fmt_dyn(buf_fmt);
		strcat(buf_fmt, "i");
		i = (int)rand_dbl(INT_MIN, INT_MAX);
		COMPARE_WITH_PRINTF_DYN(buf_fmt, i, width, prec);
	};
	PASS();
}

TEST test_col_align(void)
{
	snprnf(buf_prnf, BUF_SIZE, "Hi\v10*There");
	ASSERT_STR_EQ("Hi********There", buf_prnf);
	PASS();
}

TEST test_ext(void)
{
	snprnf(buf_prnf, BUF_SIZE, "%n", prext_period(385476351));
	ASSERT_STR_EQ("12y 81d 12h 45m 51s", buf_prnf);
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

static void gen_rand_fmt_dyn(char* dst)
{
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
		strcat(dst, "0");
	strcat(dst, "*.*");
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

static void prnf_custom_putch(void* dst, char c)
{
	char** dst_ptr = (char**)dst;
	if(dst_ptr && *dst_ptr)
		*(*dst_ptr)++ = c;
}
