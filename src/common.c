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
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

const char *get_line(const char *p, char *const line, size_t *const len)
{
    if (line && len)
    {
        while (*p)
        {
            *len = 0;

            while (isspace(*p))
            {
                p++;
            }

            if (*p == ';')
            {
                /* Skip comments until end of line is found. */
                while ((*p != '\0') && (*p != '\n'))
                {
                    p++;
                }
            }
            else
            {
                while ((*p != '\0') && (*p != '\n') && (*p != '\r'))
                {
                    if (*len < (size_t)MAX_CH_PER_LINE)
                    {
                        line[(*len)++] = *p;
                    }

                    p++;
                }

                /* A line has been read. */
                line[(*len)++] = '\0';

                return p;
            }
        }
    }

    return NULL;
}

char *open(const char *const path)
{
    FILE *const f = fopen(path, "rb");

    if (f)
    {
        if (!fseek(f, 0, SEEK_END))
        {
            const size_t sz = ftell(f);

            if (!fseek(f, 0, SEEK_SET))
            {
                char *const buf = malloc(sz);

                const size_t read = fread(buf, sizeof (*buf), sz, f);

                if (read == sz)
                {
                    fclose(f);
                    return buf;
                }
                else
                {
                    fprintf(stderr, "Only %ld out of %ld bytes were read from %s\n", read, sz, path);
                    free(buf);
                    goto f_error;
                }
            }
            else
            {
                goto f_error;
            }
        }
        else
        {
            goto f_error;
        }
    }
    else
    {
        fprintf(stderr, "Could not open file %s\n", path);
    }

    return NULL;

f_error:
    if (f)
        fclose(f);

    return NULL;
}

const char *get_global(const char *const line)
{
    if (line)
    {
        const char *p = line;

        if (!strncmp(p, ".globl", static_strlen(".globl")))
        {
            /* A global symbol is being defined. */

            while (*p && (*p != '_'))
            {
                p++;
            }

            if (*p)
            {
                /* A global label has been defined. */
                return p;
            }
        }
    }

    return NULL;
}
