#include "rrow.h"

/** @copydoc rrow_create */
void rrow_create(Rrow row, const char *s, size_t len){
    row->size = len;
    row->chars = malloc(len + 1);

    memcpy(row->chars, s, len);
    row->chars[len] = '\0';
}

/** @copydoc rrow_insert_char */
void rrow_insert_char(Rrow row, int at, uint32_t codepoint){
    char encoded[4];
    int enc_len = utf8_encode(codepoint, encoded);

    size_t num_cp = utf8_codepoint_count(row->chars, row->size);
    if(at < 0 || (size_t)at > num_cp)
        at = (int)num_cp;

    size_t byte_off = utf8_byte_offset(row->chars, row->size, at);

    row->chars = realloc(row->chars, row->size + (size_t)enc_len + 1);
    memmove(&row->chars[byte_off + enc_len],
            &row->chars[byte_off],
            row->size - byte_off + 1); /* +1 to include null terminator */

    memcpy(&row->chars[byte_off], encoded, (size_t)enc_len);
    row->size += (size_t)enc_len;
}

/** @copydoc rrow_del_char */
void rrow_del_char(Rrow row, int at){
    size_t num_cp = utf8_codepoint_count(row->chars, row->size);
    if(at < 0 || (size_t)at >= num_cp)
        return;

    size_t byte_off = utf8_byte_offset(row->chars, row->size, at);
    int char_len = utf8_byte_length((uint8_t)row->chars[byte_off]);

    /* Clamp to buffer bounds to avoid reading past the end */
    if(byte_off + (size_t)char_len > row->size)
        char_len = (int)(row->size - byte_off);

    memmove(&row->chars[byte_off],
            &row->chars[byte_off + char_len],
            row->size - byte_off - (size_t)char_len + 1); /* +1 to include null terminator */

    row->size -= (size_t)char_len;
}
