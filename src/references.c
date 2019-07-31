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

#include "references.h"
#include "common.h"
#include "options.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static void find_used_labels(struct tree *t, const struct label *l, const char *file_name);

void find_references(struct tree *t)
{
    if (t)
    {
        const char *const entry = get_entry_label();
        struct file *f = NULL;
        struct label *l = NULL;
        bool match = false;

        /* Entry label must never be removed. */
        exclude_label(entry);

        for (size_t i = 0; i < t->n_files; i++)
        {
            f = &t->files[i];

            for (size_t j = 0; j < f->n_labels; j++)
            {
                l = &f->labels[j];

                if (l->global)
                {
                    if (!strcmp(l->name, entry))
                    {
                        match = true;

                        /* Exit nested loop. */
                        goto end_loop;
                    }
                }
            }
        }

end_loop:
        if (match && f && l)
        {
            l->used = true;

            find_used_labels(t, l, f->name);
        }
        else
        {
            fprintf(stderr, "Could not find entry point %s from input files.\n", entry);
        }
    }
}

static void find_used_labels(struct tree *const t, const struct label *const l, const char *const file_name)
{
    if (l->calls)
    {
        for (size_t i = 0; i < l->n_calls; i++)
        {
            const char *const ref = l->calls[i];
            bool match = false;

            for (size_t j = 0; j < t->n_files && !match; j++)
            {
                struct file *const f = &t->files[j];

                for (size_t k = 0; k < f->n_labels; k++)
                {
                    struct label *const called_l = &f->labels[k];

                    if ((match = !strcmp(ref, called_l->name)))
                    {
                        if (called_l->global)
                        {
                            /* Reference to a global label from another file. */
                        }
                        else if (!strcmp(f->name, file_name))
                        {
                            /* Reference to a static label inside the same file. */
                        }
                        else
                        {
                            /* Even if symbol name matches, it is not a global
                             * symbol and it is not being referred from the same file. */
                            match = false;
                        }
                    }
                    else if (is_label_excluded(called_l->name) && !called_l->used)
                    {
                        /* Label is referenced somehow, as indicated by the user. */
                        match = true;
                    }

                    if (match)
                    {
                        called_l->used = true;
                        LOG("%s (%s) marked as used", called_l->name, f->name);

                        find_used_labels(t, called_l, f->name);
                    }
                }
            }

            if (!match)
            {
                /* Called label could not be found, but do not worry: just avoid removing it. */
            }
        }
    }
}
