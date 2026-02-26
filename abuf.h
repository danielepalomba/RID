#ifndef ABUF_H
#define ABUF_H

#include <stdlib.h>

/** @brief Static initializer for an empty append buffer. */
#define ABUF_INIT {NULL, 0}

/**
 * @brief Dynamically growable byte buffer used for batched terminal writes.
 */
struct abuf{
    char *b;
    int len;
};

typedef struct abuf *Abuf;

/**
 * @brief  Append bytes to the dynamic buffer.
 * @param[in,out] ab   Pointer to the append buffer.
 * @param[in]     s    Source bytes to append.
 * @param[in]     len  Number of bytes to append.
 */
void ab_append(Abuf ab, const char *s, int len);

/**
 * @brief  Free the memory owned by the append buffer.
 * @param[in,out] ab  Pointer to the append buffer to free.
 */
void ab_free(Abuf ab);

#endif
