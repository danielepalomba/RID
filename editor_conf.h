#ifndef EDITOR_CONF_H
#define EDITOR_CONF_H

#include <stddef.h>
#include "rrow.h"
#include "abuf.h"

/**
 * @brief Global editor state: cursor, viewport, file content, and window size.
 */
struct editor_conf{
    int c_x;              /**< Cursor column (codepoint index).          */
    int c_y;              /**< Cursor row (zero-based line number).      */
    size_t numrows;       /**< Total number of rows in the file.         */
    int rowoff;           /**< First visible row (vertical scroll).      */
    int coloff;           /**< First visible column (horizontal scroll). */
    struct rrow *row;     /**< Array of text rows.                       */
    char *filename;       /**< Currently open filename (heap-allocated). */
    int window_width;     /**< Terminal width in columns.                */
    int window_height;    /**< Terminal height in rows.                  */
    unsigned short word_wrap; /**< Flag for word wrapping.               */
};

typedef struct editor_conf *Ec;

/** @brief Global editor configuration instance. */
extern Ec ec;

/**
 * @brief  Initialize the editor state and query the terminal size.
 */
void editor_init(void);

/**
 * @brief  Set the word-wrap mode.
 * @param[in] flag  1 to enable word wrap, 0 to disable. Any other value
 *                  resets to the default (enabled).
 */
void editor_set_word_wrap(unsigned short flag);

/**
 * @brief  Move the cursor to an absolute position.
 * @param[in] x  Target codepoint column (zero-based). Clamped to the row length.
 * @param[in] y  Target row index (zero-based). Must be <= numrows.
 * @note   Has no effect if @p x or @p y is negative or out of bounds.
 */
void editor_set_cursor(int x, int y);

/**
 * @brief  Read the current cursor position.
 * @param[out] cords  Two-element array; cords[0] = c_x, cords[1] = c_y.
 */
void editor_get_cursor(int *cords);

/**
 * @brief  Draw the status bar (last terminal row) into the append buffer.
 * @param[in,out] ab  Append buffer that accumulates terminal output.
 */
void editor_draw_status_bar(Abuf ab);

/**
 * @brief  Draw the header bar (first terminal row) into the append buffer.
 *
 * Displays the editor name and version string, centred in the terminal width.
 *
 * @param[in,out] ab  Append buffer that accumulates terminal output.
 */
void editor_draw_header_bar(Abuf ab);

/**
 * @brief  Insert a new row at position @p at, shifting subsequent rows down.
 * @param[in] at   Zero-based index where the row is inserted.
 * @param[in] s    Source bytes for the new row.
 * @param[in] len  Byte length of @p s.
 */
void editor_insert_row(int at, const char *s, size_t len);

/**
 * @brief  Append a row at the end of the file.
 * @param[in] s    Source bytes for the new row.
 * @param[in] len  Byte length of @p s.
 */
void editor_append_row(const char *s, size_t len);

/**
 * @brief  Delete the row at position @p at, shifting subsequent rows up.
 * @param[in] at   Zero-based index of the row to delete.
 */
void editor_del_row(int at);

/**
 * @brief  Insert a character at the current cursor position.
 * @param[in] c  Unicode codepoint to insert.
 */
void editor_insert_char(int c);

/**
 * @brief  Insert a newline at the current cursor position, splitting the row.
 */
void editor_insert_newline(void);

/**
 * @brief  Insert a tab at the current cursor position.
 *
 * Expands the tab to the next 4-column tab stop using space characters.
 */
void editor_insert_tab(void);

/**
 * @brief  Delete the character immediately before the cursor (backspace).
 *
 * If the cursor is at column 0, the current row is merged into the previous
 * one and the row count decreases by one.
 *
 * @param[in] c  Reserved for future use; pass 0 or the key code.
 */
void editor_del_char(int c);

/**
 * @brief  Move the cursor by one word forward or backward in the current row.
 *
 * Forward (direction = 1): skips the current word then leading whitespace,
 * landing on the first character of the next word.
 * Backward (direction = -1): skips whitespace leftward then the preceding
 * word, landing on that word's first character.
 *
 * @param[in] direction  1 = forward (Ctrl+Right), -1 = backward (Ctrl+Left).
 */
void editor_skip_word(int direction);

/**
 * @brief  Move the cursor in the direction indicated by @p key.
 * @param[in] key  One of ARROW_LEFT, ARROW_RIGHT, ARROW_UP, ARROW_DOWN.
 */
void editor_move_cursor(int key);

/**
 * @brief  Render all visible text rows into the append buffer.
 *
 * Applies syntax highlighting and horizontal scrolling (coloff).
 * Rows beyond the file content are rendered as '~'.
 *
 * @param[in,out] ab  Append buffer that accumulates terminal output.
 */
void editor_draw_rows(Abuf ab);

/**
 * @brief  Adjust the scroll offsets so the cursor remains inside the viewport.
 *
 * Updates rowoff and coloff as needed to keep (c_x, c_y) visible.
 */
void editor_scroll(void);

/**
 * @brief  Redraw the entire screen.
 *
 * Calls editor_scroll(), then renders the header, text rows, and status bar
 * into a single append buffer before flushing it to stdout in one write.
 * Finally repositions the terminal cursor to match (c_x, c_y).
 */
void editor_refresh_screen(void);

/**
 * @brief  Open a file and load its contents into the editor row array.
 *
 * Also detects the file extension and activates the appropriate
 * syntax highlighter via syntax_set_ext().
 *
 * @param[in] filename  Path of the file to open.
 */
void editor_open(const char *filename);

/**
 * @brief  Write the current editor content back to the open file.
 *
 * Each row is written followed by a newline character.
 *
 * @return 0 on success, -1 if no filename is set or the file cannot be opened.
 */
int editor_save_file(void);

#endif
