/* Copyright (c) 2025 Kevin B. Hendricks, Stratford Ontario Canada
 *
 * Modified to work on actual exec environments and parse/modify them.
 * Now better detects when external especially  when exec*p* are invoked
 * with no absolute filename.
 *
 * Based on orignal work: Copyright (c) 2018 Pablo Marcos Oltra <pablo.marcos.oltra@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/*
 * This exec.so library is intended to selectively parse the environment
 * inside an AppImage to prevent LD_LIBRARY_PATH and LD_PRELOAD
 * set in the AppImage from impacting external links and programs
 *
 * The intended usage is as follows:
 *
 * 1. This library is injected to the dynamic loader through LD_PRELOAD
 *    and lives in AppRun
 *
 * 2. This library will intercept calls to new processes and will detect whether
 *    those calls are for binaries within the AppImage bundle or external ones.
 *
 * 3. In case it's an internal process, it will not change anything.
 *    In case it's an external process, it will parse the environment
 *    to remove added LD_LIBRARY_PATH and LD_PRELOAD so that external
 *    processes outside the AppImage are not impacted
 *
 * ======
 *
 * Build and install:
 * gcc -O2 [-DDEBUG=1] -shared -fPIC exec.c -o exec_wrapper.so -ldl
 *
 * cp ./exec_wrapper.so squashfs-root/usr/optional/
 *
 * For use with this AppRun
 *
 * Note SIGIL_APPIMAGE_LIB_DIR and SIGIL_EXEC_WRAPPER are required
 *
 * ------------------------
 * #! /usr/bin/env bash
 * # make sure errors in sourced scripts will cause this script to stop
 * set -e
 * 
 * this_dir="$(readlink -f "$(dirname "$0")")"
 * 
 * SIGIL_EXTRA_ROOT="$this_dir/usr/share/sigil"
 * export SIGIL_EXTRA_ROOT
 * echo "${SIGIL_EXTRA_ROOT}"
 *
 * LD_LIBRARY_PATH="$this_dir/usr/lib:$LD_LIBRARY_PATH"
 * export LD_LIBRARY_PATH
 *
 * export SIGIL_APPIMAGE_LIB_DIR="$this_dir/usr/lib"
 *
 * export SIGIL_EXEC_WRAPPER="#this_dir/usr/optional/exec_wrapper.so"
 *
 * source "$this_dir"/apprun-hooks/"linuxdeploy-plugin-qt-hook.sh"
 * 
 * LD_PRELOAD="$this_dir"/usr/optional/exec_wrapper.so exec "$this_dir"/AppRun.wrapped "$@"
 * -----------------------
 *
 * Finally eebuild .AppImage from squashfs-root
 * ./appimagetool-x86_64.AppImage squashfs-root/
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h> /* MIN() */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PATH_MAX
# ifdef MAXPATHLEN
#  define PATH_MAX MAXPATHLEN
# else
#  define PATH_MAX 1024
# endif
#endif

#ifndef NAME_MAX
# ifdef MAXNAMELEN
#  define NAME_MAX MAXNAMELEN
# else
#  define NAME_MAX 252
# endif
#endif

typedef int (*execve_func_t)(const char *filename, char *const argv[], char *const envp[]);

#define VISIBLE __attribute__ ((visibility ("default")))

#ifdef DEBUG
#include <errno.h>
#define DEBUG_PRINT(...) \
        printf("APPIMAGE_EXEC>> " __VA_ARGS__); \

#else
#define DEBUG_PRINT(...)  /**/
#endif

static void env_free(char* const *env)
{
    size_t len = 0;
    while (env[len] != 0) {
        free(env[len]);
        len++;
    }
    free((char**)env);
}


static size_t get_number_of_variables(char* const *env)
{
    size_t number = 0;
    while (env[number] != 0) number++;
    
    return number != 0 ? (ssize_t)number : -1;
}

#if 0
/* remove substring in place */
static char* substr_remove(char* str, const char* sub) {
    size_t len = strlen(sub);
    if (len > 0) {
        char * p = str;
        while ((p = strstr(p, sub)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
            p = str;
        }
    }

    return str;
}


static char* prepend_char(const char* searchterm, const char c)
{
    int len = strlen(searchterm);
    char buffer[len + 2];
    char * p = buffer + 1;
    buffer[0] = c;
    strcpy(p, searchterm);
    return strdup(buffer);
}


static char* append_char(const char* searchterm, const char c)
{
    size_t len = strlen(searchterm);
    char buffer[len + 2];
    strcpy(buffer, searchterm);
    buffer[len] = c;
    buffer[len+1]= '\0';
    return strdup(buffer);
}
#endif


static char* update_path_envvar_to_remove_segment(const char* path_envvar, const char* segment_to_remove)
{
    size_t pl_len = strlen(path_envvar);
    char newev[pl_len + 1];
    char buffer[pl_len + 1];

    char* bp = strchr(path_envvar, '=');
    if (!bp) {
        return strdup(path_envvar);
    }
    bp++;

    /* copy env var name and the '=' to new env var */
    char* pend = mempcpy(newev, path_envvar, bp - path_envvar);
    int first_segment = 1;

    /* parse the path list into buffer segment by segment */
    const char *subp;
    for (const char *p = bp ; ;  p = subp) {

        /* if reached end of path_envvar stop */
        if (*p == '\0') break;

        subp = strchrnul(p, ':');

        char* bend = mempcpy(buffer, p, subp - p);
        *bend = '\0';

        /* if it is empty or fully matches segment ignore it */
        if ((buffer[0] == '\0') || (strcmp(buffer, segment_to_remove) == 0)) {
            /* skip over ':' and if no futher segments stop */
            if (*subp++ == '\0') break;
            continue;
        }

        /* prepend a ':' path separator if not first segment */
        if (!first_segment) {
            *pend++ = ':';
        } else {
            first_segment = 0;
        }

        /* append buffer to new env var */
        pend = mempcpy(pend, buffer, subp - p);

        /* skip over ':' and if no further segments stop */
        if (*subp++ == '\0') break;
    }
    *pend = '\0';;
    return strdup(newev);
}


static char* const* read_env_from_evp(char* const *evp)
{
    size_t num_vars = get_number_of_variables(evp);
    char** env = calloc(num_vars + 1, sizeof(char*));
    const char *applibpath = getenv("SIGIL_APPIMAGE_LIB_DIR");
    const char *execwrapper = getenv("SIGIL_EXEC_WRAPPER");
    size_t n = 0;
    size_t j = 0;
    while((evp[j] != 0) && n < num_vars) {
        char * ptr = evp[j];
        size_t var_len = strlen(ptr);
        if (var_len == 0)
            break;

        if (strncmp(ptr,"LD_LIBRARY_PATH=",16) == 0) {
            char* libpath = NULL;
            if (strlen(applibpath) > 0) {
                libpath = update_path_envvar_to_remove_segment(ptr, applibpath);
            } else {
                libpath = calloc(var_len + 1, sizeof(char));
                strncpy(libpath, ptr, var_len + 1);
            }
            DEBUG_PRINT("\tEdited LD_LIBRARY_PATH: %s\n", libpath);
            if (strcmp(libpath, "LD_LIBRARY_PATH=") != 0) {
                size_t newlen = strlen(libpath);
                env[n] = calloc(newlen + 1, sizeof(char));
                strncpy(env[n], libpath, newlen + 1);
                DEBUG_PRINT("\tenv var copied: %s\n", env[n]);
                n++;
            }
            free(libpath);

        } else if (strncmp(ptr,"LD_PRELOAD=",11) == 0) {
            char* preloadpath = NULL;
            if (strlen(execwrapper) > 0) {
                preloadpath = update_path_envvar_to_remove_segment(ptr, execwrapper);
            } else {
                preloadpath = calloc(var_len+1, sizeof(char));
                strncpy(preloadpath, ptr, var_len + 1);
            }
            DEBUG_PRINT("\tEdited LD_PRELOAD: %s\n", preloadpath);
            if (strcmp(preloadpath, "LD_PRELOAD=") != 0) {
                size_t newlen = strlen(preloadpath);
                env[n] = calloc(newlen + 1, sizeof(char));
                strncpy(env[n], preloadpath, newlen + 1);
                DEBUG_PRINT("\tenv var copied: %s\n", env[n]);
                n++;
            }
            free(preloadpath);
            
        } else {
            env[n] = calloc(var_len + 1, sizeof(char));
            strncpy(env[n], ptr, var_len + 1);
            DEBUG_PRINT("\tenv var copied: %s\n", env[n]);
            n++;
        }
        j++;
    }
    env[j] = (char*) NULL;

    return env;
}


static char* create_fullpath_using_path_env(const char* filename)
{
    const char* path = getenv("PATH");
    const char* nopath = "/bin:/usr/bin";
    if (!path) path = nopath;
    size_t file_len = strnlen(filename, NAME_MAX -1) + 1;
    size_t path_len = strnlen(path, PATH_MAX -1) + 1;
    char buffer[path_len + file_len + 1];
    const char *subp;
    for (const char *p = path ; ;  p = subp) {
        struct stat sb;
        if (*p == '\0') break;
        subp = strchrnul(p, ':');
        if (subp - p >= path_len) {
            continue;
        }
        char *pend = mempcpy(buffer, p, subp - p);
        *pend = '/';
        memcpy(pend + (p < subp), filename, file_len);
        DEBUG_PRINT("\tTesting fullpath: %s\n", buffer);
        if (stat(buffer, &sb) == 0 && sb.st_mode & S_IXUSR) {
            /* we have a match */
            return strdup(buffer);
        }
        if (*subp++ == '\0') break;
    }
    buffer[0] = '\0';

    /* return empty string not null pointer */
    return strdup(buffer);
}


static int is_external_process(const char *fullpath)
{
    const char *appdir = getenv("APPDIR");
    if (!appdir)
        return 0;
    DEBUG_PRINT("APPDIR = %s\n", appdir);

    return strncmp(fullpath, appdir, MIN(strlen(fullpath), strlen(appdir)));
}


static int exec_common(execve_func_t function, const char *filename, char* const argv[], char* const envp[])
{
    // Try to get the canonical path in case it's a relative path or symbolic link.
    char *fullpath = canonicalize_file_name(filename);
    /* FIXME - this should really be limited to exec*p* but */
    if (!fullpath)
        fullpath = create_fullpath_using_path_env(filename);
    DEBUG_PRINT("filename %s, fullpath %s\n", filename, fullpath);
    char* const *env = envp;
    if ((*fullpath == '\0') || (is_external_process(fullpath))) {
        DEBUG_PRINT("External process detected. filtering environment variables\n");
        env = read_env_from_evp(env);
        if (!env) {
            env = envp;
            DEBUG_PRINT("Error restoring original env vars\n");
        }
    }
    if (fullpath != filename)
        free(fullpath);
    int ret = function(filename, argv, env);
    if (env != envp)
        env_free(env);
    return ret;
}


VISIBLE int execve(const char *filename, char *const argv[], char *const envp[])
{
    DEBUG_PRINT("execve call intercepted: %s\n", filename);
    execve_func_t execve_orig = dlsym(RTLD_NEXT, "execve");
    if (!execve_orig) {
        DEBUG_PRINT("Error getting execve original symbol: %s\n", strerror(errno));
    }

    return exec_common(execve_orig, filename, argv, envp);
}


VISIBLE int execv(const char *filename, char *const argv[]) {
    DEBUG_PRINT("execv call intercepted: %s\n", filename);

    return execve(filename, argv, environ);
}


VISIBLE int execvpe(const char *filename, char *const argv[], char *const envp[])
{
    DEBUG_PRINT("execvpe call intercepted: %s\n", filename);
    execve_func_t execvpe_orig = dlsym(RTLD_NEXT, "execvpe");
    if (!execvpe_orig) {
        DEBUG_PRINT("Error getting execvpe original symbol: %s\n", strerror(errno));
    }

    return exec_common(execvpe_orig, filename, argv, envp);
}


VISIBLE int execvp(const char *filename, char *const argv[]) {
    DEBUG_PRINT("execvp call intercepted: %s\n", filename);

    return execvpe(filename, argv, environ);
}
