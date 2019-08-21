/*
 * 2019 Xavier Del Campo Romero  <xavi.dcr@tutanota.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include <stdio.h>

/* Defines default main label. */
#define DEFAULT_ENTRY_LABEL "_main"

#define WARNING(str, ...)                                   \
    if (verbose())                                          \
    {                                                       \
        fprintf(stderr, "%s(), %d: (warning) " str ".\n",   \
            __func__, __LINE__, __VA_ARGS__);               \
    }

#define LOG(str, ...)                           \
    if (verbose())                              \
    {                                           \
        printf("%s(), %d: " str ".\n",          \
            __func__, __LINE__, __VA_ARGS__);   \
    }

bool verbose(void);
const char *get_entry_label(void);
void set_entry_label(const char *const l);
void enable_verbose(void);
void exclude_label(const char *const l);
bool is_label_excluded(const char *const l);
void options_cleanup(void);
int parse_options(const int offset, const int argc, const char *const *const argv);
void usage(void);

#endif /* OPTIONS_H */
