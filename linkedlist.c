#ifndef LLHC
#define LLHC
#include <assert.h>
#include <stdlib.h>

typedef struct ll_s {
  struct ll_s *next;
  struct ll_s *prev;
  int value;
} ll;

ll *llcreate(int root) {
  ll *ll_v = malloc(sizeof(*ll_v));
  ll_v->value = root;
  ll_v->next = 0;
  ll_v->prev = 0;
  return ll_v;
}

void llfree(ll *ll) {
  if (!ll)
    return;
  llfree(ll->next);
  llfree(ll->prev);
  free(ll);
}

ll *llconcat(ll *head, ll *tail) {
  if (!head && !tail)
    assert(0);
  if (!tail && head)
    return head;
  if (!head && tail)
    return tail;
  assert(!head->next);
  assert(!tail->prev);
  head->next = tail;
  tail->prev = head;
  return head;
}

ll *llconcat_and_add(int head, int last, ll *llv) {
  ll *llh = llcreate(head);
  ll *llt = llcreate(last);
  return llconcat(llconcat(llh, llv), llt);
}

ll *llpush(ll *llv, int head) {
  ll *llh = llcreate(head);
  return llconcat(llh, llv);
}

int llat(ll *llv, size_t index) {
  ll *llc = llv;
  while (llc && index) {
    llc = llc->next;
    index--;
  }
  if (index || !llc)
    assert(0);
  return llc->value;
}

int llput(ll *llv, size_t index, int value) {
  ll *llc = llv;
  while (llc && index) {
    llc = llc->next;
    index--;
  }
  if (index || !llc)
    assert(0);
  return llc->value = value;
}

size_t lllength(ll *llv) {
  ll *llc = llv;
  size_t index = 0;
  while (llc) {
    llc = llc->next;
    index++;
  }
  return index;
}

int llall(ll *llv, int (*fn)(int)) {
  if (!llv)
    return 0;
  ll *llc = llv;
  int all = 1;
  while (llc && all) {
    all = all && fn(llc->value);
    llc = llc->next;
  }
  return all;
}

int lllastIndexOf(ll *llv, int val) {
  if (!llv)
    return -1;
  ll *llc = llv;
  int index = -1, idx = -1;
  while (llc) {
    idx++;
    if (llc->value == val)
      index = idx;
    llc = llc->next;
  }
  return index;
}

int llindexOf(ll *llv, int val) {
  if (!llv)
    return -1;
  ll *llc = llv;
  int index = -1, idx = -1;
  while (llc) {
    idx++;
    if (llc->value == val)
      return idx;
    llc = llc->next;
  }
  return index;
}

ll *llslice(ll *llv, int start, int end) {
  while (start) {
    assert(llv);
    start--;
    end--;
    llv = llv->next;
  }
  ll *newll = llcreate(llv->value);
  while (llv) {
    end--;
    llv = llv->next;
    if (!llv)
      break;
    newll = llconcat(llcreate(llv->value), newll);
  }
  assert(end < 1);
  return newll;
}

int llfilterlen(ll *llv, int (*fn)(int)) {
  if (!llv)
    return 0;
  ll *llc = llv;
  int len = 0;
  while (llc) {
    len = len + !!fn(llc->value);
    llc = llc->next;
  }
  return len;
}
#endif /* end of include guard: LLHC */
