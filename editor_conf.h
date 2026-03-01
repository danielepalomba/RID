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
 * @bried Setter function for word_wrap field in editor_conf
*/
void editor_set_word_wrap(unsigned short flag);

/**
 * @brief  Draw the status bar (last terminal row) into the append buffer.
 * @param[in,out] ab  Append buffer that accumulates terminal output.
 */
void editor_draw_status_bar(Abuf ab);

/**
 * @brief Draw a headebar into the appended buffer, showing the name of the editor and actual version.
 * @param[in,out] ab Append buffer that accumulates terminal output.
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
 * @brief Insert a tab at the current cursor position. 
 */
void editor_insert_tab(void);

/**
 * @brief  Delete the character before the cursor.
 * @param[in] c  Unused (reserved for future direction parameter).
 */
void editor_del_char(int c);

/**
 * @brief  Move the cursor in the direction indicated by @p key.
 * @param[in] key  One of ARROW_LEFT, ARROW_RIGHT, ARROW_UP, ARROW_DOWN.
 */
void editor_move_cursor(int key);

/**
 * @brief  Render all visible rows into the append buffer.
 * @param[in,out] ab  Append buffer that accumulates terminal output.
 */
void editor_draw_rows(Abuf ab);

/**
 * @brief  Adjust rowoff/coloff so the cursor stays inside the viewport.
 */
void editor_scroll(void);

/**
 * @brief  Redraw the entire screen (scroll → render → reposition cursor).
 */
void editor_refresh_screen(void);

/**
 * @brief  Open a file and load its contents into the editor row array.
 * @param[in] filename  Path of the file to open.
 */
void editor_open(const char *filename);

/**
 * @brief  Write the current editor content back to the open file.
 * @return 0 on success, -1 if no filename is set or the file cannot be opened.
 */
int editor_save_file(void);

#endif
