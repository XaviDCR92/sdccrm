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

#include "function_list.h"
#include "alloc.h"
#include "common.h"
#include "options.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CALL_DIRECTIVE "call"

static bool is_label(const char *line, size_t length);
static bool is_call(const char *line);
static const char *get_ref(const char *line);
static const char *get_call(const char *line);
static struct file parse(const char *buf);
static void append_called_label(const char *const called_label, struct file *const f);
static void append_label(const char *p, size_t line_no, const char *line, struct label *l);
static void append_global_label(const char *p, size_t line_no, const char *line, struct label *l);
static void append_static_label(const char *p, size_t line_no, const char *line, struct label *l);
static size_t get_label_end(const char *p, size_t line_no);

struct tree get_function_list(const size_t n_files, const char *const *const files)
{
    struct tree t = {0};

    for (size_t i = 0; i < n_files; i++)
    {
        const char *const file_name = files[i];
        char *const buf = open(file_name);

        if (buf)
        {
            const struct file label_list = parse(buf);

            t.files = alloc(t.files, t.n_files);

            if (t.files)
            {
                t.files[t.n_files] = label_list;
                t.files[t.n_files].name = file_name;

                t.n_files++;
            }

            free(buf);
        }
        else
        {
            /* File could not be read for some reason. */
            break;
        }
    }

    return t;
}

static struct file parse(const char *const buf)
{
    const char *p = buf;
    char line[MAX_CH_PER_LINE];
    size_t len;

    struct
    {
        char **names;
        size_t n;
    } global = {0};

    struct file f = {0};
    bool area_code_found = false;

    for (size_t line_no = 1; (p = get_line(p, line, &len)); line_no++)
    {
        const char *const global_label = get_global(line);

        if (global_label)
        {
            global.names = alloc(global.names, global.n);

            if (global.names)
            {
                global.names[global.n] = malloc(sizeof (**global.names) * len);

                /* Dump global label name into the list. */
                strcpy(global.names[global.n++], global_label);
            }
            else
            {
            }
        }
        else if (!area_code_found)
        {
            if (!strcmp(".area CODE", line))
                area_code_found = true;
        }
        else if (is_label(line, len))
        {
            /* Suppress ':'. */
            line[len -= 2] = '\0';

            /* Check whether found label is global. */
            bool match = false;

            f.labels = alloc(f.labels, f.n_labels);

            if (f.labels)
            {
                struct label *l = &f.labels[f.n_labels];

                /* Clear newly allocated data. */
                memset(l, 0, sizeof *l);

                for (size_t i = 0; i < global.n; i++)
                {
                    if (!strcmp(global.names[i], line))
                    {
                        append_global_label(p, line_no, line, l);

                        match = true;
                        break;
                    }
                }

                if (!match)
                {
                    append_static_label(p, line_no, line, &f.labels[f.n_labels]);
                }

                f.n_labels++;
            }
        }
        else if (is_call(line))
        {
            /* Call to a specific label. */
            append_called_label(get_call(line), &f);
        }
        else
        {
            /* Determine whether there is a reference to a specific label. */
            const char *const ref = get_ref(line);

            if (ref)
            {
                LOG("Function %s is being referrenced", ref);
                append_called_label(ref, &f);
            }
        }
    }

    /* Clean up locally allocated data. */
    if (global.names)
    {
        for (size_t i = 0; i < global.n; i++)
        {
            if (global.names[i])
            {
                free(global.names[i]);
            }
        }

        free(global.names);
    }

    return f;
}

static const char *get_ref(const char *const line)
{
    const char *p;

    for (p = line; *p != '#' && *p; p++);

    if (*p)
    {
        static const char pattern[] = "#(_";

        if (!strncmp(p, pattern, static_strlen(pattern)))
        {
            static char l[MAX_CH_PER_LINE];

            /* Substract one position so the '_' is also returned. */
            const size_t offset = static_strlen(pattern) - 1;
            size_t i = 0;

            for (p += offset; *p && *p != ')' && *p != ' ' && i < lengthof (l); p++)
            {
                l[i++] = *p;
            }

            l[i] = '\0';

            return l;
        }
    }

    /* No reference was found. */
    return NULL;
}

static void append_called_label(const char *const called_label, struct file *const f)
{
    if (f && called_label)
    {
        if (f->n_labels)
        {
            struct label *const l = &f->labels[f->n_labels - 1];

            l->calls = alloc(l->calls, l->n_calls);

            if (l->calls)
            {
                char **const calls = &l->calls[l->n_calls];
                const size_t called_label_len = strlen(called_label) + 1;

                *calls = malloc(called_label_len * sizeof *called_label);

                if (*calls)
                {
                    strncpy(*calls, called_label, called_label_len);
                    l->n_calls++;
                }
            }
        }
    }
}

static void append_label(const char *const p, const size_t line_no, const char *const line, struct label *const l)
{
    if ((l->name = malloc((strlen(line) + 1) * sizeof *line)))
    {
        strcpy(l->name, line);
    }

    l->start_line = line_no;
    l->end_line = get_label_end(p, line_no);
}

static void append_global_label(const char *const p, const size_t line_no, const char *const line, struct label *const l)
{
    l->global = true;
    append_label(p, line_no, line, l);
}

static void append_static_label(const char *const p, const size_t line_no, const char *const line, struct label *const l)
{
    l->global = false;
    append_label(p, line_no, line, l);
}

static size_t get_label_end(const char *p, size_t line_no)
{
    size_t len;
    char line[MAX_CH_PER_LINE];
    bool search = false;

    for (line_no += 1; (p = get_line(p, line, &len)); line_no++)
    {
        if (!search)
        {
            search = true;
        }
        else if (is_label(line, len) || strstr(line, ".area"))
        {
            break;
        }
    }

    return line_no - 1;
}

static bool is_label(const char *const line, const size_t length)
{
    if (line && length >= static_strlen("_a:"))
    {
        return line[0] == '_' && line[1] != '_' && line[length - 2] == ':';
    }

    return false;
}

static const char *get_call(const char *line)
{
    if (line)
    {
        for (char c = *line; !isspace(c); c = *(++line));

        return ++line;
    }

    return NULL;
}

static bool is_call(const char *const line)
{
    if (line)
    {
        return !strncmp(line, CALL_DIRECTIVE, static_strlen(CALL_DIRECTIVE));
    }

    return false;
}
