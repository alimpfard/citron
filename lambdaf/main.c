#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shared/lib/readfile.h"
#include "shared/lib/tree.h"
#include "shared/lib/array.h"
#include "shared/lib/freelist.h"
#include "shared/lib/util.h"
#include "shared/term.h"
#include "parse/parse.h"
#include "evaluate/closure.h"
#include "evaluate/evaluate.h"
#include "lstdlib.h"

bool TEST = false;

static void print3(const char* a, const char* b, const char* c) {
    fputs(a, stderr);
    fputs(b, stderr);
    fputs(c, stderr);
}

void usageError(const char* name) {
    print3("Usage error: ", name, " [-d] [-D] [-t] [FILE]\n");
    exit(2);
}

void readError(const char* filename) {
    print3("Usage error: file '", filename, "' cannot be opened\n");
    exit(2);
}

void memoryError(const char* label, long long bytes) {
    print3("MEMORY LEAK IN \"", label, "\": ");
    fputll(bytes, stderr);
    fputs(" bytes\n", stderr);
    exit(3);
}

static void checkForMemoryLeak(const char* label, size_t expectedUsage) {
    size_t usage = getMemoryUsage();
    if (usage != expectedUsage)
        memoryError(label, (long long)(usage - expectedUsage));
}

void lambdaf_interpret(char* src) {
  initNodeAllocator();
  char *ivsp = malloc(lambdaf_stdlib_zero_len+strlen(src)+2);
  strcpy(ivsp, (char*)lambdaf_stdlib_zero);
  ivsp[lambdaf_stdlib_zero_len] = '\n';
  strcpy(ivsp+lambdaf_stdlib_zero_len+1, src);
  Program program = parse(ivsp);
  size_t memoryUsageBeforeEvaluate = getMemoryUsage();
  Hold* valueClosure = evaluateTerm(program.entry, program.globals);
  size_t memoryUsageBeforeSerialize = getMemoryUsage();
  checkForMemoryLeak("serialize", memoryUsageBeforeSerialize);
  release(valueClosure);
  checkForMemoryLeak("evaluate", memoryUsageBeforeEvaluate);
  deleteProgram(program);
  checkForMemoryLeak("parse", 0);
  destroyNodeAllocator();
}

void lambdaf_begin_instance() {
}

void lambdaf_end_instance() {
}
