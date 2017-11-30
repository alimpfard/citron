#include <stdlib.h>
#include <stdio.h>

struct a {
  int a;
  char* b;
};

void test(struct a b) {
  printf("%d %s\n", b.a, b.b);
}
