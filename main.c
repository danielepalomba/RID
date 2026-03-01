#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "editor_conf.h"
#include "term.h"

/** @brief Map a key to its Ctrl+key value (clear upper 3 bits). */
#define CTRL_KEY(k) ((k) & 0x1f)

/** @brief ANSI escape sequence to clear the entire screen. */
#define CLR_SCREEN "\x1b[2J"

/** @brief ANSI escape sequence to move the cursor to row 1, column 1. */
#define HOME_CURS "\x1b[H"

/**
* @brief Show usage to user and exit the program.
*/
void prog_failure(){
    fprintf(stderr,"Usage: ./rid <filename> [--nowr]\n");
    exit(EXIT_FAILURE);
}

/**
 * @brief  Entry point for the rid text editor.
 * @param[in] argc  Argument count.
 * @param[in] argv  Argument vector; argv[1] is the optional file to open.
 * @return EXIT_SUCCESS on normal termination.
 */
int main(int argc, char **argv){ 
    
    system("clear");
    editor_init();

    switch(argc){
        case 2: 
            editor_open(argv[1]);
            break;
        case 3: 
            if(strcmp(argv[2], "--nowr") == 0){ //no word wrap
                editor_open(argv[1]);
                editor_set_word_wrap(0);
                break; 
            }else{
                prog_failure();
                break; // in order to have no warnings
            }
        default:
          prog_failure(); 
    }

    set_input_mode();

    editor_refresh_screen();
   
    while(1){
        int c = term_read_key();

        if(c == CTRL_KEY('q')){
            write(STDOUT_FILENO, CLR_SCREEN, 4);
            write(STDOUT_FILENO, HOME_CURS, 3);
            break;
        }else if(c == CTRL_KEY('s')){
            editor_save_file();
            write(STDOUT_FILENO, CLR_SCREEN, 4);
            write(STDOUT_FILENO, HOME_CURS, 3);
            break;
        }else if(c == '\r' || c == '\n'){
            editor_insert_newline();
        }else if(c == BACKSPACE || c == CTRL_KEY('h')){
            editor_del_char(c);
        }else if(c == ARROW_UP || c == ARROW_DOWN || c == ARROW_LEFT || c == ARROW_RIGHT){
            editor_move_cursor(c);
        }else if(c == '\t'){
            editor_insert_tab();
        }else{
            if(c >= 32 && c != 127){
                editor_insert_char(c);
            }
        }
        editor_refresh_screen();
    }
    return EXIT_SUCCESS;
}
