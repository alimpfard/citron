#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char* ctr_internal_readf(char* file_name, u_int64_t* total_size);
char* escapeables(char* str, u_int64_t realsize);
char* transform(char* prg, u_int64_t size);

char* ctr_internal_readf(char* file_name, u_int64_t* total_size)
{
    char* prg;
    char ch;
    int prev;
    u_int64_t size;
    u_int64_t real_size;
    FILE* fp;
    fp = fopen(file_name, "r");
    if (fp == NULL) {
        printf("Error while opening the file (%s).\n", file_name);
        exit(1);
    }
    prev = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, prev, SEEK_SET);
    real_size = (size + 4) * sizeof(char);
    prg = malloc(real_size); /* add 4 bytes, 3 for optional closing sequence
                                verbatim mode and one lucky byte! */
    int ctr_program_length = 0;
    while ((ch = fgetc(fp)) != EOF)
        prg[ctr_program_length++] = ch;
    fclose(fp);
    *total_size = (u_int64_t)real_size;
    return prg;
}

char* escapeables(char* str, u_int64_t realsize)
{
    char c;
    u_int64_t size = strlen(str);
    u_int64_t i = 0;
    while (c = *(str + i) != '\0') {
        if (c == '\'') {
            memmove(str + i + 1, str + i, realsize - i + 1);
            *(str + i) = '\\';
            *(str + i + 1) = '\'';
        }
    }
    return str;
}

char* transform(char* prg, u_int64_t size)
{
    char* transformed = malloc(size * 4 * sizeof(char)); // HUGE size.
    char* buffer = malloc(size * sizeof(char));
    u_int64_t buflen = 0;
    u_int64_t tlen = 0;
    char c;
    int inject = 0;
    int openblocks = 0;
    int doinject = 0;
    for (u_int64_t i = 0; i < size; i++, c = *(prg + i)) {
        if (c == '\0' && i < size)
            continue;
        if (c == '%' && (i < size - 1) && *(prg + 1 + i) == '{') {
            i++;
            inject = 1;
            continue;
        }
        if (c == '}' && (i < size - 1) && *(prg + 1 + i) == '%') {
            i++;
            inject = 0;
            doinject = 1;
            continue;
        }
        if (c == '{')
            openblocks++;
        if (c == '}')
            openblocks--;

        if (c == '-' && (i < size - 1) && *(prg + 1 + i) == '>') {
            while (isspace(c)) {
                i++;
                c = *(prg + i);
            }
            c = *(prg + i);
            if (c == '{') {
                openblocks++;
                buffer[buflen++] = c;
                continue;
            }
        }
        if (doinject && openblocks == 0) {
            buffer = escapeables(buffer, size);
            memcpy(transformed + tlen, buffer, buflen);
            buffer[0] = '\0';
            buflen = 0;
            inject = 0;
            continue;
        }
        transformed[tlen++] = c;
    }
    free(buffer);
    return transformed;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        return 0;
    u_int64_t size;
    char* prg = ctr_internal_readf(argv[1], &size);
    char* t = transform(prg, size);
    printf("%s\n", t);
    free(t);
    free(prg);
    return 0;
}
