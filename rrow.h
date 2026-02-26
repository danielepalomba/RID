#ifndef RROW_H
#define RROW_H

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "utf8.h"

/**
 * @brief A single row of text stored as a UTF-8 byte buffer.
 */
struct rrow{
    size_t size;   /**< Byte length of the row (excluding null terminator). */
    char *chars;   /**< Heap-allocated, null-terminated UTF-8 string.       */
};

typedef struct rrow *Rrow;

/**
 * @brief  Initialize a row from a byte string.
 * @param[out] row  Row to initialize.
 * @param[in]  s    Source bytes (not required to be null-terminated).
 * @param[in]  len  Number of bytes in @p s.
 */
void rrow_create(Rrow row, const char *s, size_t len);

/**
 * @brief  Insert a Unicode codepoint at the given codepoint index.
 * @param[in,out] row        Row to modify.
 * @param[in]     at         Zero-based codepoint index for insertion.
 * @param[in]     codepoint  Unicode codepoint to insert.
 */
void rrow_insert_char(Rrow row, int at, uint32_t codepoint);

/**
 * @brief  Delete the codepoint at the given codepoint index.
 * @param[in,out] row  Row to modify.
 * @param[in]     at   Zero-based codepoint index to delete.
 */
void rrow_del_char(Rrow row, int at);

#endif
