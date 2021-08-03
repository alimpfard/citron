#include "citron.h"
#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *np;

/**
 * Determines the size of the specified file.
 */
int fsize(char *filename) {
  int size;
  FILE *fh;
  fh = fopen(filename, "rb");
  if (fh != NULL) {
    if (fseek(fh, 0, SEEK_END)) {
      fclose(fh);
      return -1;
    }
    size = ftell(fh);
    fclose(fh);
    return size;
  }
  return -1;
}

/**
 * DebugTree
 *
 * For debugging purposes, prints the internal AST.
 */
void ctr_internal_debug_tree(ctr_tnode *ti, int indent) {
  char *str;
  char *vbuf;
  ctr_tlistitem *li;
  ctr_tnode *t;
  // if (indent > 20)
  //   exit (1);
  li = ti->nodes;
  if (!li)
    return;
  t = li->node;
  while (1) {
    if (!t)
      goto next;
    int i;
    for (i = 0; i < indent; i++)
      printf(" ");
    str = ctr_heap_allocate(40 * sizeof(char));
    switch (t->type) {
    case CTR_AST_NODE_EXPRASSIGNMENT:
      str = "ASSIGN";
      break;
    case CTR_AST_NODE_EXPRMESSAGE:
      str = "MESSAG";
      break;
    case CTR_AST_NODE_UNAMESSAGE:
      str = "UMSSAG";
      break;
    case CTR_AST_NODE_KWMESSAGE:
      str = "KMSSAG";
      break;
    case CTR_AST_NODE_BINMESSAGE:
      str = "BMSSAG";
      break;
    case CTR_AST_NODE_LTRSTRING:
      str = "STRING";
      break;
    case CTR_AST_NODE_REFERENCE:
      str = "REFRNC";
      break;
    case CTR_AST_NODE_LTRNUM:
      str = "NUMBER";
      break;
    case CTR_AST_NODE_CODEBLOCK:
      str = "CODEBL";
      break;
    case CTR_AST_NODE_RETURNFROMBLOCK:
      str = "RETURN";
      break;
    case CTR_AST_NODE_PARAMLIST:
      str = "PARAMS";
      break;
    case CTR_AST_NODE_INSTRLIST:
      str = "INSTRS";
      break;
    case CTR_AST_NODE_SYMBOL:
      str = "SYMBOL";
      break;
    case CTR_AST_NODE_ENDOFPROGRAM:
      str = "EOPROG";
      break;
    case CTR_AST_NODE_NESTED:
      str = "NESTED";
      break;
    case CTR_AST_NODE_LTRBOOLFALSE:
      str = "BFALSE";
      break;
    case CTR_AST_NODE_LTRBOOLTRUE:
      str = "BLTRUE";
      break;
    case CTR_AST_NODE_LTRNIL:
      str = "LTRNIL";
      break;
    case CTR_AST_NODE_IMMUTABLE:
      str = "IMMUTABLE";
      break;
    case CTR_AST_NODE_LISTCOMP:
      str = "LISTCOMP";
      break;
    case CTR_AST_NODE_NATIVEFN:
      str = "NATIVEFN";
      break;
    case CTR_AST_NODE_EMBED:
      str = "EMBED";
      break;
    case CTR_AST_NODE_RAW:
      str = "RAW";
      break;
    default:
      str = "UNKNW?";
      break;
    }
    vbuf = ctr_heap_allocate(sizeof(char) * (t->vlen + 1));
    if (t->type == CTR_AST_NODE_SYMBOL)
      strncpy(vbuf, "<Symbol>", 8);
    else
      strncpy(vbuf, t->value, t->vlen);
    printf("%s[%d] %s (%p)\n", str, t->modifier, vbuf, (void *)t);
    ctr_heap_free(vbuf);
    if (t->nodes && t->type != CTR_AST_NODE_EMBED)
      ctr_internal_debug_tree(t, indent + 1);
    if (t->type == CTR_AST_NODE_EMBED) {
      if (t->modifier) {
        int i;
        for (i = 0; i <= indent; i++)
          printf(" ");
        ctr_object *lname =
            ((ctr_object *)(t->modifier == 1 ? t->nodes->node
                                             : t->nodes->next->node))
                ->lexical_name;
        if (!lname)
          lname = ((ctr_object *)(t->modifier == 1 ? t->nodes->node
                                                   : t->nodes->next->node));
        ctr_object *str = ctr_internal_cast2string(lname);
        printf("%p type %d, %.*s\n", t->nodes->node,
               ((ctr_object *)t->nodes->node)->info.type,
               str->value.svalue->vlen, str->value.svalue->value);
      } else
        ctr_internal_debug_tree(t->nodes->node, indent + 1);
    }
  next:;
    if (!li->next)
      break;
    li = li->next;
    t = li->node;
  }
}

/**
 * @internal
 * Internal plugin loader.
 * Tries to locate a plugin by its name.
 *
 * In Citron a plugin is loaded automatically as soon as a symbol
 * has been enountered that cannot be found in global scope.
 *
 * Before raising an error Citron will attempt to load a plugin
 * to meet the dependency.
 *
 * To install a plugin, copy the plugin folder to the mods folder
 * in the extensions directory of the interpreter.
 *
 * So, for instance, to install the 'Coffee Percolator Plugin',
 * we copy the libctrpercolator.so in folder mods/percolator/,
 * the resulting path is: mods/percolator/libctrpercolator.so.
 *
 * General path format:
 *
 * mods/X/libctrX.so
 *
 * where X is the name of the object the plugin offers in lowercase.
 *
 * On loading, the plugin will get a chance to add its objects to the world
 * through a constructor function.
 */
typedef void *(*plugin_init_func)();
void *ctr_internal_plugin_find(ctr_object *key) {
  ctr_object *modNameObject = key->info.type == CTR_OBJECT_TYPE_OTSTRING
                                  ? key
                                  : ctr_internal_cast2string(key);
  void *handle;
  char pathNameMod[1024];
  char *modName;
  char *modNameLow;
  plugin_init_func init_plugin;
  char *realPathModName = NULL;
  modName = ctr_heap_allocate_cstring(modNameObject);
  modNameLow = modName;
  char const *extPath = ctr_file_stdext_path_raw();
  for (; *modNameLow; ++modNameLow)
    *modNameLow = tolower(*modNameLow);
  snprintf(pathNameMod, 1024, ("%s/mods/%s/libctr%s.so"), extPath, modName,
           modName);
  ctr_heap_free(modName);
  realPathModName = realpath(pathNameMod, NULL);
  if (access(realPathModName, F_OK) == -1)
    return NULL;
  handle = dlopen(realPathModName, RTLD_NOW);
  if (!handle) {
    printf("%s: %s\n", "Failed to open file", dlerror());
    return NULL;
  }
  *(void **)(&init_plugin) = dlsym(handle, "begin");
  if (!init_plugin)
    return NULL;
  (void)init_plugin();
  return handle;
}

void *ctr_internal_plugin_find_base(char const *modName) {
  char const *extPath = ctr_file_stdext_path_raw();
  void *handle;
  char pathNameMod[1024];
  plugin_init_func init_plugin;
  char *realPathModName = NULL;
  snprintf(pathNameMod, 1024, ("%s/basemods/%s/libctr%s.so"), extPath, modName,
           modName);
  realPathModName = realpath(pathNameMod, NULL);
  if (access(realPathModName, F_OK) == -1) {
    printf("Error: %s\n", strerror(errno));
    return NULL;
  }
  handle = dlopen(realPathModName, RTLD_NOW);
  if (!handle) {
    printf("%s: %s\n", "Failed to open file", dlerror());
    return NULL;
  }
  *(void **)(&init_plugin) = dlsym(handle, "begin");
  if (!init_plugin)
    return NULL;
  (void)init_plugin();
  return handle;
}

ctr_object *ctr_format_str(const char *str_format, ...) {
  va_list ap;
  char psbf[1024];
  char *ps = ctr_heap_allocate(sizeof(char) * 1024);
  int len = 0, reserved = 1024;
  int inner_s_len = 0;
  va_start(ap, str_format);
  int i = 0, interpret = 0;
  for (char c = str_format[i]; (c = str_format[++i]) != '\0';) {
    if (reserved - len < 32)
      ps = ctr_heap_reallocate(ps, reserved *= 2);
    if (interpret) {
      interpret = 0;
      switch (c) {
      case '.': // size spec
        inner_s_len = va_arg(ap, long);
        break;
      case '%': // literal percent sign
        ps[len++] = '%';
        break;
      case 's': { // c string
        const char *s = va_arg(ap, const char *);
        int slen = inner_s_len ?: strlen(s);
        inner_s_len = 0;
        if (reserved - len < 32 + slen)
          ps = ctr_heap_reallocate(ps, reserved *= 2);
        strncpy(ps + len, s, slen);
        len += slen;
        break;
      }
      case 'd': { // c int
        int d = va_arg(ap, int);
        int slen = sprintf(psbf, "%d", d);
        if (reserved - len < 32 + slen)
          ps = ctr_heap_reallocate(ps, reserved *= 2);
        strncpy(ps + len, psbf, slen);
        len += slen;
        break;
      }
      case 'S': { // ctr string
        ctr_object *ctrs = va_arg(ap, ctr_object *);
        int slen = ctrs->value.svalue->vlen;
        if (reserved - len < 32 + slen)
          ps = ctr_heap_reallocate(ps, reserved *= 2);
        strncpy(ps + len, ctrs->value.svalue->value, ctrs->value.svalue->vlen);
        len += slen;
        break;
      }
      case 'I': { // ctr number
        ctr_object *ctrn = va_arg(ap, ctr_object *);
        int slen = sprintf(psbf, "%f", ctrn->value.nvalue);
        if (reserved - len < 32 + slen)
          ps = ctr_heap_reallocate(ps, reserved *= 2);
        strncpy(ps + len, psbf, slen);
        len += slen;
        break;
      }
      case '$': {
        ctr_object *ctrs = ctr_internal_cast2string(va_arg(ap, ctr_object *));
        int slen = ctrs->value.svalue->vlen;
        if (reserved - len < 32 + slen)
          ps = ctr_heap_reallocate(ps, reserved *= 2);
        strncpy(ps + len, ctrs->value.svalue->value, ctrs->value.svalue->vlen);
        len += slen;
        break;
      }
      default:
        break;
      }
      continue;
    }
    if (c == '%') {
      interpret = 1;
      continue;
    }
    ps[len++] = c;
  }
  va_end(ap);
  ctr_object *ss = ctr_build_string(ps, len);
  ctr_heap_free(ps);
  return ss;
}
char *ctr_itoa(int value, char *result, int base, const char *map) {
  if (!map)
    map = "zyxwvutsrqponmlkjihgfedcba9876543210123456789"
          "abcdefghijklmnopqrstuvwxyz";

  if (base < 2 || base > 36) {
    *result = '\0';
    return result;
  }

  char *ptr = result, *ptr1 = result, tmp_char;
  int tmp_value;

  do {
    tmp_value = value;
    value /= base;
    *ptr++ = map[35 + (tmp_value - value * base)];
  } while (value);

  if (tmp_value < 0)
    *ptr++ = '-';
  *ptr-- = '\0';
  while (ptr1 < ptr) {
    tmp_char = *ptr;
    *ptr-- = *ptr1;
    *ptr1++ = tmp_char;
  }
  return result;
}
