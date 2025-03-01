/*
-------------------------------------------------------------------------------------
# BUILD OPTIONS
The below can be added to compiler flags in the makefile or project configuration.

Support floating point, provides %f and %e placeholders
	-DPRNF_SUPPORT_FLOAT

Double arguments will not be demoted to float, and prnf will use double arithmetic.
	-DPRNF_SUPPORT_DOUBLE

Support long long
	-DPRNF_SUPPORT_LONG_LONG

Default precision for %e (engineering) notation
	-DPRNF_ENG_PREC_DEFAULT=0

Default precision for %f (floats)
	-DPRNF_FLOAT_PREC_DEFAULT=3

Provide column alignment using \v (see README.md)
	-DPRNF_COL_ALIGNMENT


Alternatively, may may define a selection of the above symbols in the .c file containing #define PRNF_IMPLEMENTATION before including prnf.h

	#define PRNF_SUPPORT_FLOAT
	#define PRNF_SUPPORT_DOUBLE
	#define PRNF_SUPPORT_LONG_LONG
	#define PRNF_ENG_PREC_DEFAULT 	0
	#define PRNF_FLOAT_PREC_DEFAULT 3
	#define PRNF_COL_ALIGNMENT


-------------------------------------------------------------------------------------
# PRNF

 A lightweight printf implementation.
 With some reasonable limitations, and non-standard behavior suited to microcontrollers.
 
 * Thread and re-entrant safe.
 * Single header (stb style).
 * Low stack & ram usage, zero heap usage (unless extensions are used).
 * Full support for AVR's PROGMEM requirements, with almost no cost to non-AVR targets.
 * Compatible enough to make use of GCC's format and argument checking (even for AVR).
 
 * NO exponential form, %e provides SI units (y z a f p n u m - k M G T P E Z Y).
 * NO Octal, %o outputs binary instead, (who wants octal?)
 * NO adaptive %g %G
 
  Standard placeholder syntax:

 	%[flags][width][.precision][length]type


 Supported [flags]:

	- 		left align the output
	 
	+		prepends a + for positive numeric types
	 
	(space)		prepends a space for positive numeric type

	0 		prepends 0's instead of spaces to numeric types to satisfy [width]


 Unsupported [flags]:

	#		If you want 0x it needs to be in your format string.

	' (apostrophe)	No 1000's separator is available


 [width]
 
 	The minimum number of characters to output.
	If width is specified as * A dynamic width must be provided as an int argument preceding the argument to be formatted.
	A non-standard method of centering text is also provided (see below).


 [.precision]

 	For float %f, this is the number of fractional digits after the '.', valid range is .0 - .8
	For decimal integers, this will prepend 0's (if needed) until the total number of digits equals .precision
	For binary and hex, this specifies the *exact* number of digits to print, default is based on the argument size.
	For strings %s %S, This is the maximum number of characters to read from the source.

	If precision is specified as .* A dynamic precision must be provided as int argument preceding the argument to be formatted.

	Centering strings: If [width] is specified and precision is .0 %s arguments will be centered.
	Example to center within a 16 character LCD would be "%16.0s"
	**Caution - If you are generating formatting strings at runtime, and generate a %[width].0s, you will NOT get 0 characters.
	A dynamic precision of 0 provided to .* will not centre the string.

 Supported [length]:
 
 	Used to specify the size of the argument. The following is supported:
	hh 		Expect an int-sized argument which was promoted from a char.
	h 		Expect an int-sized argument which was promoted from a short.
	l		Expect a long-sized argument.
	ll		Expect a long long sized argument (must be enabled in prnf_conf.h).
	z		Expect a size_t sized argument (size_t must not be larger than long).
	t		Expect a ptrdiff_t sized argument (ptrdiff_t must not be larger than long).

 Unsupported [length]:

	j		intmax_t not supported


 Supported types:

  	d,i		Signed decimal integer
  	u		Unsigned decimal integer
  	x,X  	Hexadecimal. Always uppercase, .precision defaults to argument size [length]
  	o		NOT Octal. Actually binary, .precision defaults to argument size [length]
  	s		null-terminated string in ram, or NULL. Outputs nothing for NULL.
	S		For AVR targets, read string from PROGMEM, otherwise same as %s
  	c		character 

	f,F		Floating point. NAN & INF are always uppercase.
			Default precision is 3 (not 6).
			Digits printed must be able to be represented by an unsigned long, or unsigned long long (if enabled in prnf_conf.h)
			ie. with a precision of 3 and 32bit long, maximum range is +/- 4294967.296 
			Values outside this range will produce "OVER".
			A value of 0.0 is always positive. 

	e		NOT exponential. Floating point with engineering notation (y z a f p n u m - k M G T P E Z Y).
			Number is postpended with the SI prefix. Default precision is 0.
 

 Unsupported types:

	g,G 	Adaptive floats not available
	a,A		Double in hex notation not available
	p		Pointer not available
	n		classic %n is not available, but %n may be repurposed for extensions if enabled (see below)

Another approach is to use GCC's --wrap feature in the compiler flags, which is probably better.

# Example debug macro:
The following is useful for debug/diagnostic and cross platform friendly with AVR:

	#define DBG(_fmtarg, ...) prnf_SL("%s:%.4i - "_fmtarg , __FILE__, __LINE__ ,##__VA_ARGS__)

Example usage:

	DBG("value is %i\n", 51);
The above will output something like "main.c:0113 - value is 51"

# Column alignment

It is possible to advance output to a specific column, with respect to the start of the output, or the last line ending. To achieve this prnf hijacks the \v (vertical tab) character.
The required format is:

	\v<col><pad character>

\v should be followed by a decimal number indicating the column on which the following text will start on (with 0 being the first column). The character used for padding is the first non-numeric character after this number. Note that due to this, digits cannot be used as the padding character. This feature is useful if you have output which contains fields of uncontrolled length, and then wish to align further output. If the current column is already at or past the column specified, then no padding will be applied. If \v occurs in your string without being followed by digits, then a regular \v character will be output.
Example:

	prnf("%s:%.4i(%s)\v30 %s\n", __FILE__, __LINE__, __func__, "This text starts on column 30");

Will yield something like:

	myfile.c:0041(main)         This text starts on column 30


It can also be used as an easy way to create banners, for example:

	prnf("\v30*\n Main Menu\n\v30*\n");

Will yield:

******************************
 Main Menu
******************************

See README.md for more info.

*/

#ifndef _PRNF_H_
#define _PRNF_H_

//	For va_list
	#include <stdarg.h>

//	size_t
	#include <stddef.h>

//	AVR's PSTR
	#ifdef __AVR__
	#include <avr/pgmspace.h>
	#endif

//********************************************************************************************************
// Public defines
//********************************************************************************************************

/*
AVR targets.

	If you have modules that you wish to compile for both AVR and non-AVR targets, you can use the _SL (String Literal) macro wrappers.
	These will put string literals in PROGMEM for AVR targets only.
	The argument list will still be tested using a duplicate format string in ram,
	 but optimisation (any level) must be enabled to remove the ram duplicate.

	PRNF_ARG_SL() is for passing string literals as arguments to %S (upper case) placeholders.

	Arguments can safely use operators ie. prnf_SL("%i", i++);

	The following two examples will put all string literals in PROGMEM for AVR targets.
		prnf_SL("%-50S\n", PRNF_ARG_SL("LEFT"));
		prnf_SL("%50S\n", PRNF_ARG_SL("RIGHT"));
*/

#ifdef __AVR__
//	Compiler will first test argument types based on format string, then remove the empty function during optimization.
	static inline void fmttst_optout(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
	static inline void fmttst_optout(const char* fmt, ...)
	{
	}

//	_SL macros for AVR
	#define prnf_SL(_fmtarg, ...) 						({int _prv; _prv = prnf_P(PSTR(_fmtarg) ,##__VA_ARGS__); while(0) fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define sprnf_SL(_dst, _fmtarg, ...) 				({int _prv; _prv = sprnf_P(_dst, PSTR(_fmtarg) ,##__VA_ARGS__); while(0) fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define snprnf_SL(_dst, _dst_size, _fmtarg, ...) 	({int _prv; _prv = snprnf_P(_dst, _dst_size, PSTR(_fmtarg) ,##__VA_ARGS__); while(0) fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define snappf_SL(_dst, _dst_size, _fmtarg, ...) 	({int _prv; _prv = snappf_P(_dst, _dst_size, PSTR(_fmtarg) ,##__VA_ARGS__); while(0) fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define fptrprnf_SL(_fptr, _fargs, _fmtarg, ...) 	({int _prv; _prv = fptrprnf_P(_fptr, _fargs, PSTR(_fmtarg) ,##__VA_ARGS__); while(0) fmttst_optout(_fmtarg ,##__VA_ARGS__); _prv;})
	#define PRNF_ARG_SL(_arg)							((wchar_t*)PSTR(_arg))
#else
	#define prnf_SL(_fmtarg, ...) 						prnf(_fmtarg ,##__VA_ARGS__)
	#define sprnf_SL(_dst, _fmtarg, ...) 				sprnf(_dst, _fmtarg ,##__VA_ARGS__)
	#define snprnf_SL(_dst, _dst_size, _fmtarg, ...) 	snprnf(_dst, _dst_size, _fmtarg ,##__VA_ARGS__)
	#define snappf_SL(_dst, _dst_size, _fmtarg, ...) 	snappf(_dst, _dst_size, _fmtarg ,##__VA_ARGS__)
	#define fptrprnf_SL(_fptr, _fargs, _fmtarg, ...)	fptrprnf(_fptr, _fargs, _fmtarg ,##__VA_ARGS__)
	#define PRNF_ARG_SL(_arg)							((wchar_t*)(_arg))
#endif

//********************************************************************************************************
// Public variables
//********************************************************************************************************

//********************************************************************************************************
// Public prototypes
//********************************************************************************************************

//	Print, passing characters to an application provided prnf_putch(void *ctx, char c), ctx is passed NULL.
	int prnf(const char* fmtstr, ...) __attribute__((format(printf, 1, 2)));

//	Print to a char* buffer, with no size limit.
	int sprnf(char* dst, const char* fmtstr, ...) __attribute__((format(printf, 2, 3)));

//	Print safely to a char[] buffer of known size.
	int snprnf(char* dst, size_t dst_size, const char* fmtstr, ...) __attribute__((format(printf, 3, 4)));

//	*Append* safely to a char[] buffer of known size, returns the number of characters appended (ignoring truncation).
	int snappf(char* dst, size_t dst_size, const char* fmtstr, ...) __attribute__((format(printf, 3, 4)));

//	Print. Sending characters to specified character handler, not including a terminating null.
//	The character handler may be NULL if no output is required.
//	void* out_vars is also passed to the void* parameter if the character handler.
	int fptrprnf(void(*out_fptr)(void*, char), void* out_vars, const char* fmtstr, ...) __attribute__((format(printf, 3, 4)));


//	non-variadic versions of the above, accepting va_list
//	The variadic functions above are quite small and call these. 
//	If you want to implement prnf() functionality in other modules,
//	 like lcd_prnf() or uart_prnf() you can write your own variadic functions which call these
	int vprnf(const char* fmtstr, va_list va);
	int vsprnf(char* dst, const char* fmtstr, va_list va);
	int vsnprnf(char* dst, size_t dst_size, const char* fmtstr, va_list va);
    int vsnappf(char* dst, size_t dst_size, const char* fmtstr, va_list va);
	int vfptrprnf(void(*out_fptr)(void*, char), void* out_vars, const char* fmtstr, va_list va);

#ifdef __AVR__
	int prnf_P(const char* fmtstr, ...);
	int sprnf_P(char* dst, const char* fmtstr, ...);
	int snprnf_P(char* dst, size_t dst_size, const char* fmtstr, ...);
	int snappf_P(char* dst, size_t dst_size, const char* fmtstr, ...);
	int fptrprnf_P(void(*out_fptr)(void*, char), void* out_vars, const char* fmtstr, ...);
	int vprnf_P(const char* fmtstr, va_list va);
	int vsprnf_P(char* dst, const char* fmtstr, va_list va);
	int vsnprnf_P(char* dst, size_t dst_size, const char* fmtstr, va_list va);
    int vsnappf_P(char* dst, size_t dst_size, const char* fmtstr, va_list va);
	int vfptrprnf_P(void(*out_fptr)(void*, char), void* out_vars, const char* fmtstr, va_list va);
#endif

#endif // _PRNF_H_





//********************************************************************************************************
// 
// PRNF Implementation
// 
//********************************************************************************************************
#ifdef PRNF_IMPLEMENTATION



#ifndef SECOND_PASS
	#define FIRST_PASS
	#define IS_SECOND_PASS false

	#include <stdbool.h>
	#include <stdint.h>
	#include <limits.h>

//********************************************************************************************************
// Local defines
//********************************************************************************************************

	#ifndef PRNF_ENG_PREC_DEFAULT
		#define PRNF_ENG_PREC_DEFAULT 0
	#endif

	#ifndef PRNF_FLOAT_PREC_DEFAULT
		#define PRNF_FLOAT_PREC_DEFAULT 3
	#endif

	#ifndef PRNF_WARN
		#define PRNF_WARN(arg)	((void)0)
	#endif

	#ifndef PRNF_ASSERT
		#define PRNF_ASSERT(arg)	((void)0)
	#endif

	#ifdef PRNF_SUPPORT_LONG_LONG
		typedef long long prnf_long_t;
		typedef unsigned long long prnf_ulong_t;
		#define PRNF_ULONG_MAX	ULLONG_MAX
	#else
		typedef long prnf_long_t;
		typedef unsigned long prnf_ulong_t;
		#define PRNF_ULONG_MAX	ULONG_MAX
		#if SIZE_MAX > ULONG_NAX
			#warning "size_t is larger than unsigned long, and support for long long is not enabled. You should define PRNF_SUPPORT_LONG_LONG if you intend to use %z placeholders"
		#endif
	#endif

	#ifdef PRNF_SUPPORT_DOUBLE
		typedef double prnf_float_t;
		#ifndef PRNF_SUPPORT_FLOAT
			#define PRNF_SUPPORT_FLOAT
		#endif
	#else
		typedef float prnf_float_t;
	#endif

	#define HEX_DIGITS_STRING	"0123456789ABCDEF"

	#if PRNF_ULONG_MAX == 4294967295U
		#define INT_BUF_SIZE 	10
		#define FLOAT_PREC_MAX	9
		#define LONG_IS_32
	#elif PRNF_ULONG_MAX == 18446744073709551615U
		#define INT_BUF_SIZE 	20
		#define FLOAT_PREC_MAX	19
	#else
		#ifdef PRNF_SUPPORT_LONG_LONG
			#error Long Long is not 32bit or 64bit
		#else
			#error Long is not 32bit or 64bit
		#endif
	#endif

	#define NO_PREFIX	0

	#define IS_PGM		true
	#define IS_NOT_PGM	false

	#ifndef SIZE_MAX
		#define SIZE_MAX ((size_t)(ssize_t)(-1))
	#endif

	#ifdef PRNF_SUPPORT_FLOAT
		#include <float.h>
	#endif

//	Macro for reading characters from the format string. For AVR we may need to read from PROGMEM or RAM so fmt_rd_either() is used.
	#ifdef __AVR__
		#define FMTRD(_fmt) 	fmt_rd_either(_fmt, is_pgm)
	#else
		#define FMTRD(_fmt) 	(*(_fmt))
	#endif

	enum {TYPE_NONE, TYPE_BIN, TYPE_INT, TYPE_UINT, TYPE_HEX, TYPE_STR, TYPE_PSTR, TYPE_NSTR, TYPE_CHAR, TYPE_FLOAT, TYPE_ENG};	// di u xX s S c fF eE

	struct placeholder_struct
	{
		bool	flag_minus;
		bool	flag_zero;
		char 	sign_pad;			//'+' or ' ' if a prepend character for positive numeric types is specified. Otherwise 0
		bool 	prec_specified;
		int 	width;
		int 	prec;
		bool	prec_is_dynamic;
		bool 	width_is_dynamic;
		uint_least8_t size_modifier;	//equal to size of int, or size of specified type (l h hh etc)
		uint_least8_t type;				//TYPE_x
	};

	struct out_struct
	{
		#ifdef PRNF_COL_ALIGNMENT
		int 	col;
		#endif
		int		char_cnt;
		int		size_limit;
		char* 	buf;
		void* 	dst_fptr_vars;
		void(*dst_fptr)(void*, char);
	};

	struct eng_struct
	{
		prnf_float_t value;
		char prefix;
	};

//	A union capable of holding any type of argument passed in the variable argument list
	union varg_union
	{
		prnf_long_t prnf_l;
		prnf_ulong_t prnf_ul;
		long l;
		unsigned long ul;
		int i;
		unsigned int ui;
		prnf_float_t f;
		char* str;
		char c;
	};

#endif	//FIRST PASS

#ifdef FIRST_PASS
	#define prnf_PX 		prnf
	#define sprnf_PX 		sprnf
	#define snprnf_PX 		snprnf
	#define snappf_PX 		snappf
	#define vprnf_PX 		vprnf
	#define vsprnf_PX 		vsprnf
	#define vsnprnf_PX 		vsnprnf
	#define fptrprnf_PX 	fptrprnf
	#define vfptrprnf_PX 	vfptrprnf
#else
	#undef prnf_PX
	#undef sprnf_PX
	#undef snprnf_PX
	#undef snappf_PX
	#undef vprnf_PX
	#undef vsprnf_PX
	#undef vsnprnf_PX
	#undef fptrprnf_PX
	#undef vfptrprnf_PX
	#define prnf_PX 		prnf_P
	#define sprnf_PX 		sprnf_P
	#define snprnf_PX 		snprnf_P
	#define snappf_PX 		snappf_P
	#define vprnf_PX 		vprnf_P
	#define vsprnf_PX 		vsprnf_P
	#define vsnprnf_PX 		vsnprnf_P
	#define fptrprnf_PX 	fptrprnf_P
	#define vfptrprnf_PX 	vfptrprnf_P
#endif

//********************************************************************************************************
// Public variables
//********************************************************************************************************

//********************************************************************************************************
// Private variables
//********************************************************************************************************

#ifdef FIRST_PASS
#ifdef PRNF_SUPPORT_FLOAT
	#ifdef LONG_IS_32
		static prnf_float_t pow10_tbl[10] = {1E0F, 1E1F, 1E2F, 1E3F, 1E4F, 1E5F, 1E6F, 1E7F, 1E8F, 1E9F};
	#else
		static prnf_float_t pow10_tbl[20] = {1E0F, 1E1F, 1E2F, 1E3F, 1E4F, 1E5F, 1E6F, 1E7F, 1E8F, 1E9F, 1E10F, 1E11F, 1E12F, 1E13F, 1E14F, 1E15F, 1E16F, 1E17F, 1E18F, 1E19F};
	#endif
#endif
#endif

//********************************************************************************************************
// Private prototypes
//********************************************************************************************************

#ifdef FIRST_PASS
	static const char* parse_placeholder(struct placeholder_struct* placeholder, const char* fmtstr, bool is_pgm);
	static void print_placeholder(struct out_struct* out_info, union varg_union varg, struct placeholder_struct* placeholder);
	static int core_prnf(struct out_struct* out_info, const char* fmtstr, bool is_pgm, va_list va);

#ifdef PRNF_COL_ALIGNMENT
	static const char* print_col_alignment(struct out_struct* out_info, const char* fmtstr, bool is_pgm);
#endif

	static void print_hex_dec(struct out_struct* out_info, struct placeholder_struct* placeholder, prnf_long_t value);
	static void print_bin(struct out_struct* out_info, struct placeholder_struct* placeholder, prnf_ulong_t uvalue);

#ifdef PRNF_SUPPORT_FLOAT
	static void print_float(struct out_struct* out_info, struct placeholder_struct* placeholder, prnf_float_t value, char postpend);
	static const char* determine_float_msg(struct placeholder_struct* placeholder, prnf_float_t value);
	static char determine_sign_char_of_float(struct placeholder_struct* placeholder, prnf_float_t value);
	static void print_float_normal(struct out_struct* out_info, struct placeholder_struct* placeholder, prnf_float_t value, char postpend);
	static void print_float_special(struct out_struct* out_info, struct placeholder_struct* placeholder, const char* out_msg, prnf_float_t value);
	static struct eng_struct get_eng(prnf_float_t value);
	static prnf_ulong_t round_float_to_ulong(prnf_float_t x);
	static uint_least8_t get_prec(struct placeholder_struct* placeholder);
#endif

	static void prepad(struct out_struct* out_info, struct placeholder_struct* placeholder, int source_len);
	static void postpad(struct out_struct* out_info, struct placeholder_struct* placeholder, int source_len);
	static bool is_type_int(uint_least8_t type);
	static bool is_type_unsigned(uint_least8_t type);
	static bool is_type_numeric(uint_least8_t type);
	static bool is_centered_string(struct placeholder_struct* placeholder);

	static bool prnf_is_digit(char x);
	static char ascii_hex_digit(uint_least8_t x);
	
	static uint_least8_t ulong2asc_revdec(char* buf, prnf_ulong_t il);
	static uint_least8_t ulong2asc_revbin(char* buf, prnf_ulong_t il);
	static uint_least8_t ulong2asc_revhex(char* buf, prnf_ulong_t il);

	static void out_char(struct out_struct* out_info, char x);
	static void out_terminate(struct out_struct* out_info);

	static void print_str(struct out_struct* out_info, struct placeholder_struct* placeholder, const char* str, bool is_pgm);
	static int prnf_strlen(const char* str, bool is_pgm, int max);
	static int prnf_atoi(const char** fmtstr, bool is_pgm);

	#ifdef __AVR__
		static char fmt_rd_either(const char* fmt, bool is_pgm);
	#endif

#endif //FIRST_PASS

//********************************************************************************************************
// Public functions
//********************************************************************************************************

#ifdef FIRST_PASS
	#pragma weak prnf_putch
void prnf_putch(void *ctx, char c)
{
	(void)ctx;
	(void)c;
}
#endif

// On AVR platforms these _PX functions are compiled twice.
// On the first pass prnf_PX expands to prnf which reads the format string from RAM.
// On the second pass prnf_PX expands to prnf_P which reads the format string from PROGMEM.

int prnf_PX(const char* fmtstr, ...)
{
  va_list va;
  va_start(va, fmtstr);

  const int ret = vprnf_PX(fmtstr, va);

  va_end(va);
  return ret;
}

int sprnf_PX(char* dst, const char* fmtstr, ...)
{
  va_list va;
  va_start(va, fmtstr);

  const int ret = vsprnf_PX(dst, fmtstr, va);

  va_end(va);
  return ret;
}

int snprnf_PX(char* dst, size_t dst_size, const char* fmtstr, ...)
{
  va_list va;
  va_start(va, fmtstr);

  const int ret = vsnprnf_PX(dst, dst_size, fmtstr, va);

  va_end(va);
  return ret;
}

int snappf_PX(char* dst, size_t dst_size, const char* fmtstr, ...)
{
	va_list va;
	va_start(va, fmtstr);
	int chars_written = 0;
	int org_len;

	if(dst_size)
		org_len = prnf_strlen(dst, false, (int)(dst_size-1));
	else
		org_len = 0;

	chars_written = vsnprnf_PX(&dst[org_len], dst_size-org_len, fmtstr, va);

	va_end(va);
	return chars_written;
}

int fptrprnf_PX(void(*out_fptr)(void*, char), void* out_vars, const char* fmtstr, ...)
{
	va_list va;
	va_start(va, fmtstr);

	const int ret = vfptrprnf_PX(out_fptr, out_vars, fmtstr, va);

	va_end(va);
	return ret;
}

int vprnf_PX(const char* fmtstr, va_list va)
{
	struct out_struct out_info = {.dst_fptr = &prnf_putch};
	return core_prnf(&out_info, fmtstr, IS_SECOND_PASS, va);
}

int vsprnf_PX(char* dst, const char* fmtstr, va_list va)
{
	struct out_struct out_info = {.size_limit=INT_MAX, .buf=dst};
	return core_prnf(&out_info, fmtstr, IS_SECOND_PASS, va);
}

int vsnprnf_PX(char* dst, size_t dst_size, const char* fmtstr, va_list va)
{
	struct out_struct out_info = {.size_limit=dst_size, .buf=dst};
	return core_prnf(&out_info, fmtstr, IS_SECOND_PASS, va);
}

int vfptrprnf_PX(void(*out_fptr)(void*, char), void* out_vars, const char* fmtstr, va_list va)
{
	struct out_struct out_info = {.dst_fptr_vars=out_vars, .dst_fptr=out_fptr};
	return core_prnf(&out_info, fmtstr, IS_SECOND_PASS, va);
}

//********************************************************************************************************
// Private functions
//********************************************************************************************************

// wrappers for PGM or non-PGM access

#ifdef FIRST_PASS

#ifdef __AVR__
static char fmt_rd_either(const char* fmt, bool is_pgm)
{
	return is_pgm? 	pgm_read_byte(fmt):(*fmt);
}
#endif

//Read from variable argument list (src) to varg union (dst)
#define READ_VARG(dst, placeholder, src) 											\
do																					\
{																					\
	if(placeholder.width_is_dynamic)												\
	{																				\
		placeholder.width = va_arg(va, int);										\
		if(placeholder.width < 0)													\
		{																			\
			placeholder.width = -placeholder.width;									\
			placeholder.flag_minus = true;											\
		};																			\
	};																				\
																					\
	if(placeholder.prec_is_dynamic)													\
		placeholder.prec = va_arg(va, int);											\
																					\
	if(is_type_int(placeholder.type))												\
	{																				\
		if(placeholder.size_modifier == sizeof(prnf_ulong_t))						\
			dst.prnf_ul = va_arg(src, prnf_ulong_t);								\
		else if(placeholder.size_modifier == sizeof(long))							\
		{																			\
			dst.ul = va_arg(src, unsigned long);									\
			if(is_type_unsigned(placeholder.type))									\
				dst.prnf_ul = dst.ul;												\
			else																	\
				dst.prnf_l = dst.l;													\
		}																			\
		else																		\
		{																			\
			dst.ui = va_arg(src, unsigned int);										\
			if(is_type_unsigned(placeholder.type))									\
				dst.prnf_ul = dst.ui;												\
			else																	\
				dst.prnf_l = dst.i;													\
		};																			\
	}																				\
	else if(placeholder.type == TYPE_FLOAT || placeholder.type == TYPE_ENG)			\
		dst.f = (prnf_float_t)va_arg(src, double);									\
																					\
	else if(placeholder.type == TYPE_CHAR)											\
		dst.c = (char)va_arg(src, int);												\
																					\
	else if(placeholder.type == TYPE_STR || placeholder.type == TYPE_PSTR)			\
		dst.str = va_arg(src, char*);												\
																					\
	else if(placeholder.type == TYPE_NSTR)											\
		dst.str = (char*)va_arg(src, int*);											\
																					\
}while(false)

static int core_prnf(struct out_struct* out_info, const char* fmtstr, bool is_pgm, va_list va)
{
	struct placeholder_struct placeholder;
	union varg_union varg;

	while(FMTRD(fmtstr))
	{
		// placeholder? %[flags][width][.precision][length]type
		if(FMTRD(fmtstr) == '%')
		{
			fmtstr++;
			if(FMTRD(fmtstr) == '%')
			{
				out_char(out_info, '%');
				fmtstr++;
			}
			else
			{
				fmtstr = parse_placeholder(&placeholder, fmtstr, is_pgm);
				READ_VARG(varg, placeholder, va);
				print_placeholder(out_info, varg, &placeholder);
			};
		}
		#ifdef PRNF_COL_ALIGNMENT
		// colum alignment?
		else if(FMTRD(fmtstr) == '\v')
		{
			fmtstr++;
			fmtstr = print_col_alignment(out_info, fmtstr, is_pgm);
		}
		#endif
		else
		{
			out_char(out_info, FMTRD(fmtstr));
			fmtstr++;
		};
	};

	// Terminate
	out_terminate(out_info);

	return out_info->char_cnt;
}

// parse textual placeholder information into a placeholder_struct
static const char* parse_placeholder(struct placeholder_struct* dst, const char* fmtstr, bool is_pgm)
{
	struct placeholder_struct placeholder = {.size_modifier=sizeof(int), .type=TYPE_NONE};
	bool finished;

	//Get flags
	do
	{
		finished = false;
		switch(FMTRD(fmtstr))
		{
			case '0': placeholder.flag_zero = true;		break;
			case '-': placeholder.flag_minus = true;	break;
			case '+': placeholder.sign_pad = '+';  		break;
			case ' ': placeholder.sign_pad = ' ';  		break;
			case '#': PRNF_WARN(true);					break;	//unsupported flag
			case '\'': PRNF_WARN(true);					break;	//unsupported flag
			default : finished = true;        			break;
		};
		if(!finished)
			fmtstr++;
	}while(!finished);

	//Get width
	if(FMTRD(fmtstr) == '*')
	{
		placeholder.width_is_dynamic = true;
		fmtstr++;
	}
	else
		placeholder.width = prnf_atoi(&fmtstr, is_pgm);

	//Get precision
	if(FMTRD(fmtstr) == '.')
	{
		placeholder.prec_specified = true;
		fmtstr++;
		if(FMTRD(fmtstr) == '*')
		{
			placeholder.prec_is_dynamic = true;
			fmtstr++;
		}
		else
			placeholder.prec = prnf_atoi(&fmtstr, is_pgm);
	};

	//Get size modifier
	switch(FMTRD(fmtstr))
	{
		case 'h' :
			fmtstr++;
			if(FMTRD(fmtstr) == 'h')
			{
				placeholder.size_modifier = sizeof(char);
				fmtstr++;
			}
			else
				placeholder.size_modifier = sizeof(short);
			break;

		case 'l' :
			fmtstr++;
			#ifdef PRNF_SUPPORT_LONG_LONG
				if(FMTRD(fmtstr) == 'l')
				{
					placeholder.size_modifier = sizeof(long long);
					fmtstr++;
				}
				else
					placeholder.size_modifier = sizeof(long);
			#else
				PRNF_ASSERT(FMTRD(fmtstr) != 'l');	//unsupported long long
				placeholder.size_modifier = sizeof(long);
			#endif
			break;

		case 't' :
			fmtstr++;
			placeholder.size_modifier = sizeof(ptrdiff_t);
			break;

		case 'z' :
			fmtstr++;
			placeholder.size_modifier = sizeof(size_t);
			break;

		case 'L' :	//unsupported long double
		case 'j' :	//unsupported intmax_t
			PRNF_ASSERT(false);
	};

	//Get type
	switch(FMTRD(fmtstr))
	{
		case 'd' :
		case 'i' :
			placeholder.type = TYPE_INT;
			break;
		
		case 'o' :
			placeholder.type = TYPE_BIN;
			break;

		case 'u' :
			placeholder.type = TYPE_UINT;
			break;

		case 'x' :
		case 'X' :
			placeholder.type = TYPE_HEX;
			break;

#ifdef PRNF_SUPPORT_FLOAT
		case 'f' :
		case 'F' :
			placeholder.type = TYPE_FLOAT;
			break;

		case 'e' :
		case 'E' :
			placeholder.type = TYPE_ENG;
			break;
#endif

		case 'S' :
		#ifdef __AVR__
			placeholder.type = TYPE_PSTR;
			break;
		#endif
		case 's' :
			placeholder.type = TYPE_STR;
			break;

		#ifdef prnf_free
		case 'n' :
			placeholder.type = TYPE_NSTR;
			break;
		#endif

		case 'c' :
			placeholder.type = TYPE_CHAR;
			break;
		
		default :
			PRNF_ASSERT(false);	//unsupported type
			break;
	};
	fmtstr++;

	//ignore zero flag for int types when precision is specified
	if(placeholder.prec_specified && is_type_int(placeholder.type))
		placeholder.flag_zero = false;

	*dst = placeholder;
	return fmtstr;
}

static void print_placeholder(struct out_struct* out_info, union varg_union varg, struct placeholder_struct* placeholder)
{
	#ifdef PRNF_SUPPORT_FLOAT
		struct eng_struct eng;
	#endif

	if(placeholder->type == TYPE_INT || placeholder->type == TYPE_UINT || placeholder->type == TYPE_HEX)
		print_hex_dec(out_info, placeholder, varg.prnf_l);
	else if(placeholder->type == TYPE_BIN)
		print_bin(out_info, placeholder, varg.prnf_ul);
#ifdef PRNF_SUPPORT_FLOAT
	else if(placeholder->type == TYPE_FLOAT)
		print_float(out_info, placeholder, varg.f, NO_PREFIX);

	else if(placeholder->type == TYPE_ENG)
	{	
		eng = get_eng(varg.f);
		print_float(out_info, placeholder, eng.value, eng.prefix);
	}
#endif
	else if(placeholder->type == TYPE_CHAR)
		out_char(out_info, varg.c);

	else if(placeholder->type == TYPE_STR)
		print_str(out_info, placeholder, varg.str, IS_NOT_PGM);

	#ifdef __AVR__
	else if(placeholder->type == TYPE_PSTR)
		print_str(out_info, placeholder, varg.str, IS_PGM);
	#endif
	#ifdef prnf_free
	else if(placeholder->type == TYPE_NSTR)
	{
		print_str(out_info, placeholder, varg.str, IS_NOT_PGM);
		prnf_free(varg.str);
	};
	#endif
}

//Handles both signed and unsigned integers, and hex 
static void print_hex_dec(struct out_struct* out_info, struct placeholder_struct* placeholder, prnf_long_t value)
{
	prnf_ulong_t uvalue;
	int number_len;
	int field_size; 		// (number digits + 0 padding for precision + sign character)
	int zero_pad_len = 0;
	char sign_char = 0;
	char txt[INT_BUF_SIZE];
	char* txt_ptr;
	bool sign_char_already_output = 0;

	if(is_type_unsigned(placeholder->type))
		uvalue = (prnf_ulong_t)value;
	else
	{
		uvalue = value;
		if(value < 0.0)
		{
			sign_char = '-';
			uvalue = -value;
		}
		else if(placeholder->sign_pad)
			sign_char = placeholder->sign_pad;
	};

	if(placeholder->type == TYPE_HEX)
		number_len = ulong2asc_revhex(txt, uvalue);
	else 
		number_len = ulong2asc_revdec(txt, uvalue);

	txt_ptr = &txt[number_len];

	//if more digits required, determine amount of zero padding to meet precision
	if(placeholder->prec_specified && placeholder->prec > number_len)
		zero_pad_len = placeholder->prec - number_len;
 
	field_size = number_len + zero_pad_len + !!sign_char;

	//If there will be a sign character, and width prepadding is with '0', output the sign character first
	if(sign_char && placeholder->flag_zero)
	{
		out_char(out_info, sign_char);
		sign_char_already_output = true;
	};

	//prepad number length to satisfy width  (if specified)
	prepad(out_info, placeholder, field_size);

	if(sign_char && !sign_char_already_output)
		out_char(out_info, sign_char);
	while(zero_pad_len--)
		out_char(out_info, '0');
	do
	{
		txt_ptr--;
		out_char(out_info, *txt_ptr);
	}while(txt_ptr != txt);

	//postpad number length to satisfy width  (if specified)
	postpad(out_info, placeholder, field_size);
}

static void print_bin(struct out_struct* out_info, struct placeholder_struct* placeholder, prnf_ulong_t uvalue)
{
	int number_len = 0;
	int zero_pad_len = 0;
	prnf_ulong_t temp_ulong = uvalue;
	prnf_ulong_t bit = 0;
	int precision = 1;

	while(temp_ulong)
	{
		number_len++;
		temp_ulong >>= 1;
	};

	if(placeholder->prec_specified)
		precision = placeholder->prec;
	
	if(precision > number_len)
		zero_pad_len = precision - number_len;
 
	//prepad number length to satisfy width  (if specified)
	prepad(out_info, placeholder, number_len + zero_pad_len);

	while(zero_pad_len--)
		out_char(out_info, '0');

	if(number_len)
		bit = (prnf_ulong_t)1 << (number_len-1);
	while(bit)
	{
		out_char(out_info, (uvalue & bit) ? '1':'0');
		bit >>= 1;
	};

	//postpad number length to satisfy width  (if specified)
	postpad(out_info, placeholder, number_len + zero_pad_len);
}

#ifdef PRNF_SUPPORT_FLOAT
//	With precision of 3 printable range is +/- 4294967.295
static void print_float(struct out_struct* out_info, struct placeholder_struct* placeholder, prnf_float_t value, char postpend)
{
	const char* out_msg;

	out_msg = determine_float_msg(placeholder, value);
	if(out_msg)
		print_float_special(out_info, placeholder, out_msg, value);
	else
		print_float_normal(out_info, placeholder, value, postpend);
}

// Return FLOAT_CASE_x
static const char* determine_float_msg(struct placeholder_struct* placeholder, prnf_float_t value)
{
	const char* retval = NULL;
	int prec;

	if(value < 0.0F)
		value = -value;

	// Determine special case messages
	if(value != value)
		retval = "NAN";
	else if(value > FLT_MAX)
		retval = "INF";

	// If not NAN or INF
	if(!retval)
	{
		// Multiply by 10^prec to move fractional digits into the integral digits
		prec = get_prec(placeholder);
		value *= pow10_tbl[prec];

		// Check within printable range
		if(value >= (prnf_float_t)PRNF_ULONG_MAX)
			retval = "OVER";
	};

	return retval;
}

static void print_float_normal(struct out_struct* out_info, struct placeholder_struct* placeholder, prnf_float_t value, char postpend)
{
	uint_least8_t prec;
	uint_least8_t number_len = 1;
	uint_least8_t radix_pos = -1;
	prnf_ulong_t uvalue;
	char sign_char;
	char txt[INT_BUF_SIZE];
	char* txt_ptr;
	bool sign_char_already_output = false;

	sign_char = determine_sign_char_of_float(placeholder, value);

	// Multiply by 10^prec to move fractional digits into the integral digits
	prec = get_prec(placeholder);

	if(value < 0.0F)
		value = -value;
	value *= pow10_tbl[prec];
	uvalue = round_float_to_ulong(value);

	//determine number of digits
	number_len = ulong2asc_revdec(txt, uvalue);
	txt_ptr = &txt[number_len];

	//minimum number of digits is .precision +1 as we always print a 0 on the left of the decimal point
	while(number_len < prec+1)
	{
		number_len++;
		*txt_ptr++ = '0';
	};

	if(prec)
	{
		number_len++;	//+1 for decimal point
		radix_pos = number_len - prec - 1;
	};

	if(sign_char)		//+1 for sign character
		number_len++;

	if(postpend && uvalue)		//+1 for engineering notaion?
		number_len++;

	//If there will be a sign character, and width prepadding is with '0', output the sign character first
	if(sign_char && placeholder->flag_zero)
	{
		out_char(out_info, sign_char);
		sign_char_already_output = true;
	};

	//prepad number length to satisfy width  (if specified)
	prepad(out_info, placeholder, number_len);

	if(sign_char && !sign_char_already_output)
		out_char(out_info, sign_char);
	
	do
	{
		if(radix_pos-- == 0)
			out_char(out_info, '.');
		txt_ptr--;
		out_char(out_info, *txt_ptr);
	}while(txt_ptr != txt);

	if(postpend && uvalue)
		out_char(out_info, postpend);

	//postpad number length to satisfy width  (if specified)
	postpad(out_info, placeholder, number_len);
}

// Floating point special cases
// "NAN" , "-INF" , "INF" , "+INF" , " INF" , "-OVER" , "OVER" , "+OVER" , " OVER"
static void print_float_special(struct out_struct* out_info, struct placeholder_struct* placeholder, const char* out_msg, prnf_float_t value)
{
	struct placeholder_struct ph = *placeholder;
	int msg_len;
	char sign_char;

	sign_char = determine_sign_char_of_float(placeholder, value);

	ph.type = TYPE_STR;
	ph.prec = 1;	// Must be non-0 or prepad/postpad may center the msg which is not what we want
	msg_len = prnf_strlen(out_msg, false, INT_MAX);
	if(sign_char)
		msg_len++;

	//prepad length to satisfy width (if specified)
	prepad(out_info, &ph, msg_len);
	if(sign_char)
		out_char(out_info, sign_char);
	while(*out_msg)
		out_char(out_info, *out_msg++);
	postpad(out_info, &ph, msg_len);
}

// +, - , <space> or 0
static char determine_sign_char_of_float(struct placeholder_struct* placeholder, prnf_float_t value)
{
	char sign_char = 0;
	bool neg = (value < 0.0F);
	bool nan = (value != value);

	// Determine sign character
	if(neg)
		sign_char = '-';
	else if(!nan)
	{
		if(placeholder->sign_pad)
			sign_char = placeholder->sign_pad;
	};

	return sign_char;
}
#endif	//^PRNF_SUPPORT_FLOAT^

static void print_str(struct out_struct* out_info, struct placeholder_struct* placeholder, const char* str, bool is_pgm)
{
	int	source_len;
	int cnt;

	source_len = prnf_strlen(str, is_pgm, INT_MAX);

	if(placeholder->prec_specified && placeholder->prec < source_len && !is_centered_string(placeholder))
		source_len = placeholder->prec;

	prepad(out_info, placeholder, source_len);

	cnt = source_len;
	while(cnt--)
	{
		out_char(out_info, FMTRD(str));
		str++;
	};

	postpad(out_info, placeholder, source_len);
}

#ifdef PRNF_COL_ALIGNMENT
// print colum alignment  \v<col><pad char>
// if \v is encountered without <col> output \v
// if \v is encountered with <col>, but <pad char> is the string terminator, output nothing
static const char* print_col_alignment(struct out_struct* out_info, const char* fmtstr, bool is_pgm)
{
	int col = 0;
	char pad_char;
	bool got_col = false;

	got_col = prnf_is_digit(FMTRD(fmtstr));
	col = prnf_atoi(&fmtstr, is_pgm);
	pad_char = FMTRD(fmtstr);

	if(pad_char < 0x20)
		pad_char = 0;

	if(!got_col)
		out_char(out_info, '\v');
	else if(pad_char)
	{
		while(out_info->col < col)
			out_char(out_info, pad_char);
		fmtstr++;
	};

	return fmtstr;
}
#endif

static int prnf_strlen(const char* str, bool is_pgm, int max)
{
	(void)is_pgm;
	int retval = 0;

	if(str)
	{
		while(FMTRD(str) && retval < max)
		{
			str++;
			retval++;
		};
	};
	return retval;
}

static int prnf_atoi(const char** fmtstr, bool is_pgm)
{
	(void)is_pgm;
	int value = 0;

	//Get width
	while(prnf_is_digit(FMTRD(*fmtstr)))
	{
		value *=10;
		value += FMTRD(*fmtstr)&0x0F;
		(*fmtstr)++;
	};

	return value;
}

// prepad the output to achieve width, if needed
static void prepad(struct out_struct* out_info, struct placeholder_struct* placeholder, int source_len)
{
	int pad_len = 0;
	char pad_char = ' ';

	if(is_centered_string(placeholder) && source_len < placeholder->width)
		pad_len = (placeholder->width - source_len)/2;

	// if not left aligned, and the required width is longer than the source length
	else if(!placeholder->flag_minus && placeholder->width > source_len)
		pad_len = placeholder->width - source_len;

	// prepad character of '0' is specified for a numeric type
	if(is_type_numeric(placeholder->type) && placeholder->flag_zero)
		pad_char = '0';

	while(pad_len--)
		out_char(out_info, pad_char);
}

// postpad the output to achieve width, if needed
static void postpad(struct out_struct* out_info, struct placeholder_struct* placeholder, int source_len)
{
	int pad_len = 0;

	if(is_centered_string(placeholder) && source_len < placeholder->width)
	{
		pad_len = (placeholder->width - source_len);
		if(pad_len & 1)
			pad_len++;	// round up, prefer larger postpad if unable to center
		pad_len >>=1;
	}

	// if left aligned, and the required width is longer than the source length
	else if(placeholder->flag_minus && placeholder->width > source_len)
		pad_len = placeholder->width - source_len;
	
	while(pad_len--)
		out_char(out_info, ' ');
}

//string type with .precision of 0?
static bool is_centered_string(struct placeholder_struct* placeholder)
{
	return ((placeholder->type == TYPE_STR || placeholder->type == TYPE_PSTR)
	 && !placeholder->prec_is_dynamic
	 && placeholder->width
	 && placeholder->prec_specified
	 && placeholder->prec==0);
}

//return true for only unsigned integer types
static bool is_type_unsigned(uint_least8_t type)
{
	return (type==TYPE_UINT || type==TYPE_HEX || type==TYPE_BIN);
}

//return true for both signed and unsigned integer types
static bool is_type_int(uint_least8_t type)
{
	return (type==TYPE_UINT || type==TYPE_INT || type==TYPE_HEX || type==TYPE_BIN);
}

//return true for any numeric type
static bool is_type_numeric(uint_least8_t type)
{
	return 	(type==TYPE_BIN || type==TYPE_INT || type==TYPE_UINT || type==TYPE_HEX || type==TYPE_FLOAT || type==TYPE_ENG);
}

static bool prnf_is_digit(char x)
{
	return ('0' <=x && x <= '9');
}

static char ascii_hex_digit(uint_least8_t x)
{
	char retval;

	x &= 0x0F;
	if(x < 0x0A)
		retval = '0'+x;
	else
		retval = 'A'+(x-0x0A);
	
	return retval;
}

#ifdef PRNF_SUPPORT_FLOAT
static struct eng_struct get_eng(prnf_float_t value)
{
	struct eng_struct retval;
	uint8_t i = 8;
	static const char tbl[17] = {'y', 'z', 'a', 'f', 'p', 'n', 'u', 'm', 0, 'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y'};

	while(!(-1000.0F < value && value < 1000.0F) && i < 16)
	{
		i++;
		value /= 1000.0F;
	};

	while((-1.0F < value && value < 1.0F) && i)
	{
		i--;
		value *= 1000.0F;
	};

	retval.value = value;
	retval.prefix = tbl[i];
	return retval;
}

static uint_least8_t get_prec(struct placeholder_struct* placeholder)
{
	uint_least8_t prec;

	if(placeholder->prec_specified)
		prec = placeholder->prec;
	else if(placeholder->type == TYPE_ENG)
		prec = PRNF_ENG_PREC_DEFAULT;
	else
	 	prec = PRNF_FLOAT_PREC_DEFAULT;

	if(prec > FLOAT_PREC_MAX)
	{
		PRNF_ASSERT(false);
		prec = FLOAT_PREC_MAX;	//limit precision if no assertion handler is available
	};

	return prec;
}

static prnf_ulong_t round_float_to_ulong(prnf_float_t x)
{
	prnf_ulong_t retval;

	retval = (prnf_ulong_t)x;

	if(x - (prnf_float_t)retval >= 0.5)
		retval++;

	return retval;
}
#endif  //^PRNF_SUPPORT_FLOAT^


// Number conversion macro, allows better optimisation of % and / operations on literal values.
// Drops to int arithmetic ASAP to improve performance
#define ulong2asc_base(buf, il, base)	\
do										\
{										\
	while(il > UINT_MAX)				\
	{									\
		*buf++ = to_digit(il % base);	\
		il = il / base;					\
		digit_count++;					\
	};									\
	i = il;								\
	while(i)							\
	{									\
		*buf++ = to_digit(i % base);	\
		i = i / base;					\
		digit_count++;					\
	};									\
	if(!digit_count)					\
	{									\
		*buf++ = '0';					\
		digit_count++;					\
	};									\
}while(false)

//convert prnf_ulong_t to decimal reversed
static uint_least8_t ulong2asc_revdec(char* buf, prnf_ulong_t il)
{
	uint_least8_t digit_count = 0;
	unsigned int i;
	#define to_digit(x) ('0'+(x))
	ulong2asc_base(buf, il, 10);
	#undef to_digit
	return digit_count;
}

//convert prnf_ulong_t to hex reversed
static uint_least8_t ulong2asc_revhex(char* buf, prnf_ulong_t il)
{
	uint_least8_t digit_count = 0;
	static const char hex_digits[sizeof(HEX_DIGITS_STRING)] = HEX_DIGITS_STRING;
	unsigned int i;
	#define to_digit(x) (hex_digits[(x)])
	ulong2asc_base(buf, il, 16);
	#undef to_digit
	return digit_count;
}

// Per-character output processing
// Counts output characters (regardless of truncation)
// Calls function pointer to destinations character output OR writes to buffer
// Truncates buffer output
// Detects line endings and tracks colum (1st column is 0)
static void out_char(struct out_struct* out_info, char x)
{
	if(out_info->buf && out_info->char_cnt+1 < out_info->size_limit)
		*(out_info->buf++) = x;

	else if(out_info->dst_fptr)
		out_info->dst_fptr(out_info->dst_fptr_vars, x);

	#ifdef PRNF_COL_ALIGNMENT
		if(x=='\r' || x=='\n')
				out_info->col = 0;
		else if(x > 0x1F)
			out_info->col++;
	#endif

	out_info->char_cnt++;
}

// possibly terminates output
static void out_terminate(struct out_struct* out_info)
{
	if(out_info->buf && out_info->size_limit)
		*(out_info->buf) = 0;
}

#endif //FIRST_PASS


// Compile _P version for PROGMEM access
#undef FMTRD
#ifdef __AVR__
	#ifdef FIRST_PASS
		#undef FIRST_PASS
		#undef IS_SECOND_PASS
		#define SECOND_PASS
		#define IS_SECOND_PASS true
		#include "prnf.h"
	#endif
#endif

#endif // PRNF_IMPLEMENTATION