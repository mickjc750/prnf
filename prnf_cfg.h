//********************************************************************************************************
// PRNF Configuration
//********************************************************************************************************

/*	To enable extensions (%n), uncomment this #define, and include your memory allocator.
******************************************************************************************/
	#define PRNF_SUPPORT_EXTENSIONS
	#include <stdlib.h>
	#define prnf_free(arg) 		free(arg)
	#define prnf_malloc(arg) 	malloc(arg)


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
