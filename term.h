#ifndef TERM_H
#define TERM_H

#include <termios.h>
#include <sys/ioctl.h>

struct editor_conf;

/**
 * @brief Special key codes returned by term_read_key().
 *
 * Values start at 1000 to avoid collisions with printable ASCII.
 */
enum editor_key {
    BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN
};

/**
 * @brief  Restore the original terminal attributes saved at startup.
 * @note   Registered with atexit() by set_input_mode().
 */
void reset_input_mode(void);

/**
 * @brief  Put the terminal into raw mode for the editor.
 * @warning Exits the process if stdin is not a terminal.
 */
void set_input_mode(void);

/**
 * @brief  Query the terminal size and store it in the editor config.
 * @param[out] ec  Editor configuration to populate with width/height.
 * @return 0 on success, -1 on failure.
 */
int get_terminal_dimension(struct editor_conf *ec);

/**
 * @brief  Read a single keypress or escape sequence from stdin.
 * @return ASCII code, Unicode codepoint, or an editor_key constant.
 */
int term_read_key(void);

#endif
