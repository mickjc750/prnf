//********************************************************************************************************
// PRNF Implementation
//********************************************************************************************************
/*
If extensions are not used, this file need only be 2 lines:
    #define PRNF_IMPLEMENTATION
    #include "prnf.h"

Otherwise before including "prnf.h" you have the options to provide a memory allocator, assert macro, and warning handler.

Other build options such as PRNF_SUPPORT_FLOAT, PRNF_SUPPORT_LONG_LONG etc.. (see prnf.h) may also be defined here
 if you do not want to define them in your build configuration.

See the below example.
*/


/*
	To enable extensions (%n), you must tell prnf how to free the allocated strings passed to %n
        * include a memory allocator,
        * #define prnf_free()
******************************************************************************************/
	#include <stdlib.h>
	#define prnf_free(arg) 		free(arg)


/*	If you have a runtime warning handler, include it here and define PRNF_WARN to be your handler.
 *  A 'true' argument is expected to generate a warning.
 *****************************************************************************************/
//	#include "my_warning_handler.h"
//	#define PRNF_WARN(arg) my_warning_handler(arg)


/*	If you have an assertion handler, include it here and define PRNF_ASSERT to be your handler.
 *  A 'false' argument is expected to generate an error.
 *****************************************************************************************/
	#include <assert.h>
	#define PRNF_ASSERT(arg) assert(arg)


/*	Finally, include the prnf impementation.
 *****************************************************************************************/
    #define PRNF_IMPLEMENTATION
    #include "prnf.h"
