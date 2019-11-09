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

#include "remove_unused.h"
#include "common.h"
#include "options.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static void write_filtered_file(FILE *const fi, const struct tree *const t, const struct file *const f, const char *p);

static const char *extension = "rm";

void remove_unused(const struct tree *const t)
{
    if (!t) return;

    for (size_t i = 0; i < t->n_files; i++)
    {
        const struct file *const f = &t->files[i];
        if (!f->name) continue;

        char *const buf = open(f->name);

        if (!buf) return;

        if(replace())
        {
            FILE *const fi = fopen(f->name, "w");

            if (fi)
            {
                LOG("Filtering %s..", f->name);
                write_filtered_file(fi, t, f, buf);

                fclose(fi);
            }

        }
        else
        {
            /* Use temporary file extension ".asmrm". */
            char *const n = malloc((strlen(f->name) + strlen(extension) + 1) * sizeof *n);
            if (!n) 
            {
                free(buf);
                continue;
            }

            strcpy(n, f->name);

            /* Create final file name. */
            if (strcat(n, extension))
            {
                FILE *const fi = fopen(n, "w");

                if (fi)
                {
                    LOG("Filtering %s..", n);
                    write_filtered_file(fi, t, f, buf);

                    fclose(fi);
                }

            }

            free(n);
        }

        free(buf);
    }
}

static void write_filtered_file(FILE *const fi, const struct tree *const t, const struct file *const f, const char *p)
{
    /* When true, all lines belonging to selected label shall be ignored. */
    bool remove_label = false;
    char line[MAX_CH_PER_LINE];

    for (size_t line_no = 1, len; (p = get_line(p, line, &len)); line_no++)
    {
        if (!remove_label)
        {
            bool skip_global_declaration = false;
            const char *const global_label = get_global(line);

            if (global_label)
            {
                /* A global label declaration was found.
                 * Determine if it has to be removed. */

                for (size_t i = 0; i < t->n_files; i++)
                {
                    const struct file *const f = &t->files[i];

                    for (size_t j = 0; j < f->n_labels; j++)
                    {
                        const struct label *const l = &f->labels[j];

                        if (!strcmp(global_label, l->name))
                        {
                            skip_global_declaration = !l->used && l->global;

                            /* Exit nested loop. */
                            goto global_label_found;
                        }
                    }
                }
            }
            else
            {
                for (size_t j = 0; j < f->n_labels; j++)
                {
                    const struct label *const l = &f->labels[j];

                    if (l->start_line == line_no && !l->used)
                    {
                        remove_label = true;
                        LOG("Removing unused label %s (%s)", l->name, f->name);

                        break;
                    }
                }
            }

global_label_found:

            if (!remove_label && !skip_global_declaration)
            {
                fprintf(fi, "%s\n", line);
            }
        }
        else
        {
            for (size_t j = 0; j < f->n_labels; j++)
            {
                const struct label *const l = &f->labels[j];

                if (l->end_line == line_no)
                {
                    remove_label = false;
                    break;
                }
            }
        }
    }
}
