#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>

#define alloc(p, elem) alloc_(p, sizeof *(p), elem);

void *alloc_(void *p, size_t sz, size_t elem);

#endif /* ALLOC_H */
