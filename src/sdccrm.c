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
        const int i = parse_options(DEFAULT_ASM_FILE, argc, argv);

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
        for (size_t i = 0; i < t->n_files; i++)
        {
            struct file *const f = &t->files[i];

            if (f->labels)
            {
                for (size_t j = 0; j < f->n_labels; j++)
                {
                    struct label *const l = &f->labels[j];

                    if (l->calls)
                    {
                        for (size_t k = 0; k < l->n_calls; k++)
                        {
                            free(l->calls[k]);
                        }

                        free(l->calls);
                    }

                    if (l->name)
                    {
                        free(l->name);
                    }
                }

                free(f->labels);
            }
        }

        free(t->files);
    }
}
