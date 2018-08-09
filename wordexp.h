#ifndef _WORDEXP_H_
#define _WORDEXP_H_

#define WRDE_NOCMD 0

#define WORDEXP_READY 0

typedef struct {
	size_t we_wordc;
	char **we_wordv;
	size_t we_offs;
} wordexp_t;


enum
  {



    WRDE_NOSPACE = 1,
    WRDE_BADCHAR,
    WRDE_BADVAL,
    WRDE_CMDSUB,
    WRDE_SYNTAX
  };


static inline int wordexp(const char *c, wordexp_t *w, int _i)
{
	return -1;
}

static inline void wordfree(wordexp_t *__wordexp)
{
}

#endif
