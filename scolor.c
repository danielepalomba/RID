#include "scolor.h"
#include <stdio.h>

const char *COLOR_SEQ[] = {
    "\033[0m",  /* RESET        */
    "\033[31m", /* RED          */
    "\033[32m", /* GREEN        */
    "\033[33m", /* YELLOW       */
    "\033[34m", /* BLUE         */
    "\033[35m", /* MAGENTA      */
    "\033[36m", /* CYAN         */
    "\033[37m", /* WHITE        */
    "\033[92m"  /* BRIGHT_GREEN */
};

/** @copydoc scolor_get_seq */
const char *scolor_get_seq(ColorCode color){
    return COLOR_SEQ[color];
}

/**
 * @brief  Print a string to stdout wrapped in ANSI color codes.
 * @param[in] color  Color to apply.
 * @param[in] text   Null-terminated string to print.
 */
void print_colored(ColorCode color, const char *text){
    printf("%s%s%s", COLOR_SEQ[color], text, COLOR_SEQ[RESET]);
}
