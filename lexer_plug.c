#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <regex.h>

#include "citron.h"

static int ctr_lex_bflmt = 255;
static ctr_size ctr_lex_tokvlen = 0;	/* length of the string value of a token */
static char *ctr_lex_buffer;
static char *ctr_code;
static char *ctr_code_eoi;
static char *ctr_eofcode;
static char *ctr_code_st;
static char *ctr_lex_oldptr;
static char *ctr_lex_olderptr;
static int ctr_lex_verbatim_mode = 0;	/* flag: indicates whether lexer operates in verbatim mode or not (1 = ON, 0 = OFF) */
static uintptr_t ctr_lex_verbatim_mode_insert_quote = 0;	/* pointer to 'overlay' the 'fake quote' for verbatim mode */
static int ctr_lex_line_number = 0;
static int ctr_lex_old_line_number = 0;

static char *ctr_lex_desc_tok_ref = "reference";
static char *ctr_lex_desc_tok_quote = "'";
static char *ctr_lex_desc_tok_number = "number";
static char *ctr_lex_desc_tok_paropen = "(";
static char *ctr_lex_desc_tok_parclose = ")";
static char *ctr_lex_desc_tok_blockopen = "{";
static char *ctr_lex_desc_tok_blockopen_map = "{\\";
static char *ctr_lex_desc_tok_blockclose = "}";
static char *ctr_lex_desc_tok_tupopen = "[";
static char *ctr_lex_desc_tok_tupclose = "]";
static char *ctr_lex_desc_tok_colon = ":";
static char *ctr_lex_desc_tok_dot = ".";
static char *ctr_lex_desc_tok_chain = ",";
static char *ctr_lex_desc_tok_booleanyes = "True";
static char *ctr_lex_desc_tok_booleanno = "False";
static char *ctr_lex_desc_tok_nil = "Nil";
static char *ctr_lex_desc_tok_assignment = ":=";	//derp
static char *ctr_lex_desc_tok_passignment = "=>";	//REEEE
static char *ctr_lex_desc_tok_ret = "^";
static char *ctr_lex_desc_tok_ret_unicode = "↑";
static char *ctr_lex_desc_tok_fin = "end of program";
static char *ctr_lex_desc_tok_unknown = "(unknown token)";

static int ctr_string_interpolation = 0;
static char *ivarname;
static int ivarlen;

/**
 * Lexer - is Symbol Delimiter ?
 * Determines whether the specified symbol is a delimiter.
 * Returns 1 if the symbol is a delimiter and 0 otherwise.
 *
 * @param char symbol symbol to be inspected
 *
 * @return uint8_t
 */
static uint8_t ctr_lex_is_delimiter(char symbol)
{

	return (symbol == '('
		|| symbol == ')' || symbol == ',' || symbol == '.'
		|| symbol == ':' || symbol == ' ');
}

unsigned long ctr_lex_position() {
		return ctr_code - ctr_code_st;
}

char* ctr_lex_get_buf() { return ctr_code; }
ctr_object* ctr_lex_get_buf_str() { return ctr_build_string(ctr_code, ctr_eofcode-ctr_code); }

void ctr_lex_skip(int len) {
	// printf("skipping %d\n", len);
	ctr_code = ctr_code + len;
	if(ctr_code > ctr_eofcode) ctr_code = ctr_eofcode;
}
/**
 * CTRLexerEmitError
 *
 * Displays an error message for the lexer.
 */
void ctr_lex_emit_error(char *message)
{
	printf("%s on line: %d. \n", message, ctr_lex_line_number);
#ifdef EXIT_ON_ERROR
	exit(1);
#endif
}

/**
 * CTRLexerLoad
 *
 * Loads program into memory.
 */
void ctr_lex_load(char *prg, ctr_size len)
{
	ctr_code = prg;
  ctr_code_st = ctr_code;
	ctr_lex_buffer = ctr_heap_allocate_tracked(ctr_lex_bflmt);
	ctr_lex_buffer[0] = '\0';
	ctr_eofcode = (ctr_code + len);
	ctr_lex_line_number = 0;
}

/**
 * CTRLexerTokenValue
 *
 * Returns the string of characters representing the value
 * of the currently selected token.
 */
char *ctr_lex_tok_value()
{
	return ctr_lex_buffer;
}

char *ctr_lex_tok_describe(int token)
{
	char *description;
	switch (token) {
	case CTR_TOKEN_RET:
		description = ctr_lex_desc_tok_ret;
		break;
	case CTR_TOKEN_ASSIGNMENT:
		description = ctr_lex_desc_tok_assignment;
		break;
	case CTR_TOKEN_PASSIGNMENT:
		description = ctr_lex_desc_tok_passignment;
		break;
	case CTR_TOKEN_BLOCKCLOSE:
		description = ctr_lex_desc_tok_blockclose;
		break;
	case CTR_TOKEN_BLOCKOPEN:
		description = ctr_lex_desc_tok_blockopen;
		break;
	case CTR_TOKEN_BLOCKOPEN_MAP:
		description = ctr_lex_desc_tok_blockopen_map;
		break;
	case CTR_TOKEN_BOOLEANNO:
		description = ctr_lex_desc_tok_booleanno;
		break;
	case CTR_TOKEN_BOOLEANYES:
		description = ctr_lex_desc_tok_booleanyes;
		break;
	case CTR_TOKEN_CHAIN:
		description = ctr_lex_desc_tok_chain;
		break;
	case CTR_TOKEN_COLON:
		description = ctr_lex_desc_tok_colon;
		break;
	case CTR_TOKEN_DOT:
		description = ctr_lex_desc_tok_dot;
		break;
	case CTR_TOKEN_FIN:
		description = ctr_lex_desc_tok_fin;
		break;
	case CTR_TOKEN_NIL:
		description = ctr_lex_desc_tok_nil;
		break;
	case CTR_TOKEN_NUMBER:
		description = ctr_lex_desc_tok_number;
		break;
	case CTR_TOKEN_PARCLOSE:
		description = ctr_lex_desc_tok_parclose;
		break;
	case CTR_TOKEN_PAROPEN:
		description = ctr_lex_desc_tok_paropen;
		break;
	case CTR_TOKEN_QUOTE:
		description = ctr_lex_desc_tok_quote;
		break;
	case CTR_TOKEN_REF:
		description = ctr_lex_desc_tok_ref;
		break;
	case CTR_TOKEN_TUPOPEN:
		description = ctr_lex_desc_tok_tupopen;
		break;
	case CTR_TOKEN_TUPCLOSE:
		description = ctr_lex_desc_tok_tupclose;
		break;
	default:
		description = ctr_lex_desc_tok_unknown;
	}
	return description;
}

/**
 * CTRLexerTokenValueLength
 *
 * Returns the length of the value of the currently selected token.
 */
long ctr_lex_tok_value_length()
{
	return ctr_lex_tokvlen;
}

/**
 * CTRLexerPutBackToken
 *
 * Puts back a token and resets the pointer to the previous one.
 */
void ctr_lex_putback()
{
	if (ctr_string_interpolation > 0) {
		ctr_string_interpolation--;
		return;
	}
	ctr_code = ctr_lex_oldptr;
	ctr_lex_oldptr = ctr_lex_olderptr;
	ctr_lex_line_number = ctr_lex_old_line_number;
}


int ctr_lex_is_valid_digit_in_base(char c, int b)
{
	if (b <= 10) {
		if ((c >= '0') && (c < ('0' + b)))
			return 1;
	} else if (b <= 36) {
		if ((c >= '0') && (c <= '9'))
			return 1;
		else if ((c >= 'A') && (c < 'A' + (b - 10)))
			return 1;
	}
	return 0;		//unsupported base or not a digit
}

/**
 * CTRLexerReadToken
 *
 * Reads the next token from the program buffer and selects this
 * token.
 */
int ctr_lex_tok()
{
	if (ctr_code == ctr_eofcode) {
		return CTR_TOKEN_FIN;
	}
	char c;
	int i, comment_mode, presetToken, pragma_mode;
	ctr_lex_tokvlen = 0;
	ctr_lex_olderptr = ctr_lex_oldptr;
	ctr_lex_oldptr = ctr_code;
	ctr_lex_old_line_number = ctr_lex_line_number;
	i = 0;
	comment_mode = 0;
	pragma_mode = 0;

	/* a little state machine to handle string interpolation, */
	/* i.e. transforms ' $$x ' into: ' ' + x + ' '. */
	switch (ctr_string_interpolation) {
	case 1:
		presetToken = CTR_TOKEN_QUOTE;
		break;
	case 2:
	case 4:
		memcpy(ctr_lex_buffer, "+", 1);
		ctr_lex_tokvlen = 1;
		presetToken = CTR_TOKEN_REF;
		break;
	case 3:
		memcpy(ctr_lex_buffer, ivarname, ivarlen);
		ctr_lex_tokvlen = ivarlen;
		presetToken = CTR_TOKEN_REF;
		break;
	case 5:
		ctr_code = ctr_code_eoi;
		presetToken = CTR_TOKEN_QUOTE;
		break;
	}
	/* return the preset token, and transition to next state */
	if (ctr_string_interpolation) {
		ctr_string_interpolation++;
		return presetToken;
	}

	/* if verbatim mode is on and we passed the '>' verbatim write message, insert a 'fake quote' (?>') */
	if (ctr_lex_verbatim_mode == 1
	    && ctr_lex_verbatim_mode_insert_quote == (uintptr_t) ctr_code) {
		return CTR_TOKEN_QUOTE;
	}

	if (ctr_code != ctr_eofcode && *ctr_code == '\n'
	    && check_next_line_empty() && oneLineExpressions->value) {
		ctr_code++;
		return CTR_TOKEN_DOT;
	}

	c = *ctr_code;
	while (ctr_code != ctr_eofcode
	       && (isspace(c) || c == '#' || comment_mode)) {
		if (c == '\n') {
			comment_mode = 0;
			pragma_mode = 0;
			ctr_lex_line_number++;
		}
		if (c == '#') {
			comment_mode = 1;
			if (*(ctr_code + 1) == ':')
				pragma_mode = 1;
		}
		if (pragma_mode)
			ctr_match_toggle_pragma();
		ctr_code++;
		c = *ctr_code;
	}
	if (ctr_code == ctr_eofcode) {
		return CTR_TOKEN_FIN;
	}
	if (c == '(') {
		ctr_code++;
		return CTR_TOKEN_PAROPEN;
	}
	if (c == ')') {
		ctr_code++;
		return CTR_TOKEN_PARCLOSE;
	}
	if (c == '[') {
		ctr_code++;
		return CTR_TOKEN_TUPOPEN;
	}
	if (c == ']') {
		ctr_code++;
		return CTR_TOKEN_TUPCLOSE;
	}
	if (c == '{') {
		ctr_code++;
		if (ctr_code != ctr_eofcode && (c = *ctr_code) == '\\') {
			ctr_code++;
			return CTR_TOKEN_BLOCKOPEN_MAP;
		} else
			return CTR_TOKEN_BLOCKOPEN;
	}
	if (c == '}') {
		ctr_code++;
		return CTR_TOKEN_BLOCKCLOSE;
	}
	if (c == '.') {
		ctr_code++;
		return CTR_TOKEN_DOT;
	}
	if (c == ',') {
		ctr_code++;
		return CTR_TOKEN_CHAIN;
	}
	if (((c == 'i') && (ctr_code + 1) < ctr_eofcode
	     && (*(ctr_code + 1) == 's')
	     && isspace(*(ctr_code + 2))) || ((c == ':')
					      && (ctr_code + 1) < ctr_eofcode
					      && (*(ctr_code + 1) == '='))) {
		ctr_code += 2;
		return CTR_TOKEN_ASSIGNMENT;
	}
	if ((c == '=') && (ctr_code + 1) < ctr_eofcode
	    && (*(ctr_code + 1) == '>')) {
		ctr_code += 2;
		return CTR_TOKEN_PASSIGNMENT;
	}
	if (c == ':' /*&& ctr_code+1!=ctr_eofcode && *(ctr_code+1) != ':' */ ) {
		int is_name = 0;
		while(ctr_code+1<ctr_eofcode && *(ctr_code+1) == ':') {
			is_name++;
			ctr_code++;
			ctr_lex_buffer[i++] = ':';
			ctr_lex_tokvlen++;
		}
		ctr_code++;
		if(is_name) {
			if(ctr_lex_tokvlen > 1) ctr_code--;
			return CTR_TOKEN_REF;
		}
		return CTR_TOKEN_COLON;
	}
	if (c == '^') {
		ctr_code++;
		return CTR_TOKEN_RET;
	}
	//↑
	if ((ctr_code + 2) < ctr_eofcode && (uint8_t) c == 226
	    && ((uint8_t) * (ctr_code + 1) == 134)
	    && ((uint8_t) * (ctr_code + 2) == 145)) {
		ctr_code += 3;
		return CTR_TOKEN_RET;
	}
	if (c == '\'') {
		ctr_code++;
		return CTR_TOKEN_QUOTE;
	}
	if ((c == '-' && (ctr_code + 1) < ctr_eofcode
	     && isdigit(*(ctr_code + 1))) || isdigit(c)) {
		int xnum_likely = c == '0';
		int base = 10;
		ctr_lex_buffer[i] = c;
		ctr_lex_tokvlen++;
		i++;
		ctr_code++;
		c = toupper(*ctr_code);
		if (xnum_likely)
			base = c == 'X' ? 16 : 10;	//let the parser handle incorrect values
		if (base != 10) {
			ctr_lex_buffer[i] = c;
			ctr_lex_tokvlen++;
			i++;
			ctr_code++;
			c = toupper(*ctr_code);
		}
		while ((ctr_lex_is_valid_digit_in_base(c, base))) {
			ctr_lex_buffer[i] = c;
			ctr_lex_tokvlen++;
			i++;
			ctr_code++;
			c = toupper(*ctr_code);
		}
		if (c == '.' && (ctr_code + 1 <= ctr_eofcode)
		    &&
		    !ctr_lex_is_valid_digit_in_base(toupper(*(ctr_code + 1)),
						     base)) {
			return CTR_TOKEN_NUMBER;
		}
		if (c == '.') {
			ctr_lex_buffer[i] = c;
			ctr_lex_tokvlen++;
			i++;
			ctr_code++;
			c = toupper(*ctr_code);
		}
		while ((ctr_lex_is_valid_digit_in_base(c, base))) {
			ctr_lex_buffer[i] = c;
			ctr_lex_tokvlen++;
			i++;
			ctr_code++;
			c = toupper(*ctr_code);
		}
		c = *ctr_code;
		return CTR_TOKEN_NUMBER;
	}
	if (strncmp(ctr_code, "True", 4) == 0) {
		if (ctr_lex_is_delimiter(*(ctr_code + 4))) {
			ctr_code += 4;
			return CTR_TOKEN_BOOLEANYES;
		}
	}
	if (strncmp(ctr_code, "False", 5) == 0) {
		if (ctr_lex_is_delimiter(*(ctr_code + 5))) {
			ctr_code += 5;
			return CTR_TOKEN_BOOLEANNO;
		}
	}
	if (strncmp(ctr_code, "Nil", 3) == 0) {
		if (ctr_lex_is_delimiter(*(ctr_code + 3))) {
			ctr_code += 3;
			return CTR_TOKEN_NIL;
		}
	}

	/* if we encounter a '?>' sequence, switch to verbatim mode in lexer */
	if (strncmp(ctr_code, "?>", 2) == 0) {
		ctr_lex_verbatim_mode = 1;
		ctr_code += 2;
		// memcpy (ctr_lex_buffer, "?", 1);
		// ctr_lex_tokvlen = 1;
		return CTR_TOKEN_QUOTE;
	}

	/* if lexer is in verbatim mode and we pass the '>' symbol insert a fake quote as next token */
	if (strncmp(ctr_code, ">", 1) == 0 && ctr_lex_verbatim_mode == 1) {
		// ctr_lex_verbatim_mode_insert_quote = (uintptr_t) (ctr_code + 1);      /* this way because multiple invocations should return same result */
		// ctr_code++;
		return CTR_TOKEN_QUOTE;
		// memcpy (ctr_lex_buffer, ">", 1);
		// ctr_lex_tokvlen = 1;
		// return CTR_TOKEN_REF;
	}
	// if (*ctr_code == ':') {
	//   int i = 1;
	//   ctr_code++;
	//   while(ctr_code+1!=ctr_eofcode && *(ctr_code++)==':') i++;
	//   if(i>ctr_lex_bflmt) ctr_lex_emit_error( "Token buffer exhausted. Tokens may not exceed 255 bytes" );
	//   ctr_lex_tokvlen = i>2 ? i-1 : i; //leave one ':' for the KWM if more than two chars
	//   for(int v=0; v<ctr_lex_tokvlen; v++)
	//     ctr_lex_buffer[v] = ':';
	//   if(i > 2)
	//     ctr_code -= 2;
	//   // else
	//     // ctr_code;
	//   return CTR_TOKEN_REF;
	// }
	while (!isspace(c) && (c != '#' &&
			       c != '(' &&
			       c != ')' &&
			       c != '[' &&
			       c != ']' &&
			       c != '{' &&
			       c != '}' &&
			       c != '.' &&
			       c != ',' &&
			       c != '^' &&
			       (!((ctr_code + 2) < ctr_eofcode
				  && (uint8_t) c == 226
				  && ((uint8_t) * (ctr_code + 1) == 134)
				  && ((uint8_t) * (ctr_code + 2) == 145)))
			       && (c != ':') && c != '\'')
	       && ctr_code != ctr_eofcode) {
		ctr_lex_buffer[i] = c;
		ctr_lex_tokvlen++;
		i++;
		if (i > ctr_lex_bflmt) {
			ctr_lex_emit_error
			    ("Token Buffer Exausted. Tokens may not exceed 255 bytes");
		}
		ctr_code++;
		if (ctr_code == ctr_eofcode)
			return CTR_TOKEN_REF;
		c = *ctr_code;
	}
	return CTR_TOKEN_REF;
}

/**
 * CTRLexerStringReader
 *
 * Reads an entire string between a pair of quotes.
 */
char *ctr_lex_readstr()
{
	char *strbuff;
	char c;
	long memblock = 1;
	int escape;
	char *beginbuff;
	long page = 100;	/* 100 byte pages */
	size_t tracking_id;

	if (ctr_string_interpolation == 6) {
		ctr_heap_free(ivarname);
		ctr_string_interpolation = 0;
	}

	ctr_lex_tokvlen = 0;
	strbuff = (char *)ctr_heap_allocate_tracked(memblock);
	tracking_id = ctr_heap_get_latest_tracking_id();
	c = *ctr_code;
	escape = 0;
	beginbuff = strbuff;
	while ((		/* reading string in non-verbatim mode, read until the first non-escaped quote */
		       ctr_lex_verbatim_mode == 0 && (c != '\'' || escape == 1)) || (	/* reading string in verbatim mode, read until the '<?' sequence */
											     ctr_lex_verbatim_mode
											     ==
											     1
											     &&
											     !
											     (c
											      ==
											      '<'
											      &&
											      ((ctr_code + 1) < ctr_eofcode)
											      &&
											      *
											      (ctr_code
											       +
											       1)
											      ==
											      '?')
											     &&
											     (ctr_code
											      <
											      ctr_eofcode)))
	{

		/* enter interpolation mode ( $$x ) */
		if (!ctr_lex_verbatim_mode &&
		    !escape && c == '$' && ((ctr_code + 1) < ctr_eofcode)
		    && *(ctr_code + 1) == '$') {
			int q = 2;
			while ((ctr_code + q) < ctr_eofcode
			       && !isspace(*(ctr_code + q))
			       && *(ctr_code + q) != '$'
			       && *(ctr_code + q) != '\'' && q < 255)
				q++;
			if (isspace(*(ctr_code + q)) || *(ctr_code + q) == '$'
			    || *(ctr_code + q) == '\'') {
				ivarname = ctr_heap_allocate(q);
				ivarlen = q - 2;
				memcpy(ivarname, ctr_code + 2, q - 2);
				ctr_string_interpolation = 1;
				ctr_code_eoi = ctr_code + q + 0;	/* '$','$' and the name  ( name + 3 ) */
				break;
			}
		}

		if (c == '\n')
			ctr_lex_line_number++;

		if (escape == 1) {
			switch (c) {
			case 'n':
				c = '\n';
				break;
			case 'r':
				c = '\r';
				break;
			case 't':
				c = '\t';
				break;
			case 'v':
				c = '\v';
				break;
			case 'b':
				c = '\b';
				break;
			case 'a':
				c = '\a';
				break;
			case 'f':
				c = '\f';
				break;
			case 'x':
				c = 0;
				while (ctr_lex_is_valid_digit_in_base
				       (toupper(*(ctr_code + 1)), 16)) {
					char t = *(ctr_code + 1);
					c = c * 16 + (t >= '0'
						      && t <=
						      '9' ? t - '0' : 10 +
						      toupper(t) - 'A');
					ctr_code++;
				}
				break;
			}
		}

		if (c == '\\' && escape == 0 && ctr_lex_verbatim_mode == 0) {
			escape = 1;
			ctr_code++;
			c = *ctr_code;
			continue;
		}
		ctr_lex_tokvlen++;
		if (ctr_lex_tokvlen >= memblock) {
			memblock += page;
			beginbuff =
			    (char *)ctr_heap_reallocate_tracked(tracking_id,
								memblock);
			if (beginbuff == NULL) {
				ctr_lex_emit_error("Out of memory");
			}
			/* reset pointer, memory location might have been changed */
			strbuff = beginbuff + (ctr_lex_tokvlen - 1);
		}
		escape = 0;
		*(strbuff) = c;
		strbuff++;
		ctr_code++;
		if (ctr_code < ctr_eofcode)
			c = *ctr_code;
		else {
			// ctr_lex_emit_error("Expected closing quote");
			c = '\'';
		}
	}
	if (ctr_lex_verbatim_mode) {
		if (ctr_code >= ctr_eofcode) {	/* if we reached EOF in verbatim mode, append closing sequence '<?.' */
			strncpy(ctr_code, "<?.", 3);
			ctr_eofcode += 3;
		}
		ctr_code++;	/* in verbatim mode, hop over the trailing ? as well */
	}
	ctr_lex_verbatim_mode = 0;	/* always turn verbatim mode off */
	ctr_lex_verbatim_mode_insert_quote = 0;	/* erase verbatim mode pointer overlay for fake quote */
	return beginbuff;
}
