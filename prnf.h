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

Provide column alignment using \v (see readme.md)
	-DPRNF_COL_ALIGNMENT

Include a file which may provide configuration for extensions and error handling (see sample).
	-DPRNF_CFG_FILE=prnf_cfg.h


-------------------------------------------------------------------------------------
# PRNF

 A lightweight printf implementation.
 With some reasonable limitations, and non-standard behavior suited to microcontrollers.
 
 * Thread and re-entrant safe.
 * Low stack & ram usage, zero heap usage.
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
