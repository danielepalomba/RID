#include "syntax.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static Syntax *syn = NULL;

/**
 * @brief  Check whether a character can appear inside a C identifier.
 * @param[in] c  Character to test.
 * @return Non-zero if @p c is alphanumeric or underscore.
 */
static int is_ident_char(char c){
    return isalnum((unsigned char)c) || c == '_';
}

/**
 * @brief  Grow the output buffer so it can hold at least @p needed more bytes.
 * @param[in,out] buf     Pointer to the heap-allocated buffer.
 * @param[in,out] cap     Current capacity of the buffer.
 * @param[in]     used    Bytes already written.
 * @param[in]     needed  Additional bytes required.
 */
static void ensure_capacity(char **buf, size_t *cap, size_t used, size_t needed){
    while(used + needed + 1 > *cap){
        *cap *= 2;
        *buf = realloc(*buf, *cap);
    }
}

/**
 * @brief  Append raw bytes to the output buffer.
 * @param[in,out] buf  Pointer to the heap-allocated buffer.
 * @param[in,out] cap  Current capacity.
 * @param[in,out] len  Bytes already written (updated on return).
 * @param[in]     src  Source bytes to copy.
 * @param[in]     n    Number of bytes to copy.
 */
static void out_append(char **buf, size_t *cap, size_t *len,
                       const char *src, size_t n){
    ensure_capacity(buf, cap, *len, n);
    memcpy(*buf + *len, src, n);
    *len += n;
}

/**
 * @brief  Append a token wrapped in ANSI color sequences to the output buffer.
 * @param[in,out] buf    Pointer to the heap-allocated buffer.
 * @param[in,out] cap    Current capacity.
 * @param[in,out] len    Bytes already written (updated on return).
 * @param[in]     color  Color to apply.
 * @param[in]     text   Token text.
 * @param[in]     tlen   Byte length of @p text.
 */
static void out_append_colored(char **buf, size_t *cap, size_t *len,
                               ColorCode color, const char *text, size_t tlen){
    const char *cseq = scolor_get_seq(color);
    const char *rseq = scolor_get_seq(RESET);
    size_t clen = strlen(cseq);
    size_t rlen = strlen(rseq);
    ensure_capacity(buf, cap, *len, clen + tlen + rlen);
    memcpy(*buf + *len, cseq, clen);  *len += clen;
    memcpy(*buf + *len, text, tlen);  *len += tlen;
    memcpy(*buf + *len, rseq, rlen);  *len += rlen;
}

/** @copydoc syntax_set_ext */
void syntax_set_ext(const char *ext){
    if(!syn){
        syn = malloc(sizeof(Syntax));
        if(!syn)
            exit(EXIT_FAILURE);
    }

    if(strcmp(ext, EXT_C) == 0 || strcmp(ext, EXT_H) == 0){
        syn->lang = LANG_C;
        const char *tmp[] = C_KEYWORDS;
        for(int i = 0; i < C_KEYWORDS_NUM; i++)
            syn->keywords[i] = tmp[i];
    }
}

/** @copydoc syntax_get_keyword_color */
ColorCode syntax_get_keyword_color(int idx){
    if(idx < C_KW_MODIFIERS_START)   return CYAN;     /* Data types      */
    if(idx < C_KW_AGGREGATES_START)  return BLUE;     /* Modifiers       */
    if(idx < C_KW_CONDITIONS_START)  return GREEN;    /* Aggregates      */
    if(idx < C_KW_LOOPS_START)       return YELLOW;   /* Conditions      */
    if(idx < C_KW_JUMPS_START)       return MAGENTA;  /* Loops           */
    if(idx < C_KW_STORAGE_START)     return RED;      /* Jumps           */
    if(idx < C_KW_QUALIFIERS_START)  return BLUE;     /* Storage classes */
    if(idx < C_KEYWORDS_NUM)         return WHITE;    /* Qualifiers + operators */
    return RESET;
}

/** @copydoc syntax_highlight_row */
char *syntax_highlight_row(const char *string){
    if(!syn || !string)
        return NULL;

    switch(syn->lang){
        case LANG_C:
            return syntax_c_highlight_row(string);
        default:
            return NULL;
    }
}

/**
 * @brief  Try to match and highlight a @c \#include directive starting at
 *         position @p i.
 *
 * Colors the directive keyword in BRIGHT_GREEN and the included path in
 * YELLOW.  If a match is found the entire remaining line is consumed.
 *
 * @param[in]     s     Source line.
 * @param[in]     slen  Byte length of @p s.
 * @param[in]     i     Starting byte index.
 * @param[in,out] out   Output buffer pointer.
 * @param[in,out] cap   Output buffer capacity.
 * @param[in,out] olen  Bytes written to @p out so far.
 * @return Number of source bytes consumed, or 0 if no match.
 */
static size_t try_highlight_include(const char *s, size_t slen, size_t i,
                                    char **out, size_t *cap, size_t *olen){
    size_t start = i;

    size_t j = i;
    while(j < slen && (s[j] == ' ' || s[j] == '\t'))
        j++;

    if(j >= slen || s[j] != '#')
        return 0;

    size_t hash_pos = j;
    j++;
    while(j < slen && (s[j] == ' ' || s[j] == '\t'))
        j++;

    if(j + 7 > slen || strncmp(&s[j], "include", 7) != 0)
        return 0;
    /* Word-boundary: avoid matching "#includeFoo", "#includes", etc. */
    if(j + 7 < slen && is_ident_char(s[j + 7]))
        return 0;
    j += 7;

    /* Emit any leading whitespace before '#' uncolored */
    if(start < hash_pos)
        out_append(out, cap, olen, &s[start], hash_pos - start);

    out_append_colored(out, cap, olen, BRIGHT_GREEN, &s[hash_pos], j - hash_pos);

    size_t path_start = j;
    while(j < slen && (s[j] == ' ' || s[j] == '\t'))
        j++;
    if(path_start < j)
        out_append(out, cap, olen, &s[path_start], j - path_start);

    /* Color the path: <...> or "..." */
    if(j < slen && (s[j] == '<' || s[j] == '"')){
        char closer = (s[j] == '<') ? '>' : '"';
        size_t pstart = j;
        j++;
        while(j < slen && s[j] != closer)
            j++;
        if(j < slen)
            j++;
        out_append_colored(out, cap, olen, YELLOW, &s[pstart], j - pstart);
    }

    if(j < slen)
        out_append(out, cap, olen, &s[j], slen - j);

    return slen - start;
}

/**
 * @brief  Try to match and highlight a @c \#define directive starting at
 *         position @p i.
 *
 * Colors the directive keyword in BRIGHT_GREEN. If a match is found the entire remaining line is consumed.
 *
 * @param[in]     s     Source line.
 * @param[in]     slen  Byte length of @p s.
 * @param[in]     i     Starting byte index.
 * @param[in,out] out   Output buffer pointer.
 * @param[in,out] cap   Output buffer capacity.
 * @param[in,out] olen  Bytes written to @p out so far.
 * @return Number of source bytes consumed, or 0 if no match.
 */
static size_t try_highlight_define(const char *s, size_t slen, size_t i,
                                    char **out, size_t *cap, size_t *olen){
    size_t start = i;

    size_t j = i;
    while(j < slen && (s[j] == ' ' || s[j] == '\t'))
        j++;

    if(j >= slen || s[j] != '#')
        return 0;

    size_t hash_pos = j;
    j++;
    while(j < slen && (s[j] == ' ' || s[j] == '\t'))
        j++;

    if(j + 6 > slen || strncmp(&s[j], "define", 6) != 0)
        return 0;
    /* Word-boundary: avoid matching "#defined", "#defineXYZ", etc. */
    if(j + 6 < slen && is_ident_char(s[j + 6]))
        return 0;
    j += 6;

    /* Emit any leading whitespace before '#' uncolored */
    if(start < hash_pos)
        out_append(out, cap, olen, &s[start], hash_pos - start);

    out_append_colored(out, cap, olen, BRIGHT_GREEN, &s[hash_pos], j - hash_pos);

    size_t ws_start = j;
    while(j < slen && (s[j] == ' ' || s[j] == '\t'))
        j++;

    if(ws_start < j)
        out_append(out, cap, olen, &s[ws_start], j - ws_start);

    if(j < slen)
        out_append(out, cap, olen, &s[j], slen - j);

    return slen - start;
}

/** @copydoc syntax_c_highlight_row */
char *syntax_c_highlight_row(const char *string){
    size_t slen = strlen(string);
    size_t cap = slen * 3 + 64;
    char *out = malloc(cap);
    if(!out) return NULL;

    size_t olen = 0;
    size_t i = 0;

    /* Try #include — if it matches, the whole line is consumed */
    size_t inc = try_highlight_include(string, slen, 0, &out, &cap, &olen);
    if(inc > 0){
        out[olen] = '\0';
        return out;
    }

    size_t def = try_highlight_define(string, slen, 0, &out, &cap, &olen);
    if(def > 0){
        out[olen] = '\0';
        return out;
    }

    while(i < slen){
        if(is_ident_char(string[i])){
            int left_ok = (i == 0) || !is_ident_char(string[i - 1]);

            if(left_ok){
                /* Keyword match: compare against the keyword table */
                int matched = 0;
                for(int k = 0; k < C_KEYWORDS_NUM; k++){
                    size_t kwlen = strlen(syn->keywords[k]);
                    if(i + kwlen > slen)
                        continue;
                    if(strncmp(&string[i], syn->keywords[k], kwlen) != 0)
                        continue;
                    if(i + kwlen < slen && is_ident_char(string[i + kwlen]))
                        continue;

                    out_append_colored(&out, &cap, &olen,
                                       syntax_get_keyword_color(k),
                                       &string[i], kwlen);
                    i += kwlen;
                    matched = 1;
                    break;
                }
                if(matched)
                    continue;

                /*
                 * Function-call heuristic: collect the full identifier,
                 * then check if '(' follows (possibly after whitespace).
                 */
                size_t id_start = i;
                while(i < slen && is_ident_char(string[i]))
                    i++;
                size_t id_len = i - id_start;

                size_t peek = i;
                while(peek < slen && (string[peek] == ' ' || string[peek] == '\t'))
                    peek++;

                if(peek < slen && string[peek] == '('){
                    out_append_colored(&out, &cap, &olen,
                                        GREEN, &string[id_start], id_len);
                }else{
                    out_append(&out, &cap, &olen, &string[id_start], id_len);
                }
                continue;
            }
        }

        ensure_capacity(&out, &cap, olen, 1);
        out[olen++] = string[i++];
    }

    out[olen] = '\0';
    return out;
}
