/*
utils.h - v1.0 - A simple logging utility for C
Dual-licensed to the public domain and under the following license:
You are granted a perpetual, irrevocable license to copy, modify,
publish, and distribute this file as you see fit.

To use:
- Define `UTILS_LOG_IMPLEMENTATION` in **one** C file before including this
header to include the implementation.
- Include this file normally for declaration-only usage.

Example:
    #define UTILS_LOG_IMPLEMENTATION
    #include "utils.h"

    int main() {
        LOG(LOG_INFO, "This is a test message: %d", 42);
        LOG(LOG_ERROR, "This is an error: %s", "something went wrong");
    }

Options:
- Define `UTILS_LOG_STATIC` before including to make all functions `static`.
*/

#ifndef UTILS_H
#define UTILS_H

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>

// Log levels
#define LOG_INFO "INFO"
#define LOG_ERROR "ERROR"

// Macro for logging
#define LOG(level, ...) log_message(level, __VA_ARGS__)

// Macro for dynamic lists
#define da_append(array, item)                                                 \
  do {                                                                         \
    if ((array)->count >= (array)->capacity) {                                 \
      (array)->capacity =                                                      \
          (array)->capacity == 0 ? INITIAL_CAPACITY : (array)->capacity * 2;   \
      (array)->items = realloc((array)->items,                                 \
                               (array)->capacity * sizeof(*(array)->items));   \
      assert((array)->items != NULL && "Buy more RAM lol");                    \
    }                                                                          \
    (array)->items[(array)->count++] = (item);                                 \
  } while (0)

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

// Allow static linkage
#ifdef UTILS_LOG_STATIC
#define UTILS_DEF static
#else
#define UTILS_DEF
#endif

// Function declarations
UTILS_DEF void log_message(const char *level, const char *format, ...);
UTILS_DEF unsigned char *
read_entire_file(const char *directory, const char *file_path, long *file_size);
UTILS_DEF char *find_file_in_directory(const char *target_dir,
                                       const char *target_file);

UTILS_DEF bool write_file(const char *file_path, const unsigned char *data,
                          long data_size);

#ifdef UTILS_LOG_IMPLEMENTATION

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Function implementation
UTILS_DEF void log_message(const char *level, const char *format, ...) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char time_str[20];
  strftime(time_str, sizeof(time_str), "%H:%M:%S", t);

  printf("[%s] [%s] ", time_str, level);

  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  printf("\n");
}

UTILS_DEF bool write_file(const char *file_path, const unsigned char *data,
                          long data_size) {
  FILE *f = fopen(file_path, "wb");
  if (!f) {
    return false;
  }

  if (fwrite(data, 1, data_size, f) != (size_t)data_size) {
    fclose(f);
    return false;
  }
  fclose(f);

  return true;
}

UTILS_DEF unsigned char *read_entire_file(const char *directory,
                                          const char *file_name,
                                          long *file_size) {
  char full_path[1024];
  snprintf(full_path, sizeof(full_path), "%s/%s", directory, file_name);

  FILE *f = fopen(full_path, "rb");
  if (!f) {
    return NULL;
  }

  fseek(f, 0, SEEK_END);
  *file_size = ftell(f);
  rewind(f);

  unsigned char *data = malloc(*file_size);
  if (!data) {
    fclose(f);
    return NULL;
  }

  if (fread(data, 1, *file_size, f) != (size_t)*file_size) {
    free(data);
    fclose(f);
    return NULL;
  }
  fclose(f);

  return data;
}

/*UTILS_DEF unsigned char *read_entire_file(const char *file_path,*/
/*                                          long *file_size) {*/
/*  FILE *f = fopen(file_path, "rb");*/
/*  if (!f) {*/
/*    return NULL;*/
/*  }*/
/**/
/*  fseek(f, 0, SEEK_END);*/
/*  *file_size = ftell(f);*/
/*  rewind(f);*/
/**/
/*  unsigned char *data = malloc(*file_size);*/
/*  if (!data) {*/
/*    fclose(f);*/
/*    return NULL;*/
/*  }*/
/**/
/*  if (fread(data, 1, *file_size, f) != (size_t)*file_size) {*/
/*    free(data);*/
/*    fclose(f);*/
/*    return NULL;*/
/*  }*/
/*  fclose(f);*/
/**/
/*  return data;*/
/*}*/

char *find_file_in_directory(const char *target_dir, const char *target_file) {
  DIR *directory;
  struct dirent *entry;

  log_message(LOG_INFO, "Reading file %s from directory %s", target_file,
              target_dir);

  directory = opendir(target_dir);
  if (directory == NULL) {
    log_message(LOG_ERROR, "Could not read directory %s: %s", target_dir,
                strerror(errno));
    return NULL;
  }

  char *found_file = NULL;
  while ((entry = readdir(directory))) {
    if (strcmp(entry->d_name, target_file) == 0) {
      found_file = strdup(entry->d_name);
      break;
    }
  }

  closedir(directory);
  return found_file;
}

#endif // UTILS_LOG_IMPLEMENTATION

#endif // UTILS_H
