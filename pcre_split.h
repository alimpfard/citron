#ifndef __GUARD_PCRE_SPLIT_H
#define __GUARD_PCRE_SPLIT_H

/*
 */

#include <stdio.h>
#include <string.h>
#include <pcre.h>
#include "citron.h"

ctr_object *pcre_split(char *, char *);
ssize_t pcre_indexof_internal(pcre* re, char* string);
ssize_t pcre_last_indexof_internal(pcre* re, char* string);
ssize_t pcre_last_indexof(char* pattern, char* string);
ssize_t pcre_indexof(char* pattern, char* string);
#define OVECCOUNT 30

typedef struct split_t_internal {
	char *string;
	char *match;
	unsigned int length;
	unsigned int offset;
} split_t_internal;

split_t_internal *pcre_split_internal(pcre *, char *, int);

ssize_t pcre_indexof(char* pattern, char* string) {
	pcre *re;
	const char* error;
	int erroffset;
	re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
	if (re == NULL) {
		char serror[1024];
		size_t len = sprintf(serror, "Error: PCRE compilation failed at offset %d: %s\n", erroffset, error);
		CtrStdFlow = ctr_build_string(serror, len);
		return 0;
	}
	ssize_t offset = pcre_indexof_internal(re, string);
	pcre_free(re);
	return offset;
}

ssize_t pcre_last_indexof(char* pattern, char* string) {
	pcre *re;
	const char* error;
	int erroffset;
	re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
	if (re == NULL) {
		char serror[1024];
		size_t len = sprintf(serror, "Error: PCRE compilation failed at offset %d: %s\n", erroffset, error);
		CtrStdFlow = ctr_build_string(serror, len);
		return 0;
	}
	ssize_t offset = pcre_indexof_internal(re, string);
	size_t position = offset;
	while(1) {
		offset = pcre_indexof_internal(re, string+position);
		if(offset != -1)
			position = position + offset;
		else break;
	}
	pcre_free(re);
	return position;
}

/* Initialise the RegEx */
ctr_object *pcre_split(char *pattern, char *string) {
	pcre *re;
	const char *error;
	int erroffset;
	split_t_internal *si;
	ctr_argument* push_arg = ctr_heap_allocate(sizeof(ctr_argument));
	ctr_object* sarr = ctr_array_new(CtrStdArray, NULL);

	/*
	  Make a local copy of pattern and string
	*/
	char *s = string;

	/*
	  Setup RegEx
	*/
	re = pcre_compile(pattern, 0, &error, &erroffset, NULL);

	/*
	  Check if compilation was successful
	*/
	if(re == NULL) {
    char serror[1079];
    size_t len = sprintf(serror, "Error: PCRE compilation failed at offset %d: %s\n", erroffset, error);
		CtrStdFlow = ctr_build_string(serror, len);
		return NULL;
	}

	/*
	  Loop thorough string
	*/
	char *match;
	int flag = 0; // match flags
	do {
		/* Return first occurence of match of RegEx */
		si = pcre_split_internal(re, s, flag);

		/* Copy pointer of match */
		match = si->match;

		/* Push into array */
		if(si->offset) {
			push_arg->object = ctr_build_string(s, si->offset);
			ctr_array_push(sarr, push_arg);
		}
		/* Increment string pointer to skip previous match */
		s += si->offset + si->length;

		if (!si->length)
			// we already got an empty match, skip past it
			flag |= PCRE_NOTEMPTY_ATSTART;
		else
			flag &= ~PCRE_NOTEMPTY_ATSTART; // 🤷

		ctr_heap_free(si);

	} while(match && *s!=0);

	/*
	  Free locally allocated memory
	 */
	if(*s != 0) {
		push_arg->object = ctr_build_string_from_cstring(s);
		ctr_array_push(sarr, push_arg);
	}
	pcre_free(re);
	ctr_heap_free(push_arg);
	return sarr;
}

split_t_internal *pcre_split_internal(pcre *re, char *string, int flags)
{

	int rc;
	int ovector[OVECCOUNT];
	int length;
	split_t_internal *s = (split_t_internal *)ctr_heap_allocate(sizeof(split_t_internal));
	s->offset = 0;
	length = (int)strlen(string);

	rc = pcre_exec(re, NULL, string, length, 0, flags, ovector, OVECCOUNT);

	/* check for matches */
	if(rc < 0) {
		switch(rc) {
			case PCRE_ERROR_NOMATCH:
				s->string = (char *)ctr_heap_allocate(sizeof(char)*(length + 1));
				strncpy(s->string, string, length);
				s->match = NULL;
				return s;
				break;
			default: {
				char err[1024];
				size_t len = sprintf(err, "Error: Matching error: %d\n", rc);
				CtrStdFlow = ctr_build_string(err, len);
				return (split_t_internal *)NULL;
				break;
			}
		}
	}

	/* check if output vector was large enough */
	if(rc == 0) {
		rc = OVECCOUNT/3;
		fprintf(stderr, "Warning: ovector only has room for %d captured substrings\n", rc-1);
	}

	s->string = ctr_heap_allocate(sizeof(char) * (ovector[0] + 1));
	strncpy(s->string, string, ovector[0]);

	s->match = ctr_heap_allocate(sizeof(char) * ((ovector[1] - ovector[0]) + 1));
	strncpy(s->match, (char *)(string + ovector[0]), (ovector[1] - ovector[0]));

	s->length = ovector[1] - ovector[0];
	s->offset = ovector[0];
	return (split_t_internal *)s;
}

ssize_t pcre_indexof_internal(pcre* re, char* string) {
		int rc;
		int ovector[OVECCOUNT];
		int length;

		length = (int)strlen(string);

		rc = pcre_exec(re, NULL, string, length, 0, 0, ovector, OVECCOUNT);

		/* check for matches */
		if(rc < 0) {
			return -1;
		}

		/* check if output vector was large enough */
		if(rc == 0) {
			rc = OVECCOUNT/3;
			fprintf(stderr, "Warning: ovector only has room for %d captured substrings\n", rc-1);
		}

		return ovector[1];
}
#endif
