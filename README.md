 # PRNF
 A lightweight printf implementation.
 With some reasonable limitations, and non-standard behavior suited to microcontrollers.

  If you need a full featured printf(), I strongly recommend checking out eyalroz's fork
 of mpaland's printf(), which can be found [here](https://github.com/eyalroz/printf).
  After some involvement in that project, I realized it would never quite be exactly what I wanted.
 So I wrote this. Some of the ideas used in this are from the authors of that project.
 Particularly the function pointer output from mpaland, and the output wrapping used in this was suggested by eyalroz.

 * Single header (stb style).
 * Thread and re-enterant safe.
 * Low stack & ram usage, zero heap usage (unless extensions are enabled).
 * Full support for AVR's PROGMEM requirements.
 * Compatible enough to make use of GCC's format and argument checking (even for AVR).
 * Optional support for long long types.
 * Optional support for float or double.
 * Full test suite using Greatest.

 * NO exponential form, %e provides SI units (y z a f p n u m - k M G T P E Z Y).
 * NO Octal, %o outputs binary instead.
 * NO adaptive %g %G
 
 If you're a linux user, for a demonstration you can type 'make' then './demo' in the /demo folder.
 
<br>
 
 
 Standard placeholder syntax:

    %[flags][width][.precision][length]type



 Supported [flags]:

    -       left align the output
     
    +       prepends a + for positive numeric types
     
    (space) prepends a space for positive numeric type

    0       prepends 0's instead of spaces to numeric types to satisfy [width]


 Unsupported [flags]:

    #       If you want 0x it needs to be in your format string.

    ' (apostrophe)	No 1000's separator is available


 [width]
 
    The minimum number of characters to output.
    If width is specified as * A dynamic width must be provided as an int argument preceding the argument to be formatted.
    A non-standard method of centering text is also provided (see below).


 [.precision]

    For float %f, this is the number of fractional digits after the '.', valid range is .0 - .8
    For integers, this will prepend 0's (if needed) until the total number of digits equals .precision
    For strings %s %S, This is the maximum number of characters to read from the source.

    If precision is specified as .* A dynamic precision must be provided as int argument preceding the argument to be formatted.

    Centering strings: If [width] is specified and precision is .0 %s arguments will be centered.
    Example to center within a 16 character LCD would be "%16.0s"
    **Caution - If you are generating formatting strings at runtime, and generate a %[width].0s, you will NOT get 0 characters.
    A dynamic precision of 0 provided to .* will not centre the string.

 Supported [length]:
 
    Used to specify the size of the argument. The following is supported:
    hh  Expect an int-sized argument which was promoted from a char.
    h   Expect an int-sized argument which was promoted from a short.
    l   Expect a long-sized argument.
    ll  Expect a long long sized argument. ** DISABLED BY DEFAULT, YOU MUST #define PRNF_SUPPORT_LONG_LONG in prnf_conf.h
    z   Expect a size_t sized argument.
    t   Expect a ptrdiff_t sized argument.

 Unsupported [length]:

    j   intmax_t not supported


 Supported types:

    d,i     Signed decimal integer
    u       Unsigned decimal integer
    x,X     Hexadecimal. Always uppercase, .precision defaults to argument size [length]
    o       NOT Octal. Actually binary, .precision defaults to argument size [length]
    s       null-terminated string in ram, or NULL. Outputs nothing for NULL.
    S       For AVR targets, read string from PROGMEM, otherwise same as %s
    c       character 

    f,F     Floating point (optional), enable PRNF_SUPPORT_FLOAT or PRNF_SUPPORT_DOUBLE in prnf_conf.h
            NAN & INF are always uppercase.
            Default precision is 3 (not 6).
            Digits printed must be able to be represented by an unsigned long (or unsigned long long if supported).
            ie. with a precision of 3 using 32bit long values, maximum range is +/- 4294967.296 
            ie. with a precision of 3 using 64bit long or long long values, maximum range is +/- 18446744073709551.615
            Values outside this range will produce "OVER".
            A value of 0.0 is always positive. 

    e       NOT exponential. Floating point with engineering notation (y z a f p n u m - k M G T P E Z Y).
            Number is postpended with the SI prefix. Default precision is 0.
 

 Unsupported types:

    g,G     Adaptive floats not available
    a,A     Double in hex notation not available
    p       Pointer not available
    n       classic %n is not available, but %n may be repurposed for extensions if enabled (see below)

<br>
<br>

 # Configuration
For the minimum default configuration, place the following 2 lines in a single .c file:

    #define PRNF_IMPLEMENTATION
    #include "prnf.h"

Optionally, you may provide an error handler by defining PRNF_ASSERT(), which will be called if prnf encounters an unsupported or unknown placeholder type (ie. %G). 

	#include <assert.h>
	#define PRNF_ASSERT(arg) assert(arg)

You may also provide a warning handler by defining PRNF_WARN(), which will be called if prnf encounters an unsupported flag (# or ').

	#include "my_warning_handler.h"
	#define PRNF_WARN(arg) my_warning_handler(arg)

 If you plan to use extensions, you may provide prnf with the means to free heap allocated strings (passed to %n) by defining prnf_free().
 
 	#include <stdlib.h>
	#define prnf_free(arg) 		free(arg)

See the example prnf.c files in either the demo or test folder.
<br>
<br>

# Build options
prnf has the following build options, which are enabled by defining symbols, either by adding -DSymbolName to compiler flags, or by defining the symbols in the same .c file containing #define PRNF_IMPLEMENTATION. These options are:
<br>

Support floating point, provides %f and %e placeholders

	PRNF_SUPPORT_FLOAT

Double arguments will not be demoted to float, and prnf will use double arithmetic.

	PRNF_SUPPORT_DOUBLE

Support long long

	PRNF_SUPPORT_LONG_LONG

Default precision for %e (engineering) notation

	PRNF_ENG_PREC_DEFAULT=0

Default precision for %f (floats)

	PRNF_FLOAT_PREC_DEFAULT=3

Provide column alignment using \v

	PRNF_COL_ALIGNMENT





<br>
<br>

 # AVR's PROGMEM support:
On AVR devices prnf.c will #include itself for a second pass, to build the _P versions of the functions.
If you wish to write code which is cross compatible with AVR and non-AVR, use the _SL (String Literal) macros provided,
these will place string literals in PROGMEM for AVR targets and produce normal string literals for 'normal' von-newman targets.


On AVR, both the the format string, and string arguments, may be in either ram or program memory.

 Unlike avr-libc, prnf DOES provide argument type checking using some macro tricks. This works by putting a do-nothing function which takes format checking into a while(0) loop. The compiler will perform the format checking, then remove the do-nothing function, providing some level of optimization is enabled.


   
<br>
<br>

# Default output for prnf()

Create a function which handles a single character, with the following signature and name. Providing this default character handler is optional.

    void prnf_putch(void* x, char c)  
    {  
        (void)x;	// As x is not used, this line may be needed to avoid a compiler warning.
        ... Do stuff here to output character c, ie. write to a uart or lcd.
    }  

<br>
<br>

# Specifying the output handler when calling prnf()

A non-standard function is available which can be used for stream-like printing to different destinations.

    int fptrprnf(void(*out_fptr)(void*, char), void* out_vars, const char* fmtstr, ...);

So to print to the above example of my_character_handler:

    fptrprnf(prnf_putch, NULL, "Hello Fred is %i years old\n", freds_age);

If you have specific information to pass to your character handler, you can pass the address of it instead of NULL, and it will be passed to the void* x in my_character_handler.

<br>
<br>


# Printing to text buffers

The usual functions are available (with print shortened to prn), and return a character count (disregarding any truncation).

    int sprnf(char* buffer, const char* format, ...) 
    int snprnf(char* buffer, size_t buffer_size, const char* format, ...) 

<br>
<br>

# Example debug macro:
The following is useful for debug/diagnostic and cross platform friendly with AVR:

    #define DBG(_fmtarg, ...) prnf_SL("%s:%.4i - "_fmtarg , __FILE__, __LINE__ ,##__VA_ARGS__)

Example usage:

    DBG("value is %i\n", 51);
The above will output something like "main.c:0113 - value is 51"

Note that the \_\_FILE\_\_ string literal should not be placed in PROGMEM. AVR-GCC does not offer string pooling for PROGMEM strings, so using PSTR(\_\_FILE\_\_) on many lines would consume a lot of program memory space.

<br>
<br>

# Adding prnf() functionality to other IO modules:

This can easily be achieved by writing your own character handler, and variadic function for the module.
Then the variadic version of fptrprnf (vfptrprnf) can be used to print to your character handler.

An example for lcd_prnf() may look something like:

    #include <stdarg.h>

    // LCD prnf character handler
    static void prnf_write_char(void* nothing, char x)
    {
        lcd_data(x); // (function or code to write a single character to the LCD)
    }

    // formatted print to LCD
    void lcd_prnf(const char* fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        vfptrprnf(prnf_write_char, NULL, fmt, va);
        va_end(va);
    }

<br>
<br>

# snappf()
Safely APPEND to a string in a buffer of known size.

    int snappf(char* dst, size_t dst_size, const char* fmtstr, ...);

In addition to the regular snprnf(), which is used for safely printing to a buffer of n bytes. snappf() can safely append to the buffer.
This is useful in situations where you may need to iterate through a number of fields in a loop. The return value is the number of characters appended (ignoring truncation).

<br>
<br>


# Column alignment


To enable this feature put -DPRNF_COL_ALIGNMENT in your compiler flags, or #define PRNF_COL_ALIGNMENT in the same file as #define PRNF_IMPLEMENTATION<br>

It is possible to advance output to a specific column, with respect to the start of the output, or the last line ending. To achieve this prnf hijacks the \v (vertical tab) character.<br>
The required format is:

    \v<col><pad character>


\v should be followed by a decimal number indicating the column on which the following text will start on (with 0 being the first column). The character used for padding is the first non-numeric character after this number. Note that due to this, digits cannot be used as the padding character. This feature is useful if you have output which contains fields of uncontrolled length, and then wish to align further output. If the current column is already at or past the column specified, then no padding will be applied. If \v occurs in your string without being followed by digits, then a regular \v character will be output.<br>
Example:

    prnf("%s:%.4i(%s)\v30 %s\n", __FILE__, __LINE__, __func__, "This text starts on column 30");

Will yield something like:

    myfile.c:0041(main)         This text starts on column 30

<br>
It can also be used as an easy way to create banners, for example:

    prnf("\v30*\n Main Menu\n\v30*\n");

Will yield:

    ******************************
     Main Menu
    ******************************


<br>
<br>

# Extending prnf

 Most applications need to print things beyond what is offered by the standard placeholder types. This feature provides an easy way of doing that, but it requires the heap (or some other dynamic memory allocator) to work.

 When enabled, prnf() will accept a %n as a C string (like %s), only it will free() the address after printing. The argument type is int*, even though it points to a string. This allows the user to create functions which produce whatever strings they need (coordinates, timestamps, etc..), and then use these functions as arguments to prnf(). Example:

    static int* prext_bananas(int bananas)
    {
        int txt_size = 25;
        char* txt = malloc(txt_size);
        if(bananas > 10)
            snprnf(txt, txt_size, "LOTS OF BANANAS!");
        else if(bananas > 1)
            snprnf(txt, txt_size, "A few bananas");
        else if (bananas)
            snprnf(txt, txt_size, "Only one banana");
        else 
            snprnf(txt, txt_size, "NO BANANAS! :-(");
        return (int*)txt;
    }

    prnf("Gorilla Fred has %n and Uncle Bob has %n\n", prext_bananas(freds_bananas), prext_bananas(bobs_bananas));

As these functions need to return an int* to a string on the heap, their name should indicate that their sole purpose is
 to provide arguments to prnf() for %n placeholders. The name prefix of prext_ is suggested.
Note that if you mistakenly mix up %s and %n, you will get a compilation warning (a good thing), as %s expects a char* and %n expects an int*

<br>

# How to enable extensions

To enable extensions, provide prnf() with the prnf_free() function needed to free the strings. This is done in the .c file containing #define PRNF_IMPLEMENTATION (see configuration above). Example:

 	#include <stdlib.h>
	#define prnf_free(arg) 		free(arg)
    
    #define PRNF_IMPLEMENTATION
    #include "prnf.h"


<br>

# Questions regarding extensions:

Q. Why cannibalize %n when there are so many letters left in the alphabet?

1. GCC (and probably most compilers) will check the format string and issue a warning "unknown specifier %"
2. Unsupported specifiers offer no argument type checking.
3. The standard behavior of %n does not appear to be useful, (if you do use it, please share why).

Q. Why not just make functions to output the custom strings instead of passing them to prnf?

  A. Consider you use vprnf to create yourself a uart_prnf() lcd_prnf(), and also use snprnf(), snappf(), and fptrprnf().
You can pass your custom strings to any of these functions.

<br>
<br>



<br>
<br>

Please raise an issue if you find a bug (unlikely), or even if you find any of the above instructions confusing.
I'm willing to help.
