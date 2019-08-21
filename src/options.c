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

#include "options.h"
#include "common.h"
#include "alloc.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PARAM_STR "[param]"

static const struct
{
    const char *flag;
    const char *descr;
    bool param;
    union
    {
        void (*f)(void);
        void (*f_param)(const char *);
    };
} options[] =
{
    {
        .flag = "-v",
        .descr = "Enables verbose mode",
        .param = false,
        .f = enable_verbose
    },

    {
        .flag = "-x",
        .descr = "Excludes a given label " PARAM_STR " from being removed",
        .param = true,
        .f_param = exclude_label
    },

    {
        .flag = "-e",
        .descr = "Sets " PARAM_STR " as entry label. Defaults to " DEFAULT_ENTRY_LABEL,
        .param = true,
        .f_param = set_entry_label
    }
};

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
            config.excluded_labels = alloc(config.excluded_labels, config.n_excluded_labels);
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

int parse_options(const int offset, const int argc, const char *const *const argv)
{
    /* Calculate where file list starts. If options
     * are given, this index shall be increased. */
    int i;

    bool reading_parameter = false;
    size_t param_i;

    for (i = offset; i < argc; i++)
    {
        const char *const option = argv[i];

        if (*option == '-')
        {
            for (size_t j = 0; j < lengthof (options); j++)
            {
                if (!strcmp(option, options[j].flag))
                {
                    if (options[j].param)
                    {
                        reading_parameter = true;
                        param_i = j;
                    }
                    else
                    {
                        if (options[j].f)
                        {
                            options[j].f();
                        }
                    }

                    break;
                }
            }
        }
        else if (reading_parameter)
        {
            if (options[param_i].f_param)
            {
                options[param_i].f_param(option);
            }
            else
            {
                WARNING("No callback defined for flag %s", options[param_i].flag);
            }

            reading_parameter = false;
        }
        else
        {
            /* End of option switches. */
            break;
        }
    }

    return i;
}

void usage(void)
{
    /* Avoid using define so only one copy is stored. */
    static const char app[] = "sdccrm";

    printf("Usage:\n%s [options] file1 file2 ... filen\n", app);

    printf("Options:\n");

    for (size_t i = 0; i < lengthof (options); i++)
    {
        printf("%s%s%s.\n", options[i].flag, options[i].param ? " " PARAM_STR "\t" : "\t\t", options[i].descr);
    }
}
