#ifndef SYNTAX_H
#define SYNTAX_H

#include "scolor.h"

/** @brief File extension for C source files. */
#define EXT_C ".c"
/** @brief File extension for C header files. */
#define EXT_H ".h"

/**
 * @brief C keyword table (44 entries, ordered by category).
 *
 * Index ranges map to keyword categories — see the C_KW_*_START macros.
 */
#define C_KEYWORDS { \
    /* Data types (indices 0-7) */ \
    "char", "int", "float", "double", "void", "_Bool", "_Complex", "_Imaginary", \
    \
    /* Size/sign modifiers (indices 8-11) */ \
    "short", "long", "signed", "unsigned", \
    \
    /* Aggregates and user types (indices 12-15) */ \
    "struct", "union", "enum", "typedef", \
    \
    /* Conditions and choices (indices 16-20) */ \
    "if", "else", "switch", "case", "default", \
    \
    /* Loops and loop control (indices 21-25) */ \
    "for", "while", "do", "break", "continue", \
    \
    /* Direct jumps (indices 26-27) */ \
    "goto", "return", \
    \
    /* Storage classes (indices 28-32) */ \
    "auto", "extern", "register", "static", "_Thread_local", \
    \
    /* Qualifiers and function specifiers (indices 33-38) */ \
    "const", "volatile", "restrict", "inline", "_Atomic", "_Noreturn", \
    \
    /* Operators and C11 introspection (indices 39-43) */ \
    "sizeof", "_Alignas", "_Alignof", "_Generic", "_Static_assert" \
}

/** @brief Total number of C keywords in the table. */
#define C_KEYWORDS_NUM 44

/** @name Keyword category start indices
 *  Each macro marks the first index of a keyword category inside C_KEYWORDS.
 *  @{
 */
#define C_KW_TYPES_START       0
#define C_KW_MODIFIERS_START   8
#define C_KW_AGGREGATES_START 12
#define C_KW_CONDITIONS_START 16
#define C_KW_LOOPS_START      21
#define C_KW_JUMPS_START      26
#define C_KW_STORAGE_START    28
#define C_KW_QUALIFIERS_START 33
#define C_KW_OPERATORS_START  39
/** @} */

/**
 * @brief Supported programming languages.
 */
typedef enum {
    LANG_C
    /* Future: LANG_PYTHON, LANG_JS, ... */
} LangType;

/**
 * @brief Language-specific syntax definition.
 */
typedef struct {
    LangType lang;                        /**< Active language identifier. */
    const char *keywords[C_KEYWORDS_NUM]; /**< Keyword lookup table.      */
} Syntax;

/**
 * @brief  Select the active syntax based on a file extension.
 * @param[in] ext  File extension string (e.g. ".c", ".h").
 */
void syntax_set_ext(const char *ext);

/**
 * @brief  Highlight a row of source code using the active syntax.
 * @param[in] string  Null-terminated row text.
 * @return Heap-allocated highlighted string (caller frees), or NULL if no
 *         syntax is active or @p string is NULL.
 */
char *syntax_highlight_row(const char *string);

/**
 * @brief  Highlight a single row of C source code.
 * @param[in] string  Null-terminated row text.
 * @return Heap-allocated highlighted string (caller frees), or NULL on
 *         allocation failure.
 * @note   This is the C-specific backend; prefer syntax_highlight_row()
 *         for language-agnostic usage.
 */
char *syntax_c_highlight_row(const char *string);

/**
 * @brief  Map a keyword index to its corresponding highlight color.
 * @param[in] kw_index  Zero-based index into the C_KEYWORDS table.
 * @return ColorCode for the keyword's category, or RESET if out of range.
 */
ColorCode syntax_get_keyword_color(int kw_index);

#endif
