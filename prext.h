/*
 prnf() extensions

 To use:

 1. Ensure you have enabled SUPPORT_EXTENSIONS in prnf_cfg.h and defined prnf_malloc() and prnf_free()

 2. Include prext.h and prext.c in your build.

 The functions in this module should only be used as arguments in calls to prnf() (and friends), and associated with the type specifier %n.
 They will allocate memory on the heap to store a C string, which will then be freed within prnf() after printing.

 Example:
 	prnf("Age of Fred is %n\n", prext_period(age_of_fred));

 Note that the %n specifier does not allow width or precision.
 So any variables which need to affect the output of the extension must be passed as parameters to the extension itself.

 This header is an example only, and is expected to be extended with various prext_mything(mything_t mything) functions specific to your application

*/

#ifndef _PRNF_EXT_H_
#define _PRNF_EXT_H_

	#include <stdint.h>
	#include <complex.h>

//********************************************************************************************************
// Public prototypes
//********************************************************************************************************

//	Formatted time stamp using strftime()
	int* prext_tstamp(const char* fmt);

//	Describe period in the form XXy XXd XXh XXm XXs
	int* prext_period(uint32_t seconds);

//	Complex number in rectangular form
	int* prext_cplex_rec(complex float c);

//	Complex number in polar form (radians)
	int* prext_cplex_pol(complex float c);

#endif // _PRNF_H_
