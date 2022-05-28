/*
*/

	#include <stdint.h>
	#include "version.h"

//********************************************************************************************************
// Public variables
//********************************************************************************************************

#ifdef PLATFORM_AVR
	#define SLMEM PROGMEM
	#include <avr/pgmspace.h>
#else
	#define SLMEM
#endif

	const char version_name[] SLMEM = VERSION_NAME;

	const uint32_t version_build_number = 
	#include "build_number.inc"
	;

	const char version_build_date[] SLMEM =
	#include "build_date.inc"
	;

	const char version_gcc_version[] SLMEM =
	#include "gcc_version.inc"
	;

	const uint8_t version_numbers[3] = {VERSION_NUMBERS};	// 0:major 1:minor 2:patch
	const char version_string[] SLMEM = VERSION_STRING;
