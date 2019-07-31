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

#include "common.h"
#include "options.h"
#include "function_list.h"
#include "references.h"
#include "remove_unused.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define PARAM_STR "[param]"

/* Avoid using define so only one copy is stored. */
static const char app[] = "sdccrm";

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

static void usage(void);
static void start(size_t n_files, const char *const *files);
static void cleanup(struct tree *t);

int main(const int argc, const char *const argv[])
{
    enum
    {
        EXECUTABLE_PATH,
        DEFAULT_ASM_FILE,

        MINIMUM_PARAMETERS
    };

    if (argc >= MINIMUM_PARAMETERS)
    {
        /* Calculate where file list starts. If options
         * are given, this index shall be increased. */
        size_t i;

        bool reading_parameter = false;
        size_t param_i;

        for (i = DEFAULT_ASM_FILE; i < argc; i++)
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

        if (i < argc)
        {
            /* Get file list given calculated index. */
            const char *const *const files = &argv[i];

            /* Substract executable path. */
            const size_t n_files = argc - i;

            start(n_files, files);
        }
        else
        {
            usage();
        }
    }
    else
    {
        usage();
    }

    return errno;
}

static void usage(void)
{
    printf("Usage:\n%s [options] file1 file2 ... filen\n", app);

    printf("Options:\n");

    for (size_t i = 0; i < lengthof (options); i++)
    {
        printf("%s%s%s.\n", options[i].flag, options[i].param ? " " PARAM_STR "\t" : "\t\t", options[i].descr);
    }
}

static void start(const size_t n_files, const char *const *const files)
{
    struct tree t = get_function_list(n_files, files);

    find_references(&t);

    remove_unused(&t);

    cleanup(&t);
    options_cleanup();
}

static void cleanup(struct tree *const t)
{
    if (t->files)
    {
        if (t->files->labels)
        {
            for (size_t i = 0; i < t->files->n_labels; i++)
            {
                struct label *const l = &t->files->labels[i];

                if (l->calls)
                {
                    for (size_t j = 0; j < l->n_calls; j++)
                    {
                        free(l->calls[j]);
                    }

                    free(l->calls);
                }

                if (l->name)
                {
                    free(l->name);
                }
            }

            free(t->files->labels);
        }

        free(t->files);
    }
}
