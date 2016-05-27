/*
* FILE : main.c
* PROJECT : PROG1165 - Special Project
* PROGRAMMER : Verdi Rodrigues-Diamond
* FIRST VERSION : 2016-05-19
* DESCRIPTION :
* The functions in this file are used to …
*/ 

/// @todo break functions into their appropriate files
/// @todo create function prototypes
/// @todo create test cases for validation and utility functions
/// @todo create command line parameter or other function for configuring logging output
/// @todo check and remove any magic numbers
/// @todo verify working on windows

#include <regex>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "main.h"

// Polyfill for lack of C99 support in Visual Studio
// https://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010

#if defined(_MSC_VER) && _MSC_VER < 1900

#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf

__inline int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
	int count = -1;

	if (size != 0)
		count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
	if (count == -1)
		count = _vscprintf(format, ap);

	return count;
}

__inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
	int count;
	va_list ap;

	va_start(ap, format);
	count = c99_vsnprintf(outBuf, size, format, ap);
	va_end(ap);

	return count;
}

#endif

// Utility Constants
#define TRUE 1
#define FALSE 0

#define EOL "\n"
#define EOL_LENGTH 1

#define PATH_MAX_LEN 200
#define TIME_MAX_LEN 80

#define BUFFER_MAX_LEN 4096
#define BUFFER_WAIT_FOR_KEY 2


#ifdef WIN32
#define snprintf _snprintf // Remember #define _CRT_SECURE_NO_WARNINGS and multibyte encoding
#endif

/*
#ifdef WIN32
#define EOL "\r\n" // Does not work as expected
#define EOL_LENGTH 2
#else
#define EOL "\n"
#define EOL_LENGTH 1
#endif
*/

// Constants / Business variables

enum ADDRESS_BOOK_FIELDS {
	FIELD_NAME = 0,
	FIELD_STREET_ADDRESS,
	FIELD_CITY,
	FIELD_PROVINCE,
	FIELD_POSTAL_CODE,
	FIELD_PHONE_NUMBER
};

#define ADDRESS_BOOK_NUM_FIELDS 6

#define NAME_MAX_LENGTH 30 + EOL_LENGTH
#define STREET_ADDRESS_MAX_LENGTH 60 + EOL_LENGTH
#define CITY_MAX_LENGTH 60 + EOL_LENGTH
#define PROVINCE_MAX_LENGTH 2 + 1 + EOL_LENGTH
#define POSTAL_CODE_SEGMENT 3
#define POSTAL_CODE_MAX_LENGTH (POSTAL_CODE_SEGMENT * 2) + 1 + EOL_LENGTH
#define PHONE_NUMBER_SEGMENT_1 3
#define PHONE_NUMBER_SEGMENT_2 3
#define PHONE_NUMBER_SEGMENT_3 4
#define PHONE_NUMBER_MAX_LENGTH PHONE_NUMBER_SEGMENT_1 + PHONE_NUMBER_SEGMENT_2 + PHONE_NUMBER_SEGMENT_3 + 1 + EOL_LENGTH

#define ADDRESS_BOOK_MAX_LENGTH 10

#define ADDRESS_BOOK_PAGINATION_DEFAULT_NUM_ENTRIES 3

// Strings

const char MESSAGE_ERROR_PREFIX [] = "ERROR: ";
const char MESSAGE_ERROR_SUFFIX [] = EOL;


const char * MESSAGE_ADDRESS_BOOK_FIELD_PROMPT [] = {
	"Contact name: ",
	"Street address: ",
	"City: ",
	"Province: ",
	"Postal code: ",
	"Phone number: "
};

const char * MESSAGE_ADDRESS_BOOK_FIELD_ERROR_INVALID [] = {
	"The name enterred is not valid.",
	"The street address enterred is not valid.",
	"The city enterred is not valid.",
	"The province enterred is not valid.",
	"The postal code enterred is not valid.",
	"The phone number enterred is not valid."
};

const char MESSAGE_ADDRESS_BOOK_FIELD_ERROR_UNABLE_TO_READ_INPUT [] = "Failed to read from the input file.";

const char MESSAGE_ADDRESS_BOOK_FIELD_SKIP [] = "Skipping field.";
const char MESSAGE_ADDRESS_BOOK_CONTACT_SKIP [] = "Skipping contact.";


// Validation artifacts

// As specified in the requirements document
const char * VALID_PROVINCE_CODES [] = {
	"NL",
	"PE",
	"NS",
	"NB",
	"QC",
	"ON",
	"MB",
	"SK",
	"AB",
	"BC",
	"YT",
	"NT",
	"NU"
};

const int NUM_VALID_PROVINCE_CODES = 13;

// defines whether area codes are conditionally validated based on if they are a member
// of the valid area codes list or NOT a member of the invalid area codes list 
// TRUE = area codes must be contained in the valid area codes list
// FALSE = area codes must not be a member of the invalid area codes list
const int AREA_CODES_EXCLUSIVE = TRUE;

const int VALID_AREA_CODES [] = {
	403, 587, 780, // Alberta
	236, 250, 604, 778, // British Columbia
	204, 431, // Manitoba
	506, // New Bruinswick
	709, // Newfoundland and Labrador
	867, // Northwest Territories
	782, 902, // Nova Scotia
	867, // Nunavut
	226, 249, 289, 343, 365, 416, 437, 519, 613, 647, 705, 807, 905, // Ontario
	782, 902, // Prince Edward Island
	418, 438, 450, 514, 579, 581, 819, 873, // Quebec
	306, 639, // Saskatchewan
	867, // Yukon
	600, 622, // Non-Geographic
	800, 844, 855, 866, 877, 888 // Toll-Free Numbers
};

const int NUM_VALID_AREA_CODES = 49;

const int INVALID_AREA_CODES [] = {
	11, 101, 211, 311, 411, 511, 611, 711, 811, 911, 958, 959
};

const int NUM_INVALID_AREA_CODES = 12;

// Utility functions

//
// FUNCTION : string_to_upper
// DESCRIPTION :
// This function takes all letters in a string and makes them uppercase.
// The passed in character array must be a valid null terminated c-string or
// else segmentation fault will occur.
// PARAMETERS :
// char * str : string to make uppercase
// 
void string_to_upper (char * str) {
	int i = 0;
	for (i = 0; ; i++) {
		if (str[i] == 0) {
			break;
		}

		str[i] = toupper(str[i]);
	}
}

//
// FUNCTION : string_to_lower
// DESCRIPTION :
// This function takes all letters in a string and makes them lowercase.
// The passed in character array must be a valid null terminated c-string or
// else segmentation fault will occur.
// PARAMETERS :
// char * str : string to make lowercase
// 
void string_to_lower (char * str) {
	int i = 0;
	for (i = 0; ; i++) {
		if (str[i] == 0) {
			break;
		}

		str[i] = tolower(str[i]);
	}
}

//
// FUNCTION : string_terminate_at_first_newline
// DESCRIPTION :
// This function trims a given string to the first newline.
// The passed in character array must be a valid null terminated c-string or
// else segmentation fault will occur.
// PARAMETERS :
// char * str : string to terminate at newline
// 
void string_terminate_at_first_newline (char * str) {
	int i = 0;
	for (i = 0;; i++) {
		if (str[i] == EOL[0]) {
			str[i] = 0;
			break;
		}
		else if ( str[i] == 0 ) {
			break;
		}
	}
}

//
// FUNCTION : string_is_empty
// DESCRIPTION :
// Checks whether a string is empty. This function will provide undefined behaviour
// if given an invalid string.
// PARAMETERS :
// char * str : string to check
// RETURNS :
// int : TRUE if empty
// 	FALSE if contains data
// 

//
// FUNCTION : string_terminate_at_first_newline
// DESCRIPTION :
// This function trims a given string to the first newline.
// The passed in character array must be a valid null terminated c-string or
// else segmentation fault will occur.
// PARAMETERS :
// char * str : string to terminate at newline
// 
int string_is_empty (char * str) {
	int result = FALSE;

	if (strlen(str) == 0) {
		result = TRUE;
	}

	return result;
}

//
// FUNCTION : string_strip_leading_whitespace
// DESCRIPTION :
// This function removes any whitespace characters preceeding character data.
// The passed in character array must be a valid null terminated c-string or
// else segmentation fault will occur.
// PARAMETERS :
// char * str : string to terminate at newline
// 
void string_strip_leading_whitespace (char * str) {
	int index = 0;
	for (;;) {
		if (str[index] == 0) {
			// found end of string
			break;
		}
		else if (str[index] == ' ' || str[index] == '\t' || str[index] == '\r' || str[index] == '\n') {
			index++;
		}
		else {
			// found string characters
			break;
		}
	}

	sprintf(str, "%s", &str[index]);
}



// Data structures

// Stores data required to collect input for a given address book field.
struct field_input {
	int field; // Links into address book field enum
	char * buffer;
	int bufferSize;
	int (*validator) (struct field_input *);
};

// An entry for the address book.
struct address_book_entry {
	char name [NAME_MAX_LENGTH];
	char streetAddress [STREET_ADDRESS_MAX_LENGTH];
	char city [CITY_MAX_LENGTH];
	char province [PROVINCE_MAX_LENGTH];
	char postalCode [POSTAL_CODE_MAX_LENGTH];
	char phoneNumber [PHONE_NUMBER_MAX_LENGTH];
};

// General app configuration values such as verbosity and logging
struct app_config {
	int log;
	char logFile [PATH_MAX_LEN];
	FILE * logHandle;

	int paginationNumEntries;
};

// Stores current app context such as address book entries and configuration details.
struct app {
	struct address_book_entry addressBook [ADDRESS_BOOK_MAX_LENGTH];
	int addressBookSize;
	struct app_config config;
};

//
// FUNCTION : field_input_init
// DESCRIPTION :
// This function initializes the field_input structure with default values.
// PARAMETERS :
// struct field_input * field : field to initialize
// 
void field_input_init (struct field_input * field) {
	field->field = FIELD_NAME;
	field->buffer = NULL;
	field->bufferSize = 0;
	field->validator = NULL;
}

//
// FUNCTION : address_book_entry_init
// DESCRIPTION :
// This function initializes the address_book_entry structure with default values.
// PARAMETERS :
// struct address_book_entry * entry : entry to initialize
// 
void address_book_entry_init (struct address_book_entry * entry) {
	// Set the first byte of each c string to ASCII NULL to designate that it is an empty string
	entry->name[0] = 0;
	entry->streetAddress[0] = 0;
	entry->city[0] = 0;
	entry->province[0] = 0;
	entry->postalCode[0] = 0;
	entry->phoneNumber[0] = 0;
}


//
// FUNCTION : app_config_init
// DESCRIPTION :
// This function initializes the app_config structure with default values.
// PARAMETERS :
// struct app_config * config : config to initialize
// 
void app_config_init (struct app_config * config) {
	config->log = FALSE;
	config->logFile[0] = 0;
	config->logHandle = NULL;
	
	config->paginationNumEntries = ADDRESS_BOOK_PAGINATION_DEFAULT_NUM_ENTRIES;
}

//
// FUNCTION : app_init
// DESCRIPTION :
// This function initializes the app structure with default values.
// PARAMETERS :
// struct app * app : app to initialize
// 
void app_init (struct app * app) {
	int i = 0;
	for (i = 0; i < ADDRESS_BOOK_MAX_LENGTH; i++) {
		address_book_entry_init(&app->addressBook[i]);
	}
	app->addressBookSize = 0;

	app_config_init(&app->config);
}

// Logging

//
// FUNCTION : app_log_open
// DESCRIPTION :
// Starts application logging by creating or appending to an existing log file.
// PARAMETERS :
// struct app * app : application context
// char * logFile : system path to the log file location
// RETURNS :
// int : TRUE if log file could be opened
// 	FALSE if the log file could not be opened
// 
int app_log_open (struct app * app, char * logFile) {
	int result = FALSE;
	
	app->config.log = TRUE;
	strncpy(app->config.logFile, logFile, PATH_MAX_LEN);

	app->config.logHandle = fopen (logFile, "a");

	if (app->config.logHandle != NULL) {
		result = TRUE;
	}

	return result;
}

//
// FUNCTION : app_log_open_handle
// DESCRIPTION :
// Starts application logging with an existing FILE * handle.
// PARAMETERS :
// struct app * app : application context
// FILE * logHandle : file handle to log to
// 
void app_log_open_handle (struct app * app, FILE * logHandle) {
	app->config.log = TRUE;

	app->config.logHandle = logHandle;
}

//
// FUNCTION : app_log_close
// DESCRIPTION :
// Ends application logging and closes log handle.
// PARAMETERS :
// struct app * app : application context
// 
void app_log_close (struct app * app) {
	if ( app->config.logHandle != NULL) {
		fclose(app->config.logHandle);
	}
}

//
// FUNCTION : app_logv
// DESCRIPTION :
// This function formats and writes messages to the log file
// PARAMETERS :
// struct app * app : application context
// const char * format : printf formated format string
// va_list args : variadic arguments for formating
// 
void app_logv (struct app * app, const char * format, va_list args) {
	/// @todo break up newlines into properly formatted log lines
	time_t rawTime;
	struct tm * timeInfo;
	char timeBuffer [TIME_MAX_LEN] = {0};

	if (app->config.log == TRUE && app->config.logHandle != NULL) {
		time(&rawTime);
		timeInfo = localtime(&rawTime);

		strftime(timeBuffer, TIME_MAX_LEN, "%c", timeInfo);

		fprintf(app->config.logHandle, "[%s] ", timeBuffer);
		
		vfprintf(app->config.logHandle, format, args);
		
		fflush(app->config.logHandle);
	}

}

//
// FUNCTION : app_logf
// DESCRIPTION :
// This function writes to the log file in the same format as printf
// PARAMETERS :
// struct app * app : application context
// const char * format : printf formated format string
// ... : formatting parameters
// 
void app_logf (struct app * app, const char * format, ...) {
	va_list args;
	va_start(args, format);
	app_logv(app, format, args);
	va_end(args);
}

//
// FUNCTION : app_printf
// DESCRIPTION :
// This function writes to the screen and optionally the log file in the same format as printf.
// The log file will be written to if application logging has been opened in the application context.
// PARAMETERS :
// struct app * app : application context
// const char * format : printf formated format string
// ... : formatting parameters
// 
void app_printf (struct app * app, const char * format, ...) {
	va_list printArgs;
	va_list logArgs;

	va_start(printArgs, format);
	va_copy(logArgs, printArgs);


	vprintf(format, printArgs);
	va_end (printArgs);

	if ( app->config.log == TRUE ) {
		app_logv(app, format, logArgs);
	}
	va_end(logArgs);
}

//
// FUNCTION : print_address_book_entry
// DESCRIPTION :
// This function prints an address book entry according to the requirements
// PARAMETERS :
// struct app * app : app context
// struct address_book_entry : address book entry to print
// 
void print_address_book_entry (struct app * app, struct address_book_entry * entry) {
	/// @todo fix issue with printing invalid indexes of fields which may be empty
	const char format_string [] = 
		"%s" EOL // Name
		"%s" EOL // Street Address
		"%s, %s, %.*s %.*s" EOL // City, Province, Postal Code
		"%.*s-%.*s-%.*s" EOL // Phone Number
		;

	app_printf(app,
		format_string,
		entry->name,
		entry->streetAddress,
		entry->city,
		entry->province,
		POSTAL_CODE_SEGMENT,
		entry->postalCode,
		POSTAL_CODE_SEGMENT,
		&entry->postalCode[POSTAL_CODE_SEGMENT],
		PHONE_NUMBER_SEGMENT_1,
		entry->phoneNumber,
		PHONE_NUMBER_SEGMENT_2,
		&entry->phoneNumber[PHONE_NUMBER_SEGMENT_1],
		PHONE_NUMBER_SEGMENT_3,
		&entry->phoneNumber[PHONE_NUMBER_SEGMENT_1 + PHONE_NUMBER_SEGMENT_2]
	);
}

//
// FUNCTION : validate_address_book_name
// DESCRIPTION :
// This function validates names for entry into the address book.
// PARAMETERS :
// struct field_input * field : field data
// RETURNS :
// int : TRUE if valid
// 	FALSE if invalid
// 
int validate_address_book_name (struct field_input * field) {
	int valid = FALSE;
	std::cmatch cm;

	if (std::regex_match(field->buffer, cm, std::regex("(?!.*([A-Za-z-'.])\\1)[A-Za-z-'. ]{0,30}$"))) {

		snprintf(field->buffer, field->bufferSize, "%s", cm[0].str().c_str());

		valid = TRUE;
	}

	return valid;
}


//
// FUNCTION : validate_address_street_address
// DESCRIPTION :
// This function validates a street address for entry into the address book.
// PARAMETERS :
// struct field_input * field : field data
// RETURNS :
// int : TRUE if valid
// 	FALSE if invalid
// 
int validate_address_street_address (struct field_input * field) {
	int valid = FALSE;
	std::cmatch cm;

	if (std::regex_match(field->buffer, cm, std::regex("^[a-zA-Z0-9/-'. ,]{0,60}$"))) {

		snprintf(field->buffer, field->bufferSize, "%s", cm[0].str().c_str());

		valid = TRUE;
	}

	return valid;
}

//
// FUNCTION : validate_address_city
// DESCRIPTION :
// This function validates a city name for entry into the address book.
// PARAMETERS :
// struct field_input * field : field data
// RETURNS :
// int : TRUE if valid
// 	FALSE if invalid
// 
int validate_address_city (struct field_input * field) {
	int valid = FALSE;
	std::cmatch cm;

	if (std::regex_match(field->buffer, cm, std::regex("(?!.*([A-Za-a-'])\\1)[A-Za-z-'. ]{0,60}$"))) {

		snprintf(field->buffer, field->bufferSize, "%s", cm[0].str().c_str());

		valid = TRUE;
	}

	return valid;
}

//
// FUNCTION : validate_address_province
// DESCRIPTION :
// This function validates a province code for entry into the address book.
// PARAMETERS :
// struct field_input * field : field data
// RETURNS :
// int : TRUE if valid
// 	FALSE if invalid
// 
int validate_address_province (struct field_input * field) {
	/// @todo implement logic for name validation
	int valid = FALSE;
	int i = 0;
	std::cmatch cm;

	if (std::regex_match(field->buffer, cm, std::regex("^[a-zA-Z]{2}$"))) {
		strncpy(field->buffer, cm[0].str().c_str(), field->bufferSize);

		string_to_upper(field->buffer);
		for (i = 0; i < NUM_VALID_PROVINCE_CODES; i++) {
			if (strncmp(field->buffer, VALID_PROVINCE_CODES[i], field->bufferSize) == 0) {
				valid = TRUE;
				break;
			}
		}
	}


	return valid;
}

//
// FUNCTION : validate_address_postal_code
// DESCRIPTION :
// This function validates a postal code for entry into the address book.
// PARAMETERS :
// struct field_input * field : field data
// RETURNS :
// int : TRUE if valid
// 	FALSE if invalid
// 
int validate_address_postal_code (struct field_input * field) {
	int valid = FALSE;
	std::cmatch cm;

	if (std::regex_match(field->buffer, cm, std::regex("^([a-zA-Z]\\d[a-zA-Z])[ ]?(\\d[a-zA-Z]\\d)$"))) {

		snprintf(field->buffer, field->bufferSize, "%s%s", cm[1].str().c_str(), cm[2].str().c_str());

		string_to_upper(field->buffer);

		valid = TRUE;
	}

	return valid;
}


//
// FUNCTION : validate_address_phone_number
// DESCRIPTION :
// This function validates a phone number for entry into the address book.
// PARAMETERS :
// struct field_input * field : field data
// RETURNS :
// int : TRUE if valid
// 	FALSE if invalid
// 
int validate_address_phone_number (struct field_input * field) {
	int valid = FALSE;
	std::cmatch cm;

	int areaCode = 0;
	int prefix = 0;
	int lineNumber = 0;

	/// @todo fix regex bug with phone number not being validated if it contains a dash between codes
	if (std::regex_match(field->buffer, cm, std::regex("^(\\d\\d\\d)[- ]?(\\d\\d\\d)[- ]?(\\d\\d\\d\\d)$"))) {
		/*
		printf("matches:\r\n");

		for (std::cmatch::iterator it = cm.begin(); it!=cm.end(); ++it) {
			printf("%s\r\n", it->str().c_str());
		}
		*/

		areaCode = atoi(cm[1].str().c_str());
		prefix = atoi(cm[2].str().c_str());
		lineNumber = atoi(cm[3].str().c_str());

		if (!(prefix == 555 && lineNumber >= 100 && lineNumber <= 199)) {
			snprintf(field->buffer, field->bufferSize, "%s%s%s", cm[1].str().c_str(), cm[2].str().c_str(), cm[3].str().c_str());
		
		}

		/// @todo finish phone number validation for invalidating system specific codes


		valid = TRUE;
	}

	return valid;
}

//
// FUNCTION : collect_wait_for_key
// DESCRIPTION :
// This function pauses the terminal/command prompt until a key is pressed.
// 
void collect_wait_for_key () {
	char buffer[BUFFER_WAIT_FOR_KEY];
	printf(":");
	fgets(buffer, BUFFER_WAIT_FOR_KEY, stdin);
}

//
// FUNCTION : collect_field_input
// DESCRIPTION :
// This function collects address book field information.
// PARAMETERS :
// struct app * app : app context
// struct field_input * field : field data including a function pointer for validation
// RETURNS :
// int : TRUE if field data is valid
// 	FALSE if stdin cannot be read or if field is is skipped
// 
int collect_field_input (struct app * app, struct field_input * field) {
	int skip = FALSE;
	
	for (;;) {
		app_printf(app, "%s", MESSAGE_ADDRESS_BOOK_FIELD_PROMPT[field->field]);

		if ( fgets(field->buffer, field->bufferSize, stdin) == NULL ) {
			// Error reading input
			// Undefined behaviour
			app_printf(app, "%s%s%s", MESSAGE_ERROR_PREFIX, MESSAGE_ADDRESS_BOOK_FIELD_ERROR_UNABLE_TO_READ_INPUT, MESSAGE_ERROR_SUFFIX);
			snprintf(field->buffer, field->bufferSize, "");
			break;
		}
		else if ( strcmp(field->buffer, EOL) == 0 ) {
			app_printf(app, "%s" EOL, MESSAGE_ADDRESS_BOOK_FIELD_SKIP);
			// Clear the field buffer from ugly input
			snprintf(field->buffer, field->bufferSize, "");
			skip = TRUE;
			break;
		}
		
		// this is might be actual input.
		// let's validate it.

		// strip leading whitespace
		string_strip_leading_whitespace(field->buffer);
		
		// strip newline
		string_terminate_at_first_newline(field->buffer);
		
		if ( field->validator(field) == TRUE ) {
			// Name is valid
			break;
		}
		else {
			app_printf(app, "%s" EOL, MESSAGE_ADDRESS_BOOK_FIELD_ERROR_INVALID[field->field]);
		}
	}
	
	return skip;
}

//
// FUNCTION : collect_address_book_entry
// DESCRIPTION :
// This function collects all fields necessary for an address book entry
// PARAMETERS :
// struct app * app : app context
// struct address_book_entry * currentEntry : a pointer to an address book entry we can collect data into
// RETURNS :
// int : TRUE address book entry is filled
// 	FALSE address book entry is skipped
// 
int collect_address_book_entry (struct app * app, struct address_book_entry * currentEntry) {
	int skip = FALSE;
	struct field_input field;
	field_input_init(&field);

	field.field = FIELD_NAME;
	field.buffer = currentEntry->name;
	field.bufferSize = NAME_MAX_LENGTH;
	field.validator = &validate_address_book_name;
	skip = collect_field_input(app, &field);

	if (skip == FALSE) {
		field.field = FIELD_STREET_ADDRESS;
		field.buffer = currentEntry->streetAddress;
		field.bufferSize = STREET_ADDRESS_MAX_LENGTH;
		field.validator = &validate_address_street_address;
		collect_field_input(app, &field);
		
		field.field = FIELD_CITY;
		field.buffer = currentEntry->city;
		field.bufferSize = CITY_MAX_LENGTH;
		field.validator = &validate_address_city;
		collect_field_input(app, &field);

		field.field = FIELD_PROVINCE;
		field.buffer = currentEntry->province;
		field.bufferSize = PROVINCE_MAX_LENGTH;
		field.validator = &validate_address_province;
		collect_field_input(app, &field);

		field.field = FIELD_POSTAL_CODE;
		field.buffer = currentEntry->postalCode;
		field.bufferSize = POSTAL_CODE_MAX_LENGTH;
		field.validator = &validate_address_postal_code;
		collect_field_input(app, &field);

		field.field = FIELD_PHONE_NUMBER;
		field.buffer = currentEntry->phoneNumber;
		field.bufferSize = PHONE_NUMBER_MAX_LENGTH;
		field.validator = &validate_address_phone_number;
		collect_field_input(app, &field);
		app->addressBookSize++;
	}
	return skip;
}

//
// FUNCTION : collect_address_book
// DESCRIPTION :
// This function collects address book entries up to the maximum allowd in the address book (see ADDRESS_BOOK_MAX_LENGTH).
// PARAMETERS :
// struct app * app : app context
// 
void collect_address_book (struct app * app) {
	int i = 0;
	
	app_printf(app, "Address Book Collection Service\r\n");
	app_printf(app, "===============================\r\n");

	for (i = 0; i < ADDRESS_BOOK_MAX_LENGTH; i++) {
		app_printf(app, "Entry %d:\r\n", i + 1);
		if ( collect_address_book_entry(app, &app->addressBook[i]) == TRUE ) {
			app_printf(app, "%s" EOL, MESSAGE_ADDRESS_BOOK_CONTACT_SKIP);
			break;
		}
		else {
		}
	}
}

//
// FUNCTION : print_address_book
// DESCRIPTION :
// This function prints all collected address book entries.
// PARAMETERS :
// struct app * app : app context
// 
void print_address_book (struct app * app) {
	int i = 0;
	
	app_printf(app, "Address Book Distribution Service\r\n");
	app_printf(app, "=================================\r\n");

	for (i = 0; i < app->addressBookSize; i++) {
		if (i % app->config.paginationNumEntries == 0 && i > 0) {
			collect_wait_for_key();
		}

		print_address_book_entry(app, &app->addressBook[i]);
		printf("++++++++++++" EOL);
	}
}


#ifndef UNIT_TESTING
int main (void) {
	struct app app;
	app_init(&app);

	//app_log_open(&app, "addressbook.log");
	app_log_open_handle(&app, stderr);
	
	/*
	for (int i = 0; i < ADDRESS_BOOK_MAX_LENGTH; i++) {
		snprintf(app.addressBook[i].name, NAME_MAX_LENGTH, "Norbert Mika");
		snprintf(app.addressBook[i].streetAddress, STREET_ADDRESS_MAX_LENGTH, "299 Doon Valley Drive");
		snprintf(app.addressBook[i].city, CITY_MAX_LENGTH, "Kitchener");
		snprintf(app.addressBook[i].province, PROVINCE_MAX_LENGTH, "ON");
		snprintf(app.addressBook[i].postalCode, POSTAL_CODE_MAX_LENGTH, "N2G4M4");
		snprintf(app.addressBook[i].phoneNumber, PHONE_NUMBER_MAX_LENGTH, "5197485220");
		app.addressBookSize++;
	}
	*/

	collect_address_book(&app);
	print_address_book(&app);

	app_log_close(&app);

	return EXIT_SUCCESS;
}
#else

// Unit testing

#define UNIT_TEST_NAME_MAX_LEN 60
#define UNIT_TEST_DESCRIPTION_MAX_LEN 100

struct unit_test {
	char name [UNIT_TEST_NAME_MAX_LEN];
	char description [UNIT_TEST_DESCRIPTION_MAX_LEN];
	int expectedResult;
	int (*test)();
};

int test_validate_address_book_name_assert_valid () {
	int result = FALSE;
	char fieldBuffer [NAME_MAX_LENGTH];
	struct field_input field;
	field_input_init(&field);

	field.field = FIELD_NAME;
	field.buffer = fieldBuffer;
	snprintf(field.buffer, NAME_MAX_LENGTH, "Verdi R-D");
	field.bufferSize = NAME_MAX_LENGTH;
	field.validator = &validate_address_book_name;
	
	result = field.validator(&field);
	
	return result;
}

int test_validate_address_book_name_assert_invalid () {
	int result = FALSE;
	char fieldBuffer [NAME_MAX_LENGTH];
	struct field_input field;
	field_input_init(&field);

	field.field = FIELD_NAME;
	field.buffer = fieldBuffer;
	snprintf(field.buffer, NAME_MAX_LENGTH, "Verdi R--D");
	field.bufferSize = NAME_MAX_LENGTH;
	field.validator = &validate_address_book_name;
	
	result = field.validator(&field);
	
	return result;
}

int test_validate_address_street_address_assert_valid () {
	int result = FALSE;
	char fieldBuffer [STREET_ADDRESS_MAX_LENGTH];
	struct field_input field;
	field_input_init(&field);

	field.field = FIELD_STREET_ADDRESS;
	field.buffer = fieldBuffer;
	snprintf(field.buffer, STREET_ADDRESS_MAX_LENGTH, "299 Doon Valley Drive");
	field.bufferSize = STREET_ADDRESS_MAX_LENGTH;
	field.validator = &validate_address_street_address;
	
	result = field.validator(&field);
	
	return result;
}

int test_validate_address_street_address_assert_invalid () {
	int result = FALSE;
	char fieldBuffer [STREET_ADDRESS_MAX_LENGTH];
	struct field_input field;
	field_input_init(&field);

	field.field = FIELD_STREET_ADDRESS;
	field.buffer = fieldBuffer;
	snprintf(field.buffer, STREET_ADDRESS_MAX_LENGTH, "Corner across from the //");
	field.bufferSize = STREET_ADDRESS_MAX_LENGTH;
	field.validator = &validate_address_street_address;
	
	result = field.validator(&field);
	
	return result;
}

int test_validate_address_city_assert_valid () {
	int result = FALSE;
	char fieldBuffer [CITY_MAX_LENGTH];
	struct field_input field;
	field_input_init(&field);

	field.field = FIELD_CITY;
	field.buffer = fieldBuffer;
	snprintf(field.buffer, CITY_MAX_LENGTH, "Guelph");
	field.bufferSize = CITY_MAX_LENGTH;
	field.validator = &validate_address_city;
	
	result = field.validator(&field);
	
	return result;
}

int test_validate_address_city_assert_invalid () {
	int result = FALSE;
	char fieldBuffer [CITY_MAX_LENGTH];
	struct field_input field;
	field_input_init(&field);

	field.field = FIELD_CITY;
	field.buffer = fieldBuffer;
	snprintf(field.buffer, CITY_MAX_LENGTH, "Jefferson #3");
	field.bufferSize = CITY_MAX_LENGTH;
	field.validator = &validate_address_city;
	
	result = field.validator(&field);
	
	return result;
}

int test_validate_address_province_assert_valid () {
	int result = FALSE;
	char fieldBuffer [PROVINCE_MAX_LENGTH];
	struct field_input field;
	field_input_init(&field);

	field.field = FIELD_CITY;
	field.buffer = fieldBuffer;
	snprintf(field.buffer, PROVINCE_MAX_LENGTH, "ON");
	field.bufferSize = PROVINCE_MAX_LENGTH;
	field.validator = &validate_address_province;
	
	result = field.validator(&field);
	
	return result;
}

int test_validate_address_province_assert_invalid () {
	int result = FALSE;
	char fieldBuffer [PROVINCE_MAX_LENGTH];
	struct field_input field;
	field_input_init(&field);

	field.field = FIELD_CITY;
	field.buffer = fieldBuffer;
	snprintf(field.buffer, PROVINCE_MAX_LENGTH, "CA");
	field.bufferSize = PROVINCE_MAX_LENGTH;
	field.validator = &validate_address_province;
	
	result = field.validator(&field);
	
	return result;
}

int test_validate_address_postal_code_assert_valid () {
	int result = FALSE;
	char fieldBuffer [POSTAL_CODE_MAX_LENGTH];
	struct field_input field;
	field_input_init(&field);

	field.field = FIELD_CITY;
	field.buffer = fieldBuffer;
	snprintf(field.buffer, POSTAL_CODE_MAX_LENGTH, "N2G 4M4");
	field.bufferSize = POSTAL_CODE_MAX_LENGTH;
	field.validator = &validate_address_postal_code;
	
	result = field.validator(&field);
	
	return result;
}

int test_validate_address_postal_code_assert_invalid () {
	int result = FALSE;
	char fieldBuffer [POSTAL_CODE_MAX_LENGTH];
	struct field_input field;
	field_input_init(&field);

	field.field = FIELD_CITY;
	field.buffer = fieldBuffer;
	snprintf(field.buffer, POSTAL_CODE_MAX_LENGTH, "H0H0H0H0");
	field.bufferSize = POSTAL_CODE_MAX_LENGTH;
	field.validator = &validate_address_postal_code;
	
	result = field.validator(&field);
	
	return result;
}

int test_validate_address_phone_number_assert_valid () {
	int result = FALSE;
	char fieldBuffer [PHONE_NUMBER_MAX_LENGTH];
	struct field_input field;
	field_input_init(&field);

	field.field = FIELD_CITY;
	field.buffer = fieldBuffer;
	snprintf(field.buffer, PHONE_NUMBER_MAX_LENGTH, "5197485220");
	field.bufferSize = PHONE_NUMBER_MAX_LENGTH;
	field.validator = &validate_address_postal_code;
	
	result = field.validator(&field);
	
	return result;
}

int test_validate_address_phone_number_assert_invalid () {
	int result = FALSE;
	char fieldBuffer [PHONE_NUMBER_MAX_LENGTH];
	struct field_input field;
	field_input_init(&field);

	field.field = FIELD_CITY;
	field.buffer = fieldBuffer;
	snprintf(field.buffer, PHONE_NUMBER_MAX_LENGTH, "(519) 748-5220");
	field.bufferSize = PHONE_NUMBER_MAX_LENGTH;
	field.validator = &validate_address_phone_number;
	
	result = field.validator(&field);
	
	return result;
}

int run_test (struct unit_test * test) {
	int result = FALSE;
	int passed = FALSE;
	
	result = test->test();

	if (result == test->expectedResult) {
		passed = TRUE;
	}
	else {
		passed = FALSE;
	}

	printf("Running test: %s [%s]" EOL "%s Asserts value: %s Result: %s" EOL EOL, 
			test->name,
			passed == TRUE ? "PASS" : "FAIL",
			test->description,
			test->expectedResult == TRUE ? "TRUE" : "FALSE",
			result == TRUE ? "TRUE" : "FALSE"
	      );

	return passed;

}

void run_tests() {
	struct unit_test tests [] = {
		{
			"Validate address book name",
			"Tests if a valid name is valid.",
			TRUE,
			&test_validate_address_book_name_assert_valid
		},
		{
			"Validate address book name",
			"Tests if an invalid name is valid.",
			FALSE,
			&test_validate_address_book_name_assert_invalid
		},
		{
			"Validate address book street address",
			"Tests if a valid street address is valid.",
			TRUE,
			&test_validate_address_street_address_assert_valid
		},
		{
			"Validate address book street address",
			"Tests if an invalid street address is valid.",
			FALSE,
			&test_validate_address_street_address_assert_invalid
		},
		{
			"Validate address book city",
			"Tests if a valid city name is valid.",
			TRUE,
			&test_validate_address_city_assert_valid
		},
		{
			"Validate address book city",
			"Tests if an invalid city name is valid.",
			FALSE,
			&test_validate_address_city_assert_invalid
		},
		{
			"Validate address book province",
			"Tests if a valid province code is valid.",
			TRUE,
			&test_validate_address_province_assert_valid
		},
		{
			"Validate address book province",
			"Tests if an invalid province code is valid.",
			FALSE,
			&test_validate_address_province_assert_invalid
		},
		{
			"Validate address book postal code",
			"Tests if a valid postal code is valid.",
			TRUE,
			&test_validate_address_postal_code_assert_valid
		},
		{
			"Validate address book postal code",
			"Tests if an invalid postal code is valid.",
			FALSE,
			&test_validate_address_postal_code_assert_invalid
		},
		{
			"Validate address book phone number",
			"Tests if a valid phone number is valid.",
			TRUE,
			&test_validate_address_phone_number_assert_valid
		},
		{
			"Validate address book phone number",
			"Tests if a valid phone number in the incorrect format is valid.",
			FALSE,
			&test_validate_address_phone_number_assert_invalid
		},
	};

	int num_tests = 12;
	
	
	int i = 0;
	int passedTests = 0;
	float passAverage = 0.0f;

	printf("Running %d tests:" EOL EOL, num_tests);
	for (i = 0; i < num_tests; i++) {
		if (run_test(&tests[i]) == TRUE) {
			passedTests++;
		}
	}

	passAverage = ((passedTests * 1.0f) / (num_tests * 1.0f)) * 100.0f;

	printf("Test Summary: Passed %d/%d tests. Average %.1f%%" EOL, passedTests, num_tests, passAverage);


}




int main (void) {
	run_tests();

	return EXIT_SUCCESS;
}

#endif
