#include "alloc.h"
#include <stdlib.h>

void *alloc_(void *p, const size_t sz, const size_t elem)
{
    void *const t = p;

    p = realloc(p, (elem + 1) * sz);

    if (t && !p)
    {
        free(t);
        return t;
    }

    return p;
}
