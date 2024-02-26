/***************************************************************************
Fuzzer driver for PCRE2. Given an arbitrary string of bytes and a length, it
tries to compile and match it, deriving options from the string itself. If
STANDALONE is defined, a main program that calls the driver with the contents
of specified files is compiled, and commentary on what is happening is output.
If an argument starts with '=' the rest of it it is taken as a literal string
rather than a file name. This allows easy testing of short strings.

Written by Philip Hazel, October 2016
Updated February 2024 (Addison Crump added 16-bit/32-bit and JIT support)
***************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* stack size adjustment */
#include <sys/time.h>
#include <sys/resource.h>

#define STACK_SIZE_MB 256

#ifndef PCRE2_CODE_UNIT_WIDTH
#define PCRE2_CODE_UNIT_WIDTH 8
#endif

#include "config.h"
#include "pcre2.h"

#define MAX_MATCH_SIZE 1000

#define DFA_WORKSPACE_COUNT 100

#define ALLOWED_COMPILE_OPTIONS \
  (PCRE2_ANCHORED|PCRE2_ALLOW_EMPTY_CLASS|PCRE2_ALT_BSUX|PCRE2_ALT_CIRCUMFLEX| \
   PCRE2_ALT_VERBNAMES|PCRE2_AUTO_CALLOUT|PCRE2_CASELESS|PCRE2_DOLLAR_ENDONLY| \
   PCRE2_DOTALL|PCRE2_DUPNAMES|PCRE2_ENDANCHORED|PCRE2_EXTENDED|PCRE2_FIRSTLINE| \
   PCRE2_MATCH_UNSET_BACKREF|PCRE2_MULTILINE|PCRE2_NEVER_BACKSLASH_C| \
   PCRE2_NO_AUTO_CAPTURE| \
   PCRE2_NO_AUTO_POSSESS|PCRE2_NO_DOTSTAR_ANCHOR|PCRE2_NO_START_OPTIMIZE| \
   PCRE2_UCP|PCRE2_UNGREEDY|PCRE2_USE_OFFSET_LIMIT| \
   PCRE2_UTF)

#define ALLOWED_MATCH_OPTIONS \
  (PCRE2_ANCHORED|PCRE2_ENDANCHORED|PCRE2_NOTBOL|PCRE2_NOTEOL|PCRE2_NOTEMPTY| \
   PCRE2_NOTEMPTY_ATSTART|PCRE2_PARTIAL_HARD| \
   PCRE2_PARTIAL_SOFT)

#if defined(SUPPORT_DIFF_FUZZ) || defined(STANDALONE)
static void print_compile_options(FILE *stream, uint32_t compile_options)
{
fprintf(stream, "Compile options %.8x never_backslash_c", compile_options);
fprintf(stream, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
  ((compile_options & PCRE2_ALT_BSUX) != 0)? ",alt_bsux" : "",
  ((compile_options & PCRE2_ALT_CIRCUMFLEX) != 0)? ",alt_circumflex" : "",
  ((compile_options & PCRE2_ALT_VERBNAMES) != 0)? ",alt_verbnames" : "",
  ((compile_options & PCRE2_ALLOW_EMPTY_CLASS) != 0)? ",allow_empty_class" : "",
  ((compile_options & PCRE2_ANCHORED) != 0)? ",anchored" : "",
  ((compile_options & PCRE2_AUTO_CALLOUT) != 0)? ",auto_callout" : "",
  ((compile_options & PCRE2_CASELESS) != 0)? ",caseless" : "",
  ((compile_options & PCRE2_DOLLAR_ENDONLY) != 0)? ",dollar_endonly" : "",
  ((compile_options & PCRE2_DOTALL) != 0)? ",dotall" : "",
  ((compile_options & PCRE2_DUPNAMES) != 0)? ",dupnames" : "",
  ((compile_options & PCRE2_ENDANCHORED) != 0)? ",endanchored" : "",
  ((compile_options & PCRE2_EXTENDED) != 0)? ",extended" : "",
  ((compile_options & PCRE2_FIRSTLINE) != 0)? ",firstline" : "",
  ((compile_options & PCRE2_MATCH_UNSET_BACKREF) != 0)? ",match_unset_backref" : "",
  ((compile_options & PCRE2_MULTILINE) != 0)? ",multiline" : "",
  ((compile_options & PCRE2_NEVER_UCP) != 0)? ",never_ucp" : "",
  ((compile_options & PCRE2_NEVER_UTF) != 0)? ",never_utf" : "",
  ((compile_options & PCRE2_NO_AUTO_CAPTURE) != 0)? ",no_auto_capture" : "",
  ((compile_options & PCRE2_NO_AUTO_POSSESS) != 0)? ",no_auto_possess" : "",
  ((compile_options & PCRE2_NO_DOTSTAR_ANCHOR) != 0)? ",no_dotstar_anchor" : "",
  ((compile_options & PCRE2_NO_UTF_CHECK) != 0)? ",no_utf_check" : "",
  ((compile_options & PCRE2_NO_START_OPTIMIZE) != 0)? ",no_start_optimize" : "",
  ((compile_options & PCRE2_UCP) != 0)? ",ucp" : "",
  ((compile_options & PCRE2_UNGREEDY) != 0)? ",ungreedy" : "",
  ((compile_options & PCRE2_USE_OFFSET_LIMIT) != 0)? ",use_offset_limit" : "",
  ((compile_options & PCRE2_UTF) != 0)? ",utf" : "");
}

static void print_match_options(FILE *stream, uint32_t match_options)
{
fprintf(stream, "Match options %.8x", match_options);
fprintf(stream, "%s%s%s%s%s%s%s%s%s\n",
  ((match_options & PCRE2_ANCHORED) != 0)? ",anchored" : "",
  ((match_options & PCRE2_ENDANCHORED) != 0)? ",endanchored" : "",
  ((match_options & PCRE2_NO_UTF_CHECK) != 0)? ",no_utf_check" : "",
  ((match_options & PCRE2_NOTBOL) != 0)? ",notbol" : "",
  ((match_options & PCRE2_NOTEMPTY) != 0)? ",notempty" : "",
  ((match_options & PCRE2_NOTEMPTY_ATSTART) != 0)? ",notempty_atstart" : "",
  ((match_options & PCRE2_NOTEOL) != 0)? ",noteol" : "",
  ((match_options & PCRE2_PARTIAL_HARD) != 0)? ",partial_hard" : "",
  ((match_options & PCRE2_PARTIAL_SOFT) != 0)? ",partial_soft" : "");
}
#endif /* defined(SUPPORT_DIFF_FUZZ || defined(STANDALONE) */

#ifdef SUPPORT_JIT
#ifdef SUPPORT_DIFF_FUZZ
static void dump_matches(FILE *stream, int count, pcre2_match_data *match_data)
{
#if PCRE2_CODE_UNIT_WIDTH == 8
PCRE2_UCHAR error_buf[256];
#endif
int errorcode;

for (int index = 0; index < count; index++)
  {
  PCRE2_UCHAR *bufferptr = NULL;
  PCRE2_SIZE bufflen = 0;

  errorcode = pcre2_substring_get_bynumber(match_data, index, &bufferptr, &bufflen);

  if (errorcode >= 0)
    {
    fprintf(stream, "Match %d (hex encoded): ", index);
    for (PCRE2_SIZE i = 0; i < bufflen; i++)
      {
      fprintf(stream, "%02x", bufferptr[i]);
      }
    fprintf(stream, "\n");
    }
  else
    {
#if PCRE2_CODE_UNIT_WIDTH == 8
    pcre2_get_error_message(errorcode, error_buf, 256);
    fprintf(stream, "Match %d failed: %s\n", index, error_buf);
#else
    fprintf(stream, "Match %d failed: %d\n", index, errorcode);
#endif
    }
  }
}

/* This function describes the current test case being evaluated, then aborts */

static void describe_failure(
  const char *task,
  const unsigned char *data,
  size_t size,
  uint32_t compile_options,
  uint32_t match_options,
  int errorcode,
  int errorcode_jit,
  int matches,
  int matches_jit,
  pcre2_match_data *match_data,
  pcre2_match_data *match_data_jit
) {
#if PCRE2_CODE_UNIT_WIDTH == 8
PCRE2_UCHAR buffer[256];
#endif

fprintf(stderr, "Encountered failure while performing %s; context:\n", task);

fprintf(stderr, "Pattern/sample string (hex encoded): ");
for (size_t i = 0; i < size; i++)
  {
  fprintf(stderr, "%02x", data[i]);
  }
fprintf(stderr, "\n");

print_compile_options(stderr, compile_options);
print_match_options(stderr, match_options);

if (errorcode < 0)
  {
#if PCRE2_CODE_UNIT_WIDTH == 8
  pcre2_get_error_message(errorcode, buffer, 256);
  fprintf(stderr, "Non-JIT'd operation emitted an error: %s (%d)\n", buffer, errorcode);
#else
  fprintf(stderr, "Non-JIT'd operation emitted an error: %d\n", errorcode);
#endif
  }
if (matches >= 0)
  {
  fprintf(stderr, "Non-JIT'd operation did not emit an error.\n");
  if (match_data != NULL)
    {
    fprintf(stderr, "%d matches discovered by non-JIT'd regex:\n", matches);
    dump_matches(stderr, matches, match_data);
    fprintf(stderr, "\n");
    }
  }

if (errorcode_jit < 0)
  {
#if PCRE2_CODE_UNIT_WIDTH == 8
  pcre2_get_error_message(errorcode_jit, buffer, 256);
  fprintf(stderr, "JIT'd operation emitted an error: %s (%d)\n", buffer, errorcode_jit);
#else
  fprintf(stderr, "JIT'd operation emitted an error: %d\n", errorcode);
#endif
  }
if (matches_jit >= 0)
  {
  fprintf(stderr, "JIT'd operation did not emit an error.\n");
  if (match_data_jit != NULL)
    {
    fprintf(stderr, "%d matches discovered by JIT'd regex:\n", matches_jit);
    dump_matches(stderr, matches_jit, match_data_jit);
    fprintf(stderr, "\n");
    }
  }

abort();
}
#endif  /* SUPPORRT_DIFF_FUZZ */
#endif  /* SUPPORT_JIT */

/* This is the callout function. Its only purpose is to halt matching if there
are more than 100 callouts, as one way of stopping too much time being spent on
fruitless matches. The callout data is a pointer to the counter. */

static int callout_function(pcre2_callout_block *cb, void *callout_data)
{
(void)cb;  /* Avoid unused parameter warning */
*((uint32_t *)callout_data) += 1;
return (*((uint32_t *)callout_data) > 100)? PCRE2_ERROR_CALLOUT : 0;
}

/* Putting in this apparently unnecessary prototype prevents gcc from giving a
"no previous prototype" warning when compiling at high warning level. */

int LLVMFuzzerInitialize(int *, char ***);

int LLVMFuzzerTestOneInput(const unsigned char *, size_t);

int LLVMFuzzerInitialize(int *argc, char ***argv)
{
int rc;
struct rlimit rlim;
getrlimit(RLIMIT_STACK, &rlim);
rlim.rlim_cur = STACK_SIZE_MB * 1024 * 1024;
if (rlim.rlim_cur > rlim.rlim_max)
  {
  fprintf(stderr, "hard stack size limit is too small (needed 8MiB)!\n");
  _exit(1);
  }
rc = setrlimit(RLIMIT_STACK, &rlim);
if (rc != 0)
  {
  fprintf(stderr, "failed to expand stack size\n");
  _exit(1);
  }

(void)argc;  /* Avoid "unused parameter" warnings */
(void)argv;
return 0;
}

/* Here's the driving function. */

int LLVMFuzzerTestOneInput(const unsigned char *data, size_t size)
{
uint32_t compile_options;
uint32_t match_options;
uint64_t random_options;
pcre2_match_data *match_data = NULL;
#ifdef SUPPORT_JIT
pcre2_match_data *match_data_jit = NULL;
#endif
pcre2_match_context *match_context = NULL;
size_t match_size;
int dfa_workspace[DFA_WORKSPACE_COUNT];
int i;

if (size < sizeof(random_options)) return -1;

/* Limiting the length of the subject for matching stops fruitless searches
in large trees taking too much time. */

random_options = *(uint64_t *)(data);
data += sizeof(random_options);
size -= sizeof(random_options);
size /= PCRE2_CODE_UNIT_WIDTH / 8;

match_size = (size > MAX_MATCH_SIZE)? MAX_MATCH_SIZE : size;

/* Ensure that all undefined option bits are zero (waste of time trying them)
and also that PCRE2_NO_UTF_CHECK is unset, as there is no guarantee that the
input is UTF-8. Also unset PCRE2_NEVER_UTF and PCRE2_NEVER_UCP as there is no
reason to disallow UTF and UCP. Force PCRE2_NEVER_BACKSLASH_C to be set because
\C in random patterns is highly likely to cause a crash. */

compile_options = ((random_options >> 32) & ALLOWED_COMPILE_OPTIONS) |
  PCRE2_NEVER_BACKSLASH_C;
match_options = (((uint32_t)random_options) & ALLOWED_MATCH_OPTIONS) |
  PCRE2_NO_JIT |
  PCRE2_DISABLE_RECURSELOOP_CHECK;

/* Discard partial matching if PCRE2_ENDANCHORED is set, because they are not
allowed together and just give an immediate error return. */

if (((compile_options|match_options) & PCRE2_ENDANCHORED) != 0)
  match_options &= ~(PCRE2_PARTIAL_HARD|PCRE2_PARTIAL_SOFT);

/* Do the compile with and without the options, and after a successful compile,
likewise do the match with and without the options. */

for (i = 0; i < 2; i++)
  {
  uint32_t callout_count;
  int errorcode;
#ifdef SUPPORT_JIT
  int errorcode_jit;
#ifdef SUPPORT_JIT_FUZZ
  int matches = 0;
  int matches_jit = 0;
#endif
#endif
  PCRE2_SIZE erroroffset;
  pcre2_code *code;

#ifdef STANDALONE
  print_compile_options(stdout, compile_options);
#endif

  code = pcre2_compile((PCRE2_SPTR)data, (PCRE2_SIZE)size, compile_options,
    &errorcode, &erroroffset, NULL);

  /* Compilation succeeded */

  if (code != NULL)
    {
    int j;
    uint32_t save_match_options = match_options;

#ifdef SUPPORT_JIT
    int jit_ret = pcre2_jit_compile(code, PCRE2_JIT_COMPLETE);
#endif

    /* Create match data and context blocks only when we first need them. Set
    low match and depth limits to avoid wasting too much searching large
    pattern trees. Almost all matches are going to fail. */

    if (match_data == NULL)
      {
      match_data = pcre2_match_data_create(32, NULL);
#ifdef SUPPORT_JIT
      match_data_jit = pcre2_match_data_create(32, NULL);
      if (match_data == NULL || match_data_jit == NULL)
#else
      if (match_data == NULL)
#endif
        {
#ifdef STANDALONE
        fprintf(stderr, "** Failed to create match data block\n");
#endif
        abort();
        }
      }

    if (match_context == NULL)
      {
      match_context = pcre2_match_context_create(NULL);
      if (match_context == NULL)
        {
#ifdef STANDALONE
        fprintf(stderr, "** Failed to create match context block\n");
#endif
        abort();
        }
      (void)pcre2_set_match_limit(match_context, 100);
      (void)pcre2_set_depth_limit(match_context, 100);
      (void)pcre2_set_callout(match_context, callout_function, &callout_count);
      }

    /* Match twice, with and without options. */

    for (j = 0; j < 2; j++)
      {
#ifdef STANDALONE
      print_match_options(stdout, match_options);
#endif

      callout_count = 0;
      errorcode = pcre2_match(code, (PCRE2_SPTR)data, (PCRE2_SIZE)match_size, 0,
        match_options, match_data, match_context);

#ifdef STANDALONE
      if (errorcode >= 0) printf("Match returned %d\n", errorcode); else
        {
#if PCRE2_CODE_UNIT_WIDTH == 8
        unsigned char buffer[256];
        pcre2_get_error_message(errorcode, buffer, 256);
        printf("Match failed: error %d: %s\n", errorcode, buffer);
#else
        printf("Match failed: error %d\n", errorcode);
#endif
        }
#endif

#ifdef SUPPORT_JIT
      if (jit_ret >= 0)
        {
        callout_count = 0;
        errorcode_jit = pcre2_match(code, (PCRE2_SPTR)data, (PCRE2_SIZE)match_size, 0,
          match_options & ~PCRE2_NO_JIT, match_data_jit, match_context);

#ifndef SUPPORT_DIFF_FUZZ
        (void)errorcode_jit;  /* Avoid compiler warning */
#else

        matches = errorcode;
        matches_jit = errorcode_jit;

        if (errorcode_jit != errorcode)
          {
          if (!(errorcode < 0 && errorcode_jit < 0) &&
                errorcode != PCRE2_ERROR_MATCHLIMIT && errorcode != PCRE2_ERROR_CALLOUT &&
                errorcode_jit != PCRE2_ERROR_MATCHLIMIT && errorcode_jit != PCRE2_ERROR_JIT_STACKLIMIT && errorcode_jit != PCRE2_ERROR_CALLOUT)
            {
            describe_failure("match errorcode comparison", data, size, compile_options, match_options, errorcode, errorcode_jit, matches, matches_jit, match_data, match_data_jit);
            }
          }
        else
          {
          for (int index = 0; index < errorcode; index++)
            {
            PCRE2_UCHAR *bufferptr, *bufferptr_jit;
            PCRE2_SIZE bufflen, bufflen_jit;

            bufferptr = bufferptr_jit = NULL;
            bufflen = bufflen_jit = 0;

            errorcode = pcre2_substring_get_bynumber(match_data, (uint32_t) index, &bufferptr, &bufflen);
            errorcode_jit = pcre2_substring_get_bynumber(match_data_jit, (uint32_t) index, &bufferptr_jit, &bufflen_jit);

            if (errorcode != errorcode_jit)
              {
              describe_failure("match entry errorcode comparison", data, size,
                compile_options, match_options, errorcode, errorcode_jit,
                matches, matches_jit, match_data, match_data_jit);
              }

            if (errorcode >= 0)
              {
              if (bufflen != bufflen_jit)
                {
                describe_failure("match entry length comparison", data, size,
                  compile_options, match_options, errorcode, errorcode_jit,
                  matches, matches_jit, match_data, match_data_jit);
                }

              if (memcmp(bufferptr, bufferptr_jit, bufflen) != 0)
                {
                describe_failure("match entry content comparison", data, size,
                  compile_options, match_options, errorcode, errorcode_jit,
                  matches, matches_jit, match_data, match_data_jit);
                }
              }

              pcre2_substring_free(bufferptr);
              pcre2_substring_free(bufferptr_jit);
            }
          }
#endif  /* SUPPORT_JIT_FUZZ */
        }
#endif  /* SUPPORT_JIT */

      match_options = PCRE2_NO_JIT;  /* For second time */
      }

    /* Match with DFA twice, with and without options. */

    match_options = save_match_options & ~PCRE2_NO_JIT;  /* Not valid for DFA */

    for (j = 0; j < 2; j++)
      {
#ifdef STANDALONE
      printf("DFA match options %.8x", match_options);
      printf("%s%s%s%s%s%s%s%s%s\n",
        ((match_options & PCRE2_ANCHORED) != 0)? ",anchored" : "",
        ((match_options & PCRE2_ENDANCHORED) != 0)? ",endanchored" : "",
        ((match_options & PCRE2_NO_UTF_CHECK) != 0)? ",no_utf_check" : "",
        ((match_options & PCRE2_NOTBOL) != 0)? ",notbol" : "",
        ((match_options & PCRE2_NOTEMPTY) != 0)? ",notempty" : "",
        ((match_options & PCRE2_NOTEMPTY_ATSTART) != 0)? ",notempty_atstart" : "",
        ((match_options & PCRE2_NOTEOL) != 0)? ",noteol" : "",
        ((match_options & PCRE2_PARTIAL_HARD) != 0)? ",partial_hard" : "",
        ((match_options & PCRE2_PARTIAL_SOFT) != 0)? ",partial_soft" : "");
#endif

      callout_count = 0;
      errorcode = pcre2_dfa_match(code, (PCRE2_SPTR)data,
        (PCRE2_SIZE)match_size, 0, match_options, match_data, match_context,
        dfa_workspace, DFA_WORKSPACE_COUNT);

#ifdef STANDALONE
      if (errorcode >= 0) printf("Match returned %d\n", errorcode); else
        {
#if PCRE2_CODE_UNIT_WIDTH == 8
        unsigned char buffer[256];
        pcre2_get_error_message(errorcode, buffer, 256);
        printf("Match failed: error %d: %s\n", errorcode, buffer);
#else
        printf("Match failed: error %d\n", errorcode);
#endif
        }
#endif

      match_options = 0;  /* For second time */
      }

    match_options = save_match_options;  /* Reset for the second compile */
    pcre2_code_free(code);
    }

  /* Compilation failed */

  else
    {
#ifdef STANDALONE
#if PCRE2_CODE_UNIT_WIDTH == 8
    unsigned char buffer[256];
    pcre2_get_error_message(errorcode, buffer, 256);
    printf("Error %d at offset %lu: %s\n", errorcode, erroroffset, buffer);
#else
    printf("Error %d at offset %lu\n", errorcode, erroroffset);
#endif

#else
    if (errorcode == PCRE2_ERROR_INTERNAL) abort();
#endif
    }

  compile_options = PCRE2_NEVER_BACKSLASH_C;  /* For second time */
  }

if (match_data != NULL) pcre2_match_data_free(match_data);
#ifdef SUPPORT_JIT
if (match_data_jit != NULL) pcre2_match_data_free(match_data_jit);
#endif
if (match_context != NULL) pcre2_match_context_free(match_context);

return 0;
}


/* Optional main program.  */

#ifdef STANDALONE
int main(int argc, char **argv)
{
int i;

LLVMFuzzerInitialize(&argc, &argv);

if (argc < 2)
  {
  printf("** No arguments given\n");
  return 0;
  }

for (i = 1; i < argc; i++)
  {
  size_t filelen;
  size_t readsize;
  unsigned char *buffer;
  FILE *f;

  /* Handle a literal string. Copy to an exact size buffer so that checks for
  overrunning work. */

  if (argv[i][0] == '=')
    {
    readsize = strlen(argv[i]) - 1;
    printf("------ <Literal> ------\n");
    printf("Length = %lu\n", readsize);
    printf("%.*s\n", (int)readsize, argv[i]+1);
    buffer = (unsigned char *)malloc(readsize);
    if (buffer == NULL)
      printf("** Failed to allocate %lu bytes of memory\n", readsize);
    else
      {
      memcpy(buffer, argv[i]+1, readsize);
      LLVMFuzzerTestOneInput(buffer, readsize);
      free(buffer);
      }
    continue;
    }

  /* Handle a string given in a file */

  f = fopen(argv[i], "rb");
  if (f == NULL)
    {
    printf("** Failed to open %s: %s\n", argv[i], strerror(errno));
    continue;
    }

  printf("------ %s ------\n", argv[i]);

  fseek(f, 0, SEEK_END);
  filelen = ftell(f);
  fseek(f, 0, SEEK_SET);

  buffer = (unsigned char *)malloc(filelen);
  if (buffer == NULL)
    {
    printf("** Failed to allocate %lu bytes of memory\n", filelen);
    fclose(f);
    continue;
    }

  readsize = fread(buffer, 1, filelen, f);
  fclose(f);

  if (readsize != filelen)
    printf("** File size is %lu but fread() returned %lu\n", filelen, readsize);
  else
    {
    printf("Length = %lu\n", filelen);
    LLVMFuzzerTestOneInput(buffer, filelen);
    }
  free(buffer);
  }

return 0;
}
#endif  /* STANDALONE */

/* End */
