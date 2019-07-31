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

#include "options.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static struct
{
    bool verbose;
    const char **excluded_labels;
    size_t n_excluded_labels;
    const char *entry_label;
} config;

bool verbose(void)
{
    return config.verbose;
}

void enable_verbose(void)
{
    config.verbose = true;
}

const char *get_entry_label(void)
{
    return config.entry_label ? config.entry_label : DEFAULT_ENTRY_LABEL;
}

void set_entry_label(const char *const l)
{
    config.entry_label = l;
}

bool is_label_excluded(const char *const l)
{
    if (config.excluded_labels)
    {
        for (size_t i = 0; i < config.n_excluded_labels; i++)
        {
            const char *const excl_l = config.excluded_labels[i];

            if (excl_l)
            {
                if (!strcmp(l, excl_l))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void exclude_label(const char *const l)
{
    if (config.excluded_labels)
    {
        if (!is_label_excluded(l))
        {
            config.excluded_labels = realloc(config.excluded_labels, (config.n_excluded_labels + 1) * sizeof *config.excluded_labels);
        }
        else
        {
            fprintf(stderr, "Label %s set as excluded more than once\n", l);
            return;
        }
    }
    else
    {
        config.excluded_labels = malloc(sizeof *config.excluded_labels);
    }

    if (config.excluded_labels)
    {
        config.excluded_labels[config.n_excluded_labels++] = l;
    }
}

void options_cleanup(void)
{
    if (config.excluded_labels)
    {
        free(config.excluded_labels);
    }
}
