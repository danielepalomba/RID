#ifndef UTF8_H
#define UTF8_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief  Determine the byte length of a UTF-8 character from its leading byte.
 * @param[in] lead  The first byte of the UTF-8 sequence.
 * @return Number of bytes in the character (1–4).
 *         Returns 1 for invalid leading bytes (treated as a single byte).
 */
int utf8_byte_length(uint8_t lead);

/**
 * @brief  Encode a Unicode codepoint into its UTF-8 byte representation.
 * @param[in]  codepoint  The Unicode codepoint to encode.
 * @param[out] out        Output buffer (must have room for at least 4 bytes).
 * @return Number of bytes written (1–4), or 3 for the replacement character
 *         U+FFFD when the codepoint is out of range.
 */
int utf8_encode(uint32_t codepoint, char *out);

/**
 * @brief  Decode one UTF-8 codepoint from a byte string.
 * @param[in]  s   Pointer to the start of the UTF-8 sequence.
 * @param[out] cp  Decoded codepoint is stored here.
 * @return Number of bytes consumed (1–4).
 * @note   On an invalid sequence, @p *cp is set to the raw leading byte
 *         value and 1 is returned.
 */
int utf8_decode(const char *s, uint32_t *cp);

/**
 * @brief  Count the number of codepoints in a UTF-8 byte buffer.
 * @param[in] s         Pointer to the UTF-8 string.
 * @param[in] byte_len  Length of the buffer in bytes.
 * @return Number of codepoints found.
 */
size_t utf8_codepoint_count(const char *s, size_t byte_len);

/**
 * @brief  Convert a codepoint index to a byte offset within a UTF-8 string.
 * @param[in] s         Pointer to the UTF-8 string.
 * @param[in] byte_len  Total byte length of the string.
 * @param[in] cp_index  Zero-based codepoint index.
 * @return Byte offset corresponding to @p cp_index, or @p byte_len if the
 *         index exceeds the number of codepoints.
 */
size_t utf8_byte_offset(const char *s, size_t byte_len, int cp_index);

/**
 * @brief  Convert a byte offset to a codepoint index within a UTF-8 string.
 * @param[in] s            Pointer to the UTF-8 string.
 * @param[in] byte_offset  Byte offset to convert.
 * @return Zero-based codepoint index.
 */
int utf8_cp_index(const char *s, size_t byte_offset);

#endif
