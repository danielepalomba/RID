#ifndef SCOLOR_H
#define SCOLOR_H

/**
 * @brief ANSI color codes used for terminal output.
 */
typedef enum {
    RESET,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    BRIGHT_GREEN
} ColorCode;

/** @brief Lookup table mapping ColorCode values to ANSI escape sequences. */
extern const char *COLOR_SEQ[];

/**
 * @brief  Return the ANSI escape sequence for a given color.
 * @param[in] color  The color code to look up.
 * @return Pointer to a null-terminated ANSI escape string.
 */
const char *scolor_get_seq(ColorCode color);

/**
 * @brief  Print a string to stdout wrapped in ANSI color codes.
 * @param[in] color  Color to apply.
 * @param[in] text   Null-terminated string to print.
 */
void print_colored(ColorCode color, const char *text);

#endif
