#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include "citron.h"
#include "siphash.h"
#include <dirent.h>
#include <wordexp.h>

#ifndef WORDEXP_READY
#define WORDEXP_READY 1
#endif

#include <termios.h>
static struct termios oldTermios, newTermios;

#include "generator.h"

/**@I_OBJ_DEF File*/

/**
 * File
 *
 * Represents a File object.
 * Creates a new file object based on the specified path.
 *
 * Usage:
 *
 * File new: '/example/path/to/file.txt'.
 */
ctr_object *ctr_file_new(ctr_object *myself, ctr_argument *argumentList) {
  ctr_check_permission(CTR_SECPRO_NO_FILE_READ);
  ctr_object *s = ctr_object_make(myself, argumentList);
  ctr_object *pathObject;
  s->info.type = CTR_OBJECT_TYPE_OTEX; /* indicates resource for GC */
  ctr_set_link_all(s, myself);
  s->value.rvalue = NULL;
  pathObject = ctr_build_string(argumentList->object->value.svalue->value,
                                argumentList->object->value.svalue->vlen);
  ctr_internal_object_add_property(s, ctr_build_string_from_cstring("path"),
                                   pathObject, 0);
  return s;
}

ctr_object *ctr_file_special(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *file;
  ctr_resource *rs = ctr_heap_allocate(sizeof(ctr_resource));
  if (ctr_internal_object_is_equal(CTR_FILE_STDIN_STR, argumentList->object)) {
    file = ctr_file_new(myself, argumentList);
    rs->ptr = stdin;
    rs->type = 1;
  } else if (ctr_internal_object_is_equal(CTR_FILE_STDOUT_STR,
                                          argumentList->object)) {
    file = ctr_file_new(myself, argumentList);
    rs->ptr = stdout;
    rs->type = 1;
  } else if (ctr_internal_object_is_equal(CTR_FILE_STDERR_STR,
                                          argumentList->object)) {
    file = ctr_file_new(myself, argumentList);
    rs->ptr = stderr;
    rs->type = 1;
  } else if (argumentList->object->interfaces->link == CtrStdNumber) {
    ctr_argument arg;
    arg.object = ctr_number_to_string(argumentList->object, NULL);
    file = ctr_file_new(myself, &arg);
    rs->ptr = fdopen((int)argumentList->object->value.nvalue,
                     argumentList->next->object->value.svalue->value);
    rs->type = 1;
  } else
    return CtrStdNil;
  file->value.rvalue = rs;
  return file;
}

/**
 * [File] fileDescriptor
 *
 * returns the decsriptor of an opened File
 */
ctr_object *ctr_file_get_descriptor(ctr_object *myself,
                                    ctr_argument *argumentList) {
  FILE *ptr;
  if (!myself->value.rvalue || !(ptr = myself->value.rvalue->ptr))
    return CtrStdNil;
  int fd = fileno(ptr);
  return ctr_build_number_from_float(fd);
}

/**
 * [File] memopen: [String] mode: [Number]
 *
 * Opens a file backed by a string
 */
ctr_object *ctr_file_memopen(ctr_object *myself, ctr_argument *argumentList) {
  ctr_check_permission(CTR_SECPRO_NO_FILE_READ);
  char *mode = ctr_heap_allocate_cstring(
      ctr_internal_cast2string(argumentList->next->object));
  ctr_string *str = argumentList->object->value.svalue;
  FILE *handle = fmemopen(str->value, str->vlen, mode);
  ctr_heap_free(mode);
  ctr_object *s = ctr_object_make(myself, argumentList);
  s->info.type = CTR_OBJECT_TYPE_OTEX; /* indicates resource for GC */
  ctr_set_link_all(s, myself);
  ctr_resource *rs = ctr_heap_allocate(sizeof(ctr_resource));
  rs->type = 1;
  rs->ptr = handle;
  s->value.rvalue = rs;
  return s;
}

/**
 * [File] duplicateDescriptor: [Number]
 *
 * duplicates a file descriptor (dup)
 */
/**
 * [File] duplicateDescriptor: [Number] toDescriptor: [Number]
 *
 * duplicates a file descriptor to a desired descriptor (dup2)
 */
/**
 * [File] duplicateDescriptor: [Number] toDescriptor: [Number] withFlags:
 * [Number]
 *
 * duplicates a file descriptor to a desired descriptor with specific flags
 * (dup3) Note: only with forLinux
 */
ctr_object *ctr_file_ddup(ctr_object *myself, ctr_argument *argumentList) {
  int oldfd = argumentList->object->value.nvalue;
  if (argumentList->next && argumentList->next->object) {
    int desiredfd = argumentList->next->object->value.nvalue;
#if forLinux
    if (argumentList->next->next && argumentList->next->next->object) {
      int flags = argumentList->next->next->object->value.nvalue;
      if (oldfd == desiredfd)
        return ctr_build_number_from_float(oldfd);
      int fd = dup3(oldfd, desiredfd, flags);
      if (fd == -1) {
        int err = errno;
        switch (err) {
        case EBADF:
          CtrStdFlow = ctr_format_str(
              "E%d is not an open file descriptor, or is out of range", fd);
          break;
        case EINTR:
          CtrStdFlow = ctr_format_str("EInterrupted");
          break;
        // case EINVAL:CtrStdFlow = ctr_format_str("E%d = %d? or the flags (%d)
        // are invalid", fd, newfd, flags); break;
        case EMFILE:
          CtrStdFlow = ctr_format_str(
              "EThe max limit of open descriptors has been reached");
          break;
        }
        return CtrStdNil;
      }
      return ctr_build_number_from_float(fd);
    } else {
#endif
      if (oldfd == desiredfd)
        return ctr_build_number_from_float(oldfd);
      int fd = dup2(oldfd, desiredfd);
      if (fd == -1) {
        int err = errno;
        switch (err) {
        case EBADF:
          CtrStdFlow = ctr_format_str(
              "E%d is not an open file descriptor, or is out of range", fd);
          break;
        case EINTR:
          CtrStdFlow = ctr_format_str("EInterrupted");
          break;
        // case EINVAL:CtrStdFlow = ctr_format_str("E%d = %d? or the flags (%d)
        // are invalid", fd, newfd, flags); break;
        case EMFILE:
          CtrStdFlow = ctr_format_str(
              "EThe max limit of open descriptors has been reached");
          break;
        }
        return CtrStdNil;
      }
      return ctr_build_number_from_float(fd);
#if forLinux
    }
#endif
  } else {
    int fd = dup(oldfd);
    if (fd == -1) {
      int err = errno;
      switch (err) {
      case EBADF:
        CtrStdFlow = ctr_format_str(
            "E%d is not an open file descriptor, or is out of range", fd);
        break;
      case EINTR:
        CtrStdFlow = ctr_format_str("EInterrupted");
        break;
      // case EINVAL:CtrStdFlow = ctr_format_str("E%d = %d? or the flags (%d)
      // are invalid", fd, newfd, flags); break;
      case EMFILE:
        CtrStdFlow = ctr_format_str(
            "EThe max limit of open descriptors has been reached");
        break;
      }
      return CtrStdNil;
    }
    return ctr_build_number_from_float(fd);
  }
}

ctr_object *ctr_file_stdext_path(ctr_object *myself,
                                 ctr_argument *argumentList) {
  ctr_object *path =
      ctr_build_string_from_cstring((char *)ctr_file_stdext_path_raw());
  return path;
}

struct ctr_file_gen {
  ctr_object *fp;
  FILE *FILEp;
  ctr_object *next;
  int vopen;
  int block;
  int noblock;
  int finished;
};

extern ctr_object *generator_end_marker;
static ctr_object *ctr_file_generate_lines_next(ctr_object *myself,
                                                ctr_argument *argumentList) {
  struct ctr_file_gen *val =
      argumentList->object->value.rvalue->ptr; // todo: error check
  if (val->finished) {
    CtrStdFlow = CtrStdBreak;
    return generator_end_marker;
  }
  if (val->vopen) {
    if (!val->fp->value.rvalue->ptr) {
      val->vopen = 0;
      CtrStdFlow = CtrStdBreak; // signal end of sequence
      return val->next ?: CtrStdNil;
    }
  nowopen:;
    FILE *f = val->FILEp;
    size_t lineBufLen = 0;
    char *lineptr = NULL;
    ssize_t len = getline(&lineptr, &lineBufLen, f);
    if (len == 0 || len == -1) {
      if (val->block) {
        while (len <= 0)
          len = getline(&lineptr, &lineBufLen, f);
      } else if (val->noblock) {
        free(lineptr);
        return CtrStdNil;
      } else {
        if (f == stdin) {
          // no touching stdin
        } else {
          val->vopen = 0;
          fclose(f);
          ctr_resource *res = val->fp->value.rvalue;
          if (f == res->ptr)
            res->ptr = 0;
        }
        free(lineptr);
        CtrStdFlow = CtrStdBreak; // signal end of sequence
        return val->next ?: CtrStdNil;
      }
    }
    ctr_object *prev = val->next;
    if (!prev) {
      prev = val->next = ctr_build_string(lineptr, len);
      goto nowopen;
    }
    val->next = ctr_build_string(lineptr, len);
    free(lineptr);
    return prev;
  } else {
    if (val->fp->value.rvalue && val->fp->value.rvalue->ptr == stdin) {
      val->vopen = 1;
      val->FILEp = stdin;
      goto nowopen;
    }
    ctr_object *path = ctr_file_rpath(val->fp, NULL);
    ctr_size vlen;
    char *pathString;
    FILE *f;
    if (!path->value.svalue) {
      CtrStdFlow = ctr_build_string_from_cstring("invalid file.");
      return generator_end_marker;
    }
    vlen = path->value.svalue->vlen;
    pathString = ctr_heap_allocate(sizeof(char) * (vlen + 1));
    memcpy(pathString, path->value.svalue->value, vlen);
    memcpy(pathString + vlen, "\0", 1);
    f = fopen(pathString, "rb");
    ctr_heap_free(pathString);
    if (!f) {
      CtrStdFlow = ctr_build_string_from_cstring("Unable to open file.");
      return generator_end_marker;
    }
    ctr_resource *rv = ctr_heap_allocate(sizeof(*rv));
    rv->ptr = f;
    rv->type = 1;
    val->fp->value.rvalue = rv;
    val->vopen = 1;
    val->FILEp = f;
    goto nowopen;
  }
}
ctr_object *ctr_file_generate_lines(ctr_object *myself,
                                    ctr_argument *argumentList) {
  ctr_object *inst = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all(inst, ctr_std_generator);
  inst->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
  ctr_resource *res = inst->value.rvalue;
  res->type = CTR_IFN_OF_GENNY;
  ctr_generator *gen = ctr_heap_allocate(sizeof(*gen));
  ctr_mapping_generator *genny = ctr_heap_allocate(sizeof(*genny));
  gen->finished = 0;
  genny->i_type = CTR_REPEAT_GENNY;
  ctr_object *methodObject =
      ctr_internal_create_object(CTR_OBJECT_TYPE_OTNATFUNC);
  ctr_set_link_all(methodObject, CtrStdBlock);
  methodObject->value.fvalue = ctr_file_generate_lines_next;
  genny->fn = methodObject;
  gen->sequence = genny;
  gen->data = ctr_heap_allocate(sizeof(ctr_argument));
  res->ptr = gen;
  inst->release_hook = ctr_generator_free;

  ctr_object *rpt_v = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  rpt_v->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
  ctr_resource *resv = rpt_v->value.rvalue;
  ctr_set_link_all(rpt_v, CtrStdObject);
  ctr_generator *rptgen = ctr_heap_allocate(sizeof(*gen));
  rptgen->finished = 0;
  rptgen->sequence = rpt_v;
  struct ctr_file_gen *alloc = ctr_heap_allocate(sizeof(*alloc));
  alloc->fp = myself;
  alloc->vopen = 0;
  if (myself->value.rvalue && myself->value.rvalue->ptr) {
    alloc->FILEp = myself->value.rvalue->ptr;
    alloc->vopen = 1;
  }

  alloc->block =
      /* block */ argumentList->object
          ? ctr_internal_cast2bool(argumentList->object)->value.bvalue
          : 0;
  alloc->noblock = /* noblock */ 0;

  resv->ptr = alloc;
  genny->genny = rptgen;
  return inst;
}

/**
 * [File] tempFileLike: [String]
 *
 * Represents a File object.
 * Creates a new temporary file object based on the specified path.
 * replacing XXXX with some chars.
 * Usage:
 *
 * File tempFileLike: '/example/path/to/fileXXXXXXXX'.
 */

ctr_object *ctr_file_tmp(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *s;
  ctr_size vlen;
  char *pathString;
  vlen = argumentList->object->value.svalue->vlen;
  pathString = ctr_heap_allocate(sizeof(char) * (vlen + 1));
  if (pathString == NULL)
    return CtrStdNil;
  memcpy(pathString, argumentList->object->value.svalue->value, vlen);
  memcpy(pathString + vlen, "\0", 1);
  FILE *f;
  int fd = mkstemp(pathString);
  if (fd == -1)
    goto err;
  f = fdopen(fd, "rb");
  if (f == NULL) {
  err:;
    char *buf = ctr_heap_allocate(sizeof(char) * 1024);
    sprintf(buf, "%d: %s", fd, strerror(errno));
    CtrStdFlow = ctr_build_string_from_cstring(buf);
    ctr_heap_free(buf);
    return CtrStdFlow;
  }
  fclose(f);
  ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
  args->object = ctr_build_string_from_cstring(pathString);
  s = ctr_file_new(myself, args);
  ctr_heap_free(pathString);
  ctr_heap_free(args);
  return s;
}

/**
 * [File] path
 *
 * Returns the path of a file. The file object will respond to this
 * message by returning a string object describing the full path to the
 * recipient.
 */
ctr_object *ctr_file_path(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *path = ctr_internal_object_find_property(
      myself, ctr_build_string_from_cstring("path"), 0);
  if (path == NULL)
    return CtrStdNil;
  return path;
}

/**
 * [File] unpack: [String:Ref], [Object:ctx]
 * Assigns the file instance to the reference
 * (Always prefer using algebraic deconstruction assignments: look at section
 * 'Assignment')
 */
ctr_object *ctr_file_assign(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_object_assign(myself, argumentList);
}

/**
 * [File] realPath
 *
 * Returns the real path of a file. The file object will respond to this
 * message by returning a string object describing the absolute path to the
 * recipient.
 */
ctr_object *ctr_file_rpath(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *path = ctr_internal_object_find_property(
      myself, ctr_build_string_from_cstring("path"), 0);
  if (path == NULL)
    return CtrStdNil;
  char *cpath = ctr_heap_allocate_cstring(path);
  char rpath[PATH_MAX + 1];
  char *ret = realpath(cpath, rpath);
  if (!ret) {
    if (WORDEXP_READY) {
      wordexp_t exp_result;
      int st = wordexp(cpath, &exp_result, 0);
      if (!st) {
        char *r = exp_result.we_wordv[0];
        memset(rpath, 0, PATH_MAX);
        memcpy(rpath, r, strlen(r));
        wordfree(&exp_result);
      } else {
        char *err;
        switch (st) {
        case WRDE_BADCHAR:
          err = "Bad character";
          break;
        case WRDE_BADVAL:
          err = "Bad value";
          break;
        case WRDE_CMDSUB:
          err = "command substitution requested";
          break;
        case WRDE_NOSPACE:
          err = "Out of memory";
          break;
        case WRDE_SYNTAX:
          err = "Shell syntax error";
          break;
        default:
          err = "Unknown error";
        }
        ctr_heap_free(cpath);
        CtrStdFlow = ctr_build_string_from_cstring(err);
        return CtrStdNil;
      }
    } else {
      ctr_heap_free(cpath);
      CtrStdFlow = ctr_build_string_from_cstring(strerror(errno));
      return CtrStdNil;
    }
  }
  path = ctr_build_string_from_cstring(rpath);
  ctr_heap_free(cpath);
  return path;
}

/**
 * [File] expand: [String: path]
 *
 * Expands a given path by user paths
 */
ctr_object *ctr_file_expand(ctr_object *myself, ctr_argument *argumentList) {
  if (argumentList->object == NULL)
    return CtrStdNil;
  ctr_object *path = ctr_internal_cast2string(argumentList->object);
  char *cpath = ctr_heap_allocate_cstring(path);
  wordexp_t exp_result;
  int st = wordexp(cpath, &exp_result, 0);
  ctr_object *arr;
  if (!st) {
    arr = ctr_array_new(CtrStdArray, NULL);
    char *r;
    int i = 0;
    while (1) {
      r = exp_result.we_wordv[i++];
      if (!r)
        break;
      argumentList->object = ctr_build_string_from_cstring(r);
      ctr_array_push(arr, argumentList);
    }
    wordfree(&exp_result);
  } else {
    char *err;
    switch (st) {
    case WRDE_BADCHAR:
      err = "Bad character";
      break;
    case WRDE_BADVAL:
      err = "Bad value";
      break;
    case WRDE_CMDSUB:
      err = "command substitution requested";
      break;
    case WRDE_NOSPACE:
      err = "Out of memory";
      break;
    case WRDE_SYNTAX:
      err = "Shell syntax error";
      break;
    default:
      err = "Unknown error";
    }
    ctr_heap_free(cpath);
    CtrStdFlow = ctr_build_string_from_cstring(err);
    return CtrStdNil;
  }
  return arr;
}

/**
 * [File] read
 *
 * Reads contents of a file. Send this message to a file to read the entire
 * contents in one go. For big files you might want to prefer a streaming
 * approach to avoid memory exhaustion (see readBytes etc).
 *
 * Usage:
 *
 * data := File new: '/path/to/mydata.csv', read.
 *
 * In the example above we read the contents of the entire CSV file callled
 * mydata.csv in the variable called data.
 */
ctr_object *ctr_file_read(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *path = ctr_file_rpath(myself, NULL);
  ctr_object *str;
  ctr_size vlen;
  ssize_t fileLen;
  char *pathString;
  char *buffer;
  FILE *f;
  if (!myself->value.rvalue || !myself->value.rvalue->ptr) {
    if (path == NULL)
      return CtrStdNil;
    vlen = path->value.svalue->vlen;
    pathString = ctr_heap_allocate(sizeof(char) * (vlen + 1));
    memcpy(pathString, path->value.svalue->value, vlen);
    memcpy(pathString + vlen, "\0", 1);
    f = fopen(pathString, "rb");
    ctr_heap_free(pathString);
    if (!f) {
      CtrStdFlow = ctr_build_string_from_cstring("Unable to open file.");
      CtrStdFlow->info.sticky = 1;
      return CtrStdNil;
    }
    fseek(f, 0, SEEK_END);
    fileLen = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = (char *)ctr_heap_allocate(fileLen + 1);
    if (!buffer) {
      printf("Out of memory\n");
      fclose(f);
      exit(1);
    }
    fread(buffer, fileLen, 1, f);
    fclose(f);
  } else {
    f = myself->value.rvalue->ptr;
    if (unlikely(f != stdin)) {
      fseek(f, 0, SEEK_END);
      fileLen = ftell(f);
      fseek(f, 0, SEEK_SET);
      buffer = (char *)ctr_heap_allocate(fileLen + 1);
      if (!buffer) {
        printf("Out of memory\n");
        fclose(f);
        exit(1);
      }
      fread(buffer, fileLen, 1, f);
    } else {
      char c;
#ifdef withTermios
      tcgetattr(STDIN_FILENO, &oldTermios);
      newTermios = oldTermios;
      cfmakeraw(&newTermios);
      tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);
#endif
      while ((c = fgetc(stdin)) == EOF)
        ;
#ifdef withTermios
      tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
#endif
      buffer = &c;
      fileLen = 1;
    }
  }
  str = ctr_build_string(buffer, fileLen);
  ctr_heap_free(buffer);
  return str;
}

/**
 * [File] write: [String]
 *
 * Writes content to a file. Send this message to a file object to write the
 * entire contents of the specified string to the file in one go. The file
 * object responds to this message for convience reasons, however for big files
 * it might be a better idea to use the streaming API if possible (see readBytes
 * etc.).
 *
 * data := '<xml>hello</xml>'.
 * File new: 'myxml.xml', write: data.
 *
 * In the example above we write the XML snippet in variable data to a file
 * called myxml.xml in the current working directory.
 */
ctr_object *ctr_file_write(ctr_object *myself, ctr_argument *argumentList) {
  ctr_check_permission(CTR_SECPRO_NO_FILE_WRITE);
  ctr_object *str = ctr_internal_cast2string(argumentList->object);
  ctr_object *path = ctr_file_rpath(myself, NULL);
  if (!myself->value.rvalue || !myself->value.rvalue->ptr) {
    FILE *f;
    ctr_size vlen;
    char *pathString;
    if (path == NULL)
      return CtrStdNil;
    vlen = path->value.svalue->vlen;
    pathString = ctr_heap_allocate(vlen + 1);
    memcpy(pathString, path->value.svalue->value, vlen);
    memcpy(pathString + vlen, "\0", 1);
    f = fopen(pathString, "wb+");
    ctr_heap_free(pathString);
    if (!f) {
      CtrStdFlow = ctr_build_string_from_cstring("Unable to open file.");
      CtrStdFlow->info.sticky = 1;
      return CtrStdNil;
    }
    fwrite(str->value.svalue->value, sizeof(char), str->value.svalue->vlen, f);
    fclose(f);
  } else {
    fwrite(str->value.svalue->value, sizeof(char), str->value.svalue->vlen,
           myself->value.rvalue->ptr);
  }
  return myself;
}

/**
 * [File] append: [String]
 *
 * Appends content to a file. The file object responds to this message like it
 * responds to the write-message, however in this case the contents of the
 * string will be appended to the existing content inside the file.
 */
ctr_object *ctr_file_append(ctr_object *myself, ctr_argument *argumentList) {
  ctr_check_permission(CTR_SECPRO_NO_FILE_WRITE);
  ctr_object *str = ctr_internal_cast2string(argumentList->object);
  ctr_object *path = ctr_file_rpath(myself, NULL);
  if (!myself->value.rvalue || !myself->value.rvalue->ptr) {
    ctr_size vlen;
    char *pathString;
    FILE *f;
    if (path == NULL)
      return myself;
    vlen = path->value.svalue->vlen;
    pathString = ctr_heap_allocate(vlen + 1);
    memcpy(pathString, path->value.svalue->value, vlen);
    memcpy(pathString + vlen, "\0", 1);
    f = fopen(pathString, "ab+");
    ctr_heap_free(pathString);
    if (!f) {
      CtrStdFlow = ctr_build_string_from_cstring("Unable to open file.\0");
      CtrStdFlow->info.sticky = 1;
      return CtrStdNil;
    }
    fwrite(str->value.svalue->value, sizeof(char), str->value.svalue->vlen, f);
    fclose(f);
  } else
    fwrite(str->value.svalue->value, sizeof(char), str->value.svalue->vlen,
           myself->value.rvalue->ptr);
  return myself;
}

/**
 * [File] exists
 *
 * Returns True if the file exists and False otherwise.
 */
ctr_object *ctr_file_exists(ctr_object *myself, ctr_argument *argumentList) {
  if (myself->value.rvalue && myself->value.rvalue->ptr)
    return ctr_build_bool(1);
  ctr_object *path = ctr_file_rpath(myself, NULL);
  if (CtrStdFlow) {
    CtrStdFlow = NULL;
    return ctr_build_bool(0);
  }
  ctr_size vlen;
  char *pathString;
  FILE *f;
  int exists;
  if (path == NULL || path->value.svalue == NULL)
    return ctr_build_bool(0);
  vlen = path->value.svalue->vlen;
  pathString = ctr_heap_allocate(vlen + 1);
  memcpy(pathString, path->value.svalue->value, vlen);
  memcpy(pathString + vlen, "\0", 1);
  static struct stat sb;
  exists = stat(pathString, &sb) == 0;
  ctr_heap_free(pathString);
  return ctr_build_bool(exists);
}

/**
 * [File] include
 *
 * Includes the file as a piece of executable code.
 */
ctr_object *ctr_file_include(ctr_object *myself, ctr_argument *argumentList) {
  ctr_check_permission(CTR_SECPRO_NO_INCLUDE);
  ctr_object *path = ctr_file_rpath(myself, NULL);
  ctr_tnode *parsedCode;
  ctr_size vlen;
  char *pathString;
  char *prg;
  uint64_t program_size = 0;
  if (path == NULL)
    return myself;
  vlen = path->value.svalue->vlen;
  pathString = ctr_heap_allocate_tracked(
      sizeof(char) *
      (vlen + 1)); // needed until end, pathString appears in stracktrace
  memcpy(pathString, path->value.svalue->value, vlen);
  memcpy(pathString + vlen, "\0", 1);
  if (!myself->value.rvalue || !myself->value.rvalue->ptr)
    prg = ctr_internal_readf(pathString, &program_size);
  else
    return ctr_build_nil();
  parsedCode = ctr_cparse_parse(prg, pathString);
  ctr_heap_free(prg);
  ctr_cwlk_subprogram++;
  ctr_open_context();
  ctr_cwlk_run(parsedCode);
  ctr_close_context();
  ctr_cwlk_subprogram--;
  return myself;
}

/**
 * [File] runHere
 *
 * Includes the file as a piece of executable code.
 */
ctr_object *ctr_file_include_here(ctr_object *myself,
                                  ctr_argument *argumentList) {
  ctr_check_permission(CTR_SECPRO_NO_INCLUDE);
  ctr_object *path = ctr_file_rpath(myself, NULL);
  ctr_tnode *parsedCode;
  ctr_size vlen;
  char *pathString;
  char *prg;
  uint64_t program_size = 0;
  if (path == NULL)
    return myself;
  vlen = path->value.svalue->vlen;
  pathString = ctr_heap_allocate_tracked(
      sizeof(char) *
      (vlen + 1)); // needed until end, pathString appears in stracktrace
  memcpy(pathString, path->value.svalue->value, vlen);
  memcpy(pathString + vlen, "\0", 1);
  prg = ctr_internal_readf(pathString, &program_size);
  parsedCode = ctr_cparse_parse(prg, pathString);
  ctr_heap_free(prg);
  ctr_cwlk_subprogram++;
  ctr_cwlk_run(parsedCode);
  ctr_cwlk_subprogram--;
  return myself;
}

/**
 * [File] delete
 *
 * Deletes the file.
 */
ctr_object *ctr_file_delete(ctr_object *myself, ctr_argument *argumentList) {
  ctr_check_permission(CTR_SECPRO_NO_FILE_WRITE);
  if (myself->value.rvalue && myself->value.rvalue->ptr) {
    CtrStdFlow = ctr_build_string_from_cstring("Resource is open");
    return ctr_build_nil();
  }
  ctr_object *path = ctr_file_rpath(myself, NULL);
  ctr_size vlen;
  char *pathString;
  int r;
  if (path == NULL)
    return myself;
  vlen = path->value.svalue->vlen;
  pathString = ctr_heap_allocate(sizeof(char) * (vlen + 1));
  memcpy(pathString, path->value.svalue->value, vlen);
  memcpy(pathString + vlen, "\0", 1);
  r = remove(pathString);
  ctr_heap_free(pathString);
  if (r != 0) {
    CtrStdFlow = ctr_build_string_from_cstring("Unable to delete file.");
    CtrStdFlow->info.sticky = 1;
    return CtrStdNil;
  }
  return myself;
}

/**
 * [File] size
 *
 * Returns the size of the file.
 */
ctr_object *ctr_file_size(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *path = ctr_file_rpath(myself, NULL);
  ctr_size vlen;
  char *pathString;
  FILE *f;
  if (!myself->value.rvalue || !myself->value.rvalue->ptr) {
    int prev, sz;
    if (path == NULL)
      return ctr_build_number_from_float(0);
    vlen = path->value.svalue->vlen;
    pathString = ctr_heap_allocate(sizeof(char) * (vlen + 1));
    memcpy(pathString, path->value.svalue->value, (sizeof(char) * vlen));
    memcpy(pathString + vlen, "\0", 1);
    f = fopen(pathString, "r");
    ctr_heap_free(pathString);
    if (f == NULL)
      return ctr_build_number_from_float(0);
    prev = ftell(f);
    fseek(f, 0L, SEEK_END);
    sz = ftell(f);
    fseek(f, prev, SEEK_SET);
    if (f) {
      fclose(f);
    }
    return ctr_build_number_from_float((ctr_number)sz);
  } else if (myself->value.rvalue->ptr == stdin) {
    return ctr_build_number_from_float(
        !!feof(stdin)); // will return 1 if data exists, 0 otherwise
  } else
    return ctr_build_number_from_float(0);
}

/**
 * [File] open: [string]
 *
 * Open a file with using the specified mode.
 *
 * Usage:
 *
 * f := File new: '/path/to/file'.
 * f open: 'r+'. #opens file for reading and writing
 *
 * The example above opens the file in f for reading and writing.
 */
ctr_object *ctr_file_open(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *pathObj = ctr_file_rpath(myself, NULL);
  char *mode;
  char *path;
  FILE *handle;
  ctr_resource *rs = ctr_heap_allocate(sizeof(ctr_resource));
  ctr_object *modeStrObj = ctr_internal_cast2string(argumentList->object);
  if (myself->value.rvalue != NULL) {
    ctr_heap_free(rs);
    CtrStdFlow = ctr_build_string_from_cstring("File has already been opened.");
    CtrStdFlow->info.sticky = 1;
    return myself;
  }
  if (pathObj == NULL)
    return myself;
  path = ctr_heap_allocate_cstring(pathObj);
  mode = ctr_heap_allocate_cstring(modeStrObj);
  handle = fopen(path, mode);
  ctr_heap_free(path);
  ctr_heap_free(mode);
  rs->type = 1;
  rs->ptr = handle;
  myself->value.rvalue = rs;
  return myself;
}

/**
 * [File] close.
 *
 * Closes the file represented by the recipient.
 *
 * Returns the exit code if the file is created by Shell::'open:mode:'
 * Usage:
 *
 * f := File new: '/path/to/file.txt'.
 * f open: 'r+'.
 * f close.
 *
 * The example above opens and closes a file.
 */
ctr_object *ctr_file_close(ctr_object *myself, ctr_argument *argumentList) {
  if (myself->value.rvalue == NULL)
    return myself;
  if (myself->value.rvalue->type == 2) {
    int res = pclose((FILE *)myself->value.rvalue->ptr);
    myself->value.rvalue = NULL;
    return ctr_build_number_from_float(res);
  }
  if (myself->value.rvalue->type != 1)
    return myself;
  if (myself->value.rvalue->ptr) {
    fclose((FILE *)myself->value.rvalue->ptr);
  }
  ctr_heap_free(myself->value.rvalue);
  myself->value.rvalue = NULL;
  return myself;
}

/**
 * [File] flush.
 *
 * Closes the file represented by the recipient.
 *
 * Usage:
 *
 * f := File new: '/path/to/file.txt'.
 * f open: 'r+'.
 * f close.
 *
 * The example above opens and closes a file.
 */
ctr_object *ctr_file_flush(ctr_object *myself, ctr_argument *argumentList) {
  if (myself->value.rvalue == NULL)
    return myself;
  if (myself->value.rvalue->ptr) {
    fflush((FILE *)myself->value.rvalue->ptr);
  }
  return myself;
}

/**
 * [File] readBytes: [Number].
 *
 * Reads a number of bytes from the file.
 *
 * Usage:
 *
 * f := File new: '/path/to/file.txt'.
 * f open: 'r+'.
 * x := f readBytes: 10.
 * f close.
 *
 * The example above reads 10 bytes from the file represented by f
 * and puts them in buffer x.
 */
ctr_object *ctr_file_read_bytes(ctr_object *myself,
                                ctr_argument *argumentList) {
  int bytes;
  char *buffer;
  ctr_object *result;
  if (myself->value.rvalue == NULL)
    return myself;
  if (myself->value.rvalue->type != 1)
    return myself;
  bytes = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  if (bytes < 0)
    return ctr_build_string_from_cstring("");
  buffer = (char *)ctr_heap_allocate(bytes);
  if (buffer == NULL) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "Cannot allocate memory for file buffer.");
    CtrStdFlow->info.sticky = 1;
    return ctr_build_string_from_cstring("");
  }
  if (myself->value.rvalue->ptr == stdin) {
    for (int i = 0; i < bytes; i++) {
#ifdef withTermios
      tcgetattr(STDIN_FILENO, &oldTermios);
      newTermios = oldTermios;
      cfmakeraw(&newTermios);
      tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);
#endif
      while ((buffer[i] = fgetc(stdin)) == EOF)
        ;
#ifdef withTermios
      tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
#endif
    }
  } else
    bytes = fread(buffer, sizeof(char), (int)bytes,
                  (FILE *)myself->value.rvalue->ptr);
  result = ctr_build_string(buffer, bytes);
  ctr_heap_free(buffer);
  return result;
}

/**
 * [File] writeBytes: [String].
 *
 * Takes a string and writes the bytes in the string to the file
 * object. Returns the number of bytes actually written.
 *
 * Usage:
 *
 * f := File new: '/path/to/file.txt'.
 * f open: 'r+'.
 * n := f writeBytes: 'Hello World'.
 * f close.
 *
 * The example above writes 'Hello World' to the specified file as bytes.
 * The number of bytes written is returned in variable n.
 */
ctr_object *ctr_file_write_bytes(ctr_object *myself,
                                 ctr_argument *argumentList) {
  ctr_check_permission(CTR_SECPRO_NO_FILE_WRITE);
  int bytes, written;
  ctr_object *string2write;
  char *buffer;
  if (myself->value.rvalue == NULL)
    return myself;
  if (myself->value.rvalue->type != 1)
    return myself;
  string2write = ctr_internal_cast2string(argumentList->object);
  buffer = ctr_heap_allocate_cstring(string2write);
  bytes = string2write->value.svalue->vlen;
  written = fwrite(buffer, sizeof(char), (int)bytes,
                   (FILE *)myself->value.rvalue->ptr);
  ctr_heap_free(buffer);
  return ctr_build_number_from_float((double_t)written);
}

/**
 * [File] seek: [Number].
 *
 * Moves the file pointer to the specified position in the file
 * (relative to the current position).
 *
 * Usage:
 *
 * file open: 'r', seek: 10.
 *
 * The example above opens a file for reading and moves the
 * pointer to position 10 (meaning 10 bytes from the beginning of the file).
 * The seek value may be negative.
 */
ctr_object *ctr_file_seek(ctr_object *myself, ctr_argument *argumentList) {
  int offset;
  int error;
  ctr_check_permission(CTR_SECPRO_NO_FILE_READ);
  if (myself->value.rvalue == NULL)
    return myself;
  if (myself->value.rvalue->type != 1)
    return myself;
  offset =
      (long int)ctr_internal_cast2number(argumentList->object)->value.nvalue;
  error = fseek((FILE *)myself->value.rvalue->ptr, offset, SEEK_CUR);
  if (error) {
    CtrStdFlow = ctr_build_string_from_cstring("Seek failed.");
    CtrStdFlow->info.sticky = 1;
  }
  return myself;
}

/**
 * [File] rewind.
 *
 * Rewinds the file. Moves the file pointer to the beginning of the file.
 *
 * Usage:
 *
 * file open: 'r'.
 * x := file readBytes: 10. #read 10 bytes
 * file rewind.        #rewind, set pointer to begin again
 * y := file readBytes: 10. #re-read same 10 bytes
 *
 * The example above reads the same sequence of 10 bytes twice, resulting
 * in variable x and y being equal.
 */
ctr_object *ctr_file_seek_rewind(ctr_object *myself,
                                 ctr_argument *argumentList) {
  int error;
  ctr_check_permission(CTR_SECPRO_NO_FILE_READ);
  if (myself->value.rvalue == NULL)
    return myself;
  if (myself->value.rvalue->type != 1)
    return myself;
  error = fseek((FILE *)myself->value.rvalue->ptr, 0, SEEK_SET);
  if (error) {
    CtrStdFlow = ctr_build_string_from_cstring("Seek rewind failed.");
    CtrStdFlow->info.sticky = 1;
  }
  return myself;
}

/**
 * [File] end.
 *
 * Moves the file pointer to the end of the file. Use this in combination with
 * negative seek operations.
 *
 * Usage:
 *
 * file open: 'r'.
 * file end.
 * x := file seek: -10, readBytes: 10.
 *
 * The example above will read the last 10 bytes of the file. This is
 * accomplished by first moving the file pointer to the end of the file,
 * then putting it back 10 bytes (negative number), and then reading 10
 * bytes.
 */
ctr_object *ctr_file_seek_end(ctr_object *myself, ctr_argument *argumentList) {
  int error;
  ctr_check_permission(CTR_SECPRO_NO_FILE_READ);
  if (myself->value.rvalue == NULL)
    return myself;
  if (myself->value.rvalue->type != 1)
    return myself;
  error = fseek((FILE *)myself->value.rvalue->ptr, 0, SEEK_END);
  if (error) {
    CtrStdFlow = ctr_build_string_from_cstring("Seek end failed.");
    CtrStdFlow->info.sticky = 1;
  }
  return myself;
}

ctr_object *ctr_file_lock_generic(ctr_object *myself,
                                  ctr_argument *argumentList, int lock) {
  int b;
  int fd;
  char *path;
  ctr_object *pathObj;
  ctr_object *answer;
  ctr_object *fdObj;
  ctr_object *fdObjKey;
  ctr_check_permission(CTR_SECPRO_NO_FILE_WRITE);
  pathObj = ctr_internal_object_find_property(
      myself, ctr_build_string_from_cstring("path"), 0);
  path = ctr_heap_allocate_cstring(pathObj);
  fdObjKey = ctr_build_string_from_cstring("fileDescriptor");
  fdObj = ctr_internal_object_find_property(myself, fdObjKey,
                                            CTR_CATEGORY_PRIVATE_PROPERTY);
  if (fdObj == NULL) {
    fd = open(path, O_CREAT, "w+");
    fdObj = ctr_build_number_from_float((ctr_size)fd);
    ctr_internal_object_set_property(myself, fdObjKey, fdObj,
                                     CTR_CATEGORY_PRIVATE_PROPERTY);
  } else {
    fd = (int)fdObj->value.nvalue;
  }
  b = flock(fd, lock);
  if (b != 0) {
    close(fd);
    answer = ctr_build_bool(0);
    ctr_internal_object_delete_property(myself, fdObjKey,
                                        CTR_CATEGORY_PRIVATE_PROPERTY);
  } else {
    answer = ctr_build_bool(1);
  }
  ctr_heap_free(path);
  return answer;
}

ctr_object *ctr_file_unlock(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_file_lock_generic(myself, argumentList, LOCK_UN | LOCK_NB);
}

ctr_object *ctr_file_lock(ctr_object *myself, ctr_argument *argumentList) {
  return ctr_file_lock_generic(myself, argumentList, LOCK_EX | LOCK_NB);
}

ctr_object *ctr_file_list(ctr_object *myself, ctr_argument *argumentList) {
  DIR *d;
  struct dirent *entry;
  char *pathValue;
  ctr_object *fileList;
  ctr_object *fileListItem;
  ctr_object *path;
  ctr_argument *putArgumentList;
  ctr_argument *addArgumentList;
  ctr_check_permission(CTR_SECPRO_NO_FILE_READ);
  path = ctr_internal_cast2string(argumentList->object);
  fileList = ctr_array_new(CtrStdArray, NULL);
  pathValue = ctr_heap_allocate_cstring(path);
  d = opendir(pathValue);
  if (d == 0) {
    CtrStdFlow = ctr_build_string_from_cstring("Failed to open folder.");
    ctr_heap_free(pathValue);
    return CtrStdNil;
  }
  putArgumentList = ctr_heap_allocate(sizeof(ctr_argument));
  addArgumentList = ctr_heap_allocate(sizeof(ctr_argument));
  putArgumentList->next = ctr_heap_allocate(sizeof(ctr_argument));
  while ((entry = readdir(d))) {
    fileListItem = ctr_map_new(CtrStdMap, NULL);
    putArgumentList->next->object = ctr_build_string_from_cstring("file");
    putArgumentList->object = ctr_build_string_from_cstring(entry->d_name);
    ctr_map_put(fileListItem, putArgumentList);
    putArgumentList->next->object = ctr_build_string_from_cstring("type");
    switch (entry->d_type) {
    case DT_REG:
      putArgumentList->object = ctr_build_string_from_cstring("file");
      break;
    case DT_DIR:
      putArgumentList->object = ctr_build_string_from_cstring("folder");
      break;
    case DT_LNK:
      putArgumentList->object = ctr_build_string_from_cstring("symbolic link");
      break;
    case DT_CHR:
      putArgumentList->object =
          ctr_build_string_from_cstring("character device");
      break;
    case DT_BLK:
      putArgumentList->object = ctr_build_string_from_cstring("block device");
      break;
    case DT_SOCK:
      putArgumentList->object = ctr_build_string_from_cstring("socket");
      break;
    case DT_FIFO:
      putArgumentList->object = ctr_build_string_from_cstring("named pipe");
      break;
    default:
      putArgumentList->object = ctr_build_string_from_cstring("other");
      break;
    }
    ctr_map_put(fileListItem, putArgumentList);
    addArgumentList->object = fileListItem;
    ctr_array_push(fileList, addArgumentList);
  }
  closedir(d);
  ctr_heap_free(putArgumentList->next);
  ctr_heap_free(putArgumentList);
  ctr_heap_free(addArgumentList);
  ctr_heap_free(pathValue);
  return fileList;
}

ctr_object *ctr_file_to_string(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *pathObj = ctr_internal_object_find_property(
      myself, ctr_build_string_from_cstring("path"), 0);
  if (!pathObj)
    return ctr_build_string_from_cstring("File");
  ctr_size len = pathObj->value.svalue->vlen;
  char *v = pathObj->value.svalue->value;
  char *nv = ctr_heap_allocate(sizeof(char) * (len + 20));
  len = sprintf(nv, "[File:%.*s]", len, v);
  pathObj = ctr_build_string(nv, len);
  ctr_heap_free(nv);
  return pathObj;
}

ctr_object *ctr_file_type(ctr_object *myself, ctr_argument *argumentList) {
  char *path = ctr_heap_allocate_cstring(argumentList->object);
  struct stat stats;
  char *value;
  if (lstat(path, &stats) == 0) {
    switch (stats.st_mode & S_IFMT) {
    case S_IFSOCK:
      value = "socket";
      goto ret;
    case S_IFLNK:
      value = "symbolic link";
      goto ret;
    case S_IFREG:
      value = "file";
      goto ret;
    case S_IFBLK:
      value = "block device";
      goto ret;
    case S_IFDIR:
      value = "folder";
      goto ret;
    case S_IFCHR:
      value = "character device";
      goto ret;
    case S_IFIFO:
      value = "named pipe";
      goto ret;
    default:
      value = "other";
      goto ret;
    }
  }
  ctr_heap_free(path);
  return CtrStdNil;
ret:
  ctr_heap_free(path);
  return ctr_build_string_from_cstring(value);
}

/**
 * [File] mkdir [: [Number:permissions]]
 *
 * makes a directory
 */
ctr_object *ctr_file_mkdir(ctr_object *myself, ctr_argument *argumentList) {
  ctr_check_permission(CTR_SECPRO_NO_FILE_WRITE);
  ctr_object *pathobj = ctr_file_rpath(myself, NULL);
  if (pathobj == CtrStdNil) {
    CtrStdFlow = ctr_build_string_from_cstring("file object contains no path");
    return CtrStdNil;
  }
  char *path = ctr_heap_allocate_cstring(pathobj);
  mode_t mode = 0755;
  if (argumentList->object &&
      argumentList->object->interfaces->link == CtrStdNumber)
    mode = argumentList->object->value.nvalue;
  if (mkdir(path, mode) != 0) {
    CtrStdFlow = ctr_build_string_from_cstring(strerror(errno));
    ctr_heap_free(path);
    return CtrStdNil;
  }
  ctr_heap_free(path);
  return myself;
}

ctr_object *ctr_generate_timespec(long ts) {
  return ctr_build_number_from_float(ts);
}

/**
 * [File] stat
 *
 * returns the entire dataset of stat(2) about the given file
 *
 */
ctr_object *ctr_file_stat(ctr_object *myself, ctr_argument *argumentList) {
  ctr_check_permission(CTR_SECPRO_NO_FILE_READ);
  ctr_object *pathobj = ctr_file_rpath(myself, NULL);
  if (pathobj == CtrStdNil) {
    CtrStdFlow = ctr_build_string_from_cstring("file object contains no path");
    return CtrStdNil;
  }
  char *path = ctr_heap_allocate_cstring(pathobj);
  struct stat st;
  if (stat(path, &st) == -1) {
    CtrStdFlow =
        ctr_format_str("Estat() error %s (path = %s)", strerror(errno), path);
    ctr_heap_free(path);
    return CtrStdNil;
  }
  ctr_object *res = ctr_map_new(CtrStdMap, NULL);

  ctr_map_put(res, &(ctr_argument){
                       .object = ctr_build_number_from_float((long)st.st_dev),
                       .next = &(ctr_argument){
                           .object = ctr_build_string_from_cstring("st_dev"),
                           .next = NULL}});
  ctr_map_put(res, &(ctr_argument){
                       .object = ctr_build_number_from_float((long)st.st_ino),
                       .next = &(ctr_argument){
                           .object = ctr_build_string_from_cstring("st_ino"),
                           .next = NULL}});
  ctr_map_put(res, &(ctr_argument){
                       .object = ctr_build_number_from_float((long)st.st_mode),
                       .next = &(ctr_argument){
                           .object = ctr_build_string_from_cstring("st_mode"),
                           .next = NULL}});
  ctr_map_put(res, &(ctr_argument){
                       .object = ctr_build_number_from_float((long)st.st_nlink),
                       .next = &(ctr_argument){
                           .object = ctr_build_string_from_cstring("st_nlink"),
                           .next = NULL}});
  ctr_map_put(res, &(ctr_argument){
                       .object = ctr_build_number_from_float((long)st.st_uid),
                       .next = &(ctr_argument){
                           .object = ctr_build_string_from_cstring("st_uid"),
                           .next = NULL}});
  ctr_map_put(res, &(ctr_argument){
                       .object = ctr_build_number_from_float((long)st.st_gid),
                       .next = &(ctr_argument){
                           .object = ctr_build_string_from_cstring("st_gid"),
                           .next = NULL}});
  ctr_map_put(res, &(ctr_argument){
                       .object = ctr_build_number_from_float((long)st.st_rdev),
                       .next = &(ctr_argument){
                           .object = ctr_build_string_from_cstring("st_rdev"),
                           .next = NULL}});
  ctr_map_put(res, &(ctr_argument){
                       .object = ctr_build_number_from_float((long)st.st_size),
                       .next = &(ctr_argument){
                           .object = ctr_build_string_from_cstring("st_size"),
                           .next = NULL}});
  ctr_map_put(res,
              &(ctr_argument){
                  .object = ctr_build_number_from_float((long)st.st_blksize),
                  .next = &(ctr_argument){
                      .object = ctr_build_string_from_cstring("st_blksize"),
                      .next = NULL}});
  ctr_map_put(
      res,
      &(ctr_argument){.object = ctr_build_number_from_float((long)st.st_blocks),
                      .next = &(ctr_argument){
                          .object = ctr_build_string_from_cstring("st_blocks"),
                          .next = NULL}});

#if _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700
  ctr_map_put(res, &(ctr_argument){
                       .object = ctr_generate_timespec(st.st_atime),
                       .next = &(ctr_argument){
                           .object = ctr_build_string_from_cstring("st_atime"),
                           .next = NULL}});
  ctr_map_put(res, &(ctr_argument){
                       .object = ctr_generate_timespec(st.st_ctime),
                       .next = &(ctr_argument){
                           .object = ctr_build_string_from_cstring("st_ctime"),
                           .next = NULL}});
  ctr_map_put(res, &(ctr_argument){
                       .object = ctr_generate_timespec(st.st_mtime),
                       .next = &(ctr_argument){
                           .object = ctr_build_string_from_cstring("st_mtime"),
                           .next = NULL}});
#endif

  ctr_heap_free(path);
  return res;
}
