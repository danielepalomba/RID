#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "editor_conf.h"
#include "term.h"

struct termios saved_attributes;

/** @copydoc reset_input_mode */
void reset_input_mode(void){
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);    
}

/** @copydoc set_input_mode */
void set_input_mode(void){
    struct termios attr;
    if(!isatty(STDIN_FILENO)){
        fprintf(stderr, "Not a terminal.\n");
        exit(EXIT_FAILURE);
    }

    tcgetattr(STDIN_FILENO, &saved_attributes);
    atexit(reset_input_mode);

    tcgetattr(STDIN_FILENO, &attr);
    attr.c_iflag &= ~(IXON);
    attr.c_lflag &= ~(ICANON | ECHO);
    attr.c_cc[VMIN] = 1;
    attr.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &attr);
}

/** @copydoc get_terminal_dimension */
int get_terminal_dimension(struct editor_conf *ec){
    struct winsize ws;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, (char*) &ws) == -1 || ws.ws_col == 0)
        return -1;
    else{
        ec->window_width = ws.ws_col;
        ec->window_height = ws.ws_row;
        return 0;
    }
}

/** @copydoc term_read_key */
int term_read_key(void){
    int nread;
    unsigned char c;

    while((nread = read(STDIN_FILENO, &c, 1)) != 1){
        if(nread == -1) exit(EXIT_FAILURE);    
    }

    /* VT100 escape sequences: ESC [ <letter> maps to arrow keys.
     *
     * Make stdin temporarily non-blocking so that reads of the bytes
     * following ESC return immediately (EAGAIN) if nothing is available.
     * This avoids blocking forever on a lone ESC without touching termios
     * (which would risk re-enabling ECHO/ICANON and printing raw bytes).
     */
    if(c == '\x1b'){
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

        /* Longest sequence we handle: ESC [ 1 ; 5 C  (6 bytes total,
         * 5 after ESC – but we already consumed ESC, so read up to 5) */
        char seq[5];
        int got = 0;
        for(int i = 0; i < 5; i++){
            ssize_t n = read(STDIN_FILENO, &seq[i], 1);
            if(n != 1) break;
            got++;
        }

        fcntl(STDIN_FILENO, F_SETFL, flags); /* restore blocking mode */

        if(got == 0) return '\x1b';

        if(seq[0] == '[' && got >= 2){
            /* Ctrl+Arrow: ESC [ 1 ; 5 <letter>  (got == 5) */
            if(got >= 5 && seq[1] == '1' && seq[2] == ';' && seq[3] == '5'){
                switch(seq[4]){
                    case 'C': return CTRL_ARROW_RIGHT;
                    case 'D': return CTRL_ARROW_LEFT;
                }
            }
            /* Plain arrows: ESC [ <letter> */
            switch(seq[1]){
                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                case 'C': return ARROW_RIGHT;
                case 'D': return ARROW_LEFT;
                /* Delete key: ESC [ 3 ~ */
                case '3':
                    if(got >= 3 && seq[2] == '~')
                        return BACKSPACE;
                    break;
            }
        }
        return '\x1b';
    }

    /* Multi-byte UTF-8: read remaining continuation bytes */
    if(c >= 0x80){
        int total = utf8_byte_length(c);
        unsigned char buf[4];
        buf[0] = c;

        for(int i = 1; i < total; i++){
            if(read(STDIN_FILENO, &buf[i], 1) != 1)
                return (int)c;
        }

        uint32_t cp;
        utf8_decode((const char *)buf, &cp);
        return (int)cp;
    }

    return (int)c;
}
