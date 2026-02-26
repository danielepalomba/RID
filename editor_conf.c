#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE
#include "editor_conf.h"
#include "term.h"
#include "syntax.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static struct editor_conf ec_storage;
Ec ec = &ec_storage;

/** @copydoc editor_init */
void editor_init(void){
    ec->c_x = 0;
    ec->c_y = 0;
    ec->numrows = 0;
    ec->rowoff = 0;
    ec->coloff = 0;
    ec->row = NULL;
    ec->filename = NULL;

    if(get_terminal_dimension(ec) == -1){
        perror("Could not read terminal dimension");
        exit(EXIT_FAILURE);
    }
}

/** @copydoc editor_insert_row */
void editor_insert_row(int at, const char *s, size_t len){
    if(at < 0 || at > (int) ec->numrows) return;

    ec->row = realloc(ec->row, sizeof(struct rrow) * (ec->numrows + 1));
    memmove(&ec->row[at + 1], &ec->row[at], sizeof(struct rrow) * (ec->numrows - at));

    rrow_create(&ec->row[at], s, len);
    ec->numrows++;
}

/** @copydoc editor_append_row */
void editor_append_row(const char *s, size_t len){
    editor_insert_row(ec->numrows, s, len);
}

/** @copydoc editor_insert_char */
void editor_insert_char(int c){
    if(ec->c_y == (int)ec->numrows)
        editor_append_row("", 0);
    rrow_insert_char(&ec->row[ec->c_y], ec->c_x, (uint32_t)c);
    ec->c_x++;
}

/** @copydoc editor_insert_newline */
void editor_insert_newline(void){
    if(ec->c_x == 0)
        editor_insert_row(ec->c_y, "", 0);
    else{
        Rrow row = &ec->row[ec->c_y];
        size_t byte_off = utf8_byte_offset(row->chars, row->size, ec->c_x);
        editor_insert_row(ec->c_y + 1, &row->chars[byte_off], row->size - byte_off);

        /* Re-fetch: realloc inside editor_insert_row may have moved the array */
        row = &ec->row[ec->c_y];
        row->size = byte_off;
        row->chars[row->size] = '\0';
    }

    ec->c_y++;
    ec->c_x = 0;
}

/** @copydoc editor_insert_tab */
void editor_insert_tab(void){
    if(ec->c_y == (int)ec->numrows)
        editor_append_row("", 0);
    
    int tab_width = 4 - (ec->c_x % 4);
    for(int i = 0; i < tab_width; i++){
        rrow_insert_char(&ec->row[ec->c_y], ec->c_x, ' ');
        ec->c_x++;
    }
}

/** @copydoc editor_del_row */
void editor_del_row(int at){
    if(at < 0 || at >= (int)ec->numrows) return;
    free(ec->row[at].chars);
    memmove(&ec->row[at], &ec->row[at + 1], sizeof(struct rrow) * (ec->numrows - at - 1));
    ec->numrows--;
}

/** @copydoc editor_del_char */
void editor_del_char(int c){
    (void)c;
    /* If cursor is past the last line and at column 0, simply move to the end of the previous line. */
    if(ec->c_y == (int)ec->numrows){
        if(ec->c_x == 0 && ec->c_y > 0){
            ec->c_y--;
            ec->c_x = utf8_codepoint_count(ec->row[ec->c_y].chars, ec->row[ec->c_y].size);
        }
        return;
    }
    
    if(ec->c_x == 0 && ec->c_y == 0) return;

    Rrow row = &ec->row[ec->c_y];

    if(ec->c_x > 0){
        rrow_del_char(row, ec->c_x - 1);
        ec->c_x--;
    }else{
        ec->c_x = utf8_codepoint_count(ec->row[ec->c_y - 1].chars, ec->row[ec->c_y - 1].size);
        
        Rrow prev_row = &ec->row[ec->c_y - 1];
        prev_row->chars = realloc(prev_row->chars, prev_row->size + row->size + 1);
        memcpy(&prev_row->chars[prev_row->size], row->chars, row->size);
        prev_row->size += row->size;
        prev_row->chars[prev_row->size] = '\0';
        
        editor_del_row(ec->c_y);
        ec->c_y--;
    }
}

/** @copydoc editor_move_cursor */
void editor_move_cursor(int key){
    switch(key){
        case ARROW_LEFT:
            if(ec->c_x != 0) ec->c_x--;
            break;
        case ARROW_RIGHT:
            if(ec->c_y < (int)ec->numrows){
                int row_cp = (int)utf8_codepoint_count(
                    ec->row[ec->c_y].chars, ec->row[ec->c_y].size);
                if(ec->c_x < row_cp) ec->c_x++;
            }
            break;
        case ARROW_UP:
            if(ec->c_y != 0) ec->c_y--;
            break;
        case ARROW_DOWN:
            if(ec->c_y < (int)ec->numrows - 1) ec->c_y++;
            break;
    }
}

/**
 * @brief  Truncate an ANSI-colored string to the visible window region.
 *
 * Skips @p skip_cp visible codepoints (horizontal scroll offset), then
 * keeps up to @p max_visible. ANSI escape sequences are passed through
 * but do not count as visible characters.
 *
 * @param[in] src          Null-terminated source string with ANSI escapes.
 * @param[in] skip_cp      Number of visible codepoints to skip (coloff).
 * @param[in] max_visible  Maximum visible codepoints to emit.
 * @return Heap-allocated truncated string (caller frees), or NULL on
 *         allocation failure.
 */
static char *truncate_highlighted(const char *src, int skip_cp, int max_visible){
    size_t slen = strlen(src);
    /* Allocate extra space: each tab can expand to at most 4 spaces */
    char *out = malloc(slen * 4 + 8);
    if(!out) return NULL;

    size_t si = 0;
    size_t oi = 0;
    int vis = 0;       /* visible columns emitted so far (after skip)  */
    int abs_col = 0;   /* absolute visual column (skip + vis)           */

    while(si < slen){
        /* ANSI escape: \033[ ... m — pass through without counting */
        if(src[si] == '\033' && si + 1 < slen && src[si + 1] == '['){
            size_t start = si;
            si += 2;
            while(si < slen && src[si] != 'm')
                si++;
            if(si < slen) si++;

            if(abs_col >= skip_cp){
                size_t seq_len = si - start;
                memcpy(&out[oi], &src[start], seq_len);
                oi += seq_len;
            }
            continue;
        }

        /* Tab expansion: expand to next 4-column tab stop */
        if((unsigned char)src[si] == '\t'){
            int tab_width = 4 - (abs_col % 4);
            for(int s = 0; s < tab_width; s++){
                if(abs_col < skip_cp){
                    abs_col++;
                } else {
                    if(vis >= max_visible) goto done;
                    out[oi++] = ' ';
                    vis++;
                    abs_col++;
                }
            }
            si++;
            continue;
        }

        int cplen = utf8_byte_length((uint8_t)src[si]);
        if(si + (size_t)cplen > slen) cplen = (int)(slen - si);

        if(abs_col < skip_cp){
            abs_col++;
            si += (size_t)cplen;
            continue;
        }

        if(vis >= max_visible)
            break;

        memcpy(&out[oi], &src[si], (size_t)cplen);
        oi += (size_t)cplen;
        si += (size_t)cplen;
        vis++;
        abs_col++;
    }

done:
    /* Ensure colors are reset at end of line */
    memcpy(&out[oi], "\033[0m", 4);
    oi += 4;

    out[oi] = '\0';
    return out;
}

/** @copydoc editor_draw_rows */
void editor_draw_rows(Abuf ab){
    for(int i = 0; i < (int)ec->window_height; i++){
        int filerow = i + ec->rowoff;

        if(filerow >= (int)ec->numrows){
            ab_append(ab, "~", 1);
        }else{
            const char *row_str = ec->row[filerow].chars;
            char *highlighted = syntax_highlight_row(row_str);

            if(highlighted){
                char *truncated = truncate_highlighted(highlighted,
                                                       ec->coloff,
                                                       ec->window_width);
                free(highlighted);
                if(truncated){
                    ab_append(ab, truncated, (int)strlen(truncated));
                    free(truncated);
                }
            }else{
                /* Fallback: coloff/width clipping with tab expansion */
                const char *src   = ec->row[filerow].chars;
                size_t      ssize = ec->row[filerow].size;
                int vis_col = 0;   /* visual column on screen (after coloff)  */
                int skipped = 0;   /* visual columns consumed before coloff   */

                for(size_t bi = 0; bi < ssize; ){
                    if((unsigned char)src[bi] == '\t'){
                        /* Tab stops every 4 columns (absolute) */
                        int abs_col   = skipped + vis_col;
                        int tab_width = 4 - (abs_col % 4);

                        for(int s = 0; s < tab_width; s++){
                            if(skipped < ec->coloff){
                                skipped++;
                            } else {
                                if(vis_col >= ec->window_width) goto row_done;
                                ab_append(ab, " ", 1);
                                vis_col++;
                            }
                        }
                        bi++;
                    } else {
                        int cplen = utf8_byte_length((uint8_t)src[bi]);
                        if(bi + (size_t)cplen > ssize)
                            cplen = (int)(ssize - bi);

                        if(skipped < ec->coloff){
                            skipped++;
                        } else {
                            if(vis_col >= ec->window_width) goto row_done;
                            ab_append(ab, &src[bi], cplen);
                            vis_col++;
                        }
                        bi += (size_t)cplen;
                    }
                }
                row_done:;
            }
        }
        ab_append(ab, "\x1b[K", 3);
        if(i < (int)ec->window_height - 1)
            ab_append(ab, "\r\n",2);
    }
 }


/**
 * @brief  Compute the visual (screen) column for codepoint index @p cx
 *         in the current row, expanding tab characters to 4-column stops.
 * @param[in] cx  Codepoint index into the current row (i.e., ec->c_x).
 * @return Visual column (0-based).
 */
static int cx_to_visual_col(int cx){
    if(ec->c_y >= (int)ec->numrows) return cx;

    const char *s    = ec->row[ec->c_y].chars;
    size_t      slen = ec->row[ec->c_y].size;
    int vcol = 0;
    int cp   = 0;
    size_t bi = 0;

    while(bi < slen && cp < cx){
        if((unsigned char)s[bi] == '\t'){
            vcol += 4 - (vcol % 4);
            bi++;
        } else {
            int clen = utf8_byte_length((uint8_t)s[bi]);
            if(bi + (size_t)clen > slen) break;
            bi += (size_t)clen;
            vcol++;
        }
        cp++;
    }
    return vcol;
}

/** @copydoc editor_scroll */
void editor_scroll(void){
    int vcol = cx_to_visual_col(ec->c_x);

    if(ec->c_y < ec->rowoff)
        ec->rowoff = ec->c_y;

    if(ec->c_y >= ec->rowoff + ec->window_height)
        ec->rowoff = ec->c_y - ec->window_height + 1;

    if(vcol < ec->coloff)
        ec->coloff = vcol;

    if(vcol >= ec->coloff + ec->window_width)
        ec->coloff = vcol - ec->window_width + 1;
}

/** @copydoc editor_refresh_screen */
void editor_refresh_screen(void) {
    editor_scroll();

    struct abuf ab = ABUF_INIT;

    ab_append(&ab, "\x1b[?25l", 6);
    ab_append(&ab, "\x1b[H", 3);

    editor_draw_rows(&ab);

    char buf[32];
    int vcol = cx_to_visual_col(ec->c_x);
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (ec->c_y - ec->rowoff) + 1, (vcol - ec->coloff) + 1);

    ab_append(&ab, buf, strlen(buf));
    ab_append(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len);
    ab_free(&ab);
}

/** @copydoc editor_save_file */
int editor_save_file(){

    if(ec->filename == NULL)
        return -1;

    FILE *fd = fopen(ec->filename, "w");

    if(fd == NULL)
        return -1;

    for(unsigned int i = 0; i < ec->numrows; i++){
        fputs(ec->row[i].chars, fd);
        fputs("\n", fd);
    }

    fclose(fd);
    return 0;
}

/** @copydoc editor_open */
void editor_open(const char *filename){
    ec->filename = strdup(filename);

    /* Detect file extension to enable language-specific highlighting */
    const char *dot = strrchr(filename, '.');
    if(dot)
        syntax_set_ext(dot);

    FILE *fp = fopen(filename,"r");

    if(!fp)
        return;

    char *line = NULL;
    size_t linecap = 0;
    size_t linelen;

    while((int)(linelen = getline(&line, &linecap, fp)) != -1){
        while(linelen > 0 && (line[linelen-1] == '\n' || line[linelen-1] == '\r')){
            linelen--;
        }

        editor_append_row(line,linelen);
    }
    free(line);
    fclose(fp);
}
