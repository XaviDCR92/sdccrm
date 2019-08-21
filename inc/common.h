/*
 * Copyright (C) 2019  Xavier Del Campo Romero <xavi.dcr@tutanota.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdbool.h>

enum
{
    /* Defined by SDCC's assembly specifications. */
    MAX_CH_PER_LINE = 128
};

#if defined(__GNUC__) && __STDC_VERSION__ >= 201112L

/* This macro generates compile-time errors when
 * a pointer (instead of an array) is being used. */
#define lengthof(a)                         \
    _Generic (&a,                           \
    typeof (*a) **: (void) 0,               \
    typeof (*a) *const *: (void) 0,         \
    default: sizeof (a) / sizeof ((a)[0]))

#else

/* Unsafe yet portable version. */
#define lengthof(a) (sizeof (a) / sizeof ((a)[0]))

#endif

/* This macro calculates string length at compile-time.
 * Take into account pointers are not valid and will
 * return a compile-time error. */
#define static_strlen(a) (lengthof (a) - 1)

struct tree
{
    struct file *files;
    size_t n_files;
};

struct file
{
    struct label *labels;
    const char *name;
    size_t n_labels;
};

struct label
{
    bool global;
    bool used;
    char *name;
    char **calls;
    size_t n_calls;
    size_t start_line;
    size_t end_line;
};

typedef struct
{
    bool global;
} labell;

const char *get_line(const char *p, char *const line, size_t *const len);
char *open(const char *path);
const char *get_global(const char *line);
bool verbose(void);
void enable_verbose(void);

#endif /* SDCCRM_COMMON_H */
