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

	SUITE(suite_i);
	TEST test_i(void);

	static void gen_rand_fmt(char* dst, int width_max);
	static void gen_rand_str(char* dst, int size_max);

//********************************************************************************************************
// Public functions
//********************************************************************************************************

int main(int argc, const char* argv[])
{
	GREATEST_MAIN_BEGIN();
	RUN_SUITE(suite_str);
	RUN_SUITE(suite_i);
	GREATEST_MAIN_END();

//	printf("[%- 026.8i]\n", 1786477002);
//	sprnf(buf_prnf, "[%- 026.8i]", 1786477002);
//	printf("%s\n", buf_prnf);
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

SUITE(suite_i)
{
	RUN_TEST(test_i);
}

TEST test_str(void)
{
	int count = 5000;
	while(count--)
	{
		gen_rand_fmt(buf_fmt, 20);
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
	int count = 5000;
	int i;
	while(count--)
	{
		gen_rand_fmt(buf_fmt, 30);
		strcat(buf_fmt, "i");
		i = rand();
		COMPARE_WITH_PRINTF(buf_fmt, i);
	};
	PASS();
}


static void gen_rand_fmt(char* dst, int width_max)
{
	int width = 1+rand()%width_max;
	int prec = 1+rand()%width;
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