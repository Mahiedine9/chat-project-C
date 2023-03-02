#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"

static WINDOW *input_win = NULL;
static WINDOW *output_win = NULL;
static WINDOW *info_win = NULL;

static int out_height, out_width, out_pos_x, out_pos_y;
static int in_height, in_width, in_pos_x, in_pos_y;
static int info_height, info_width, info_pos_x, info_pos_y;

void console_exit()
{
    if (input_win) delwin(input_win);
    if (output_win) delwin(output_win);
    if (info_win) delwin(info_win);
    endwin();
}

void console_show(WINDOW *w, char *title, int border)
{
    int x, y;
    
    // save current position 
    getyx(w, y, x);
    if (border) box(w, 0, 0);
    wmove(w, 0, 4);
    wprintw(w, "%s", title);
    // restore position 
    wmove(w, y, x);
    wrefresh(w);
}

void console_printc(WINDOW *w, char c)
{
    wprintw(input_win, "%c", c);
    wrefresh(w);
}

void console_printline(WINDOW *w, char *s)
{
    int x, y;
    // get current position 
    getyx(w, y, x);
    mvwprintw(w, y, 1, "%s\n", s);
}

void console_prints(WINDOW *w, char *s)
{
    int x, y;
    // get current position 
    getyx(w, y, x);
    mvwprintw(w, y, 1, "%s", s);
}


static void console_calculate_sizes()
{
    info_height = 3;
    info_width = COLS;

    in_height = INPUT_H + 2;
    in_width  = COLS;
    out_height = LINES - in_height;
    out_width = COLS;

    info_pos_x = 0;
    info_pos_y = 0;
    out_pos_x = 0;
    out_pos_y = info_pos_y + info_height;
    in_pos_x  = 0;
    in_pos_y  = out_pos_y + out_height;
}


void console_init()
{
    initscr();
    atexit(console_exit);
    
    // I leave 5 lines for the input and the rest for the output.
    console_calculate_sizes();
    
    // create the three windows
    info_win = newwin(info_height, info_width, info_pos_y, info_pos_x);
    input_win = newwin(in_height, in_width, in_pos_y, in_pos_x);
    noecho();
    //cbreak();
    raw();

    output_win = newwin(out_height, out_width, out_pos_y, out_pos_x);
    scrollok(output_win, TRUE);
    scrollok(input_win, TRUE);
    wmove(info_win, 1, 1);
    wmove(input_win, 1, 1);
    wmove(output_win, 1, 1);
    console_show(info_win, INFO_WIN_TITLE, 1);
    console_show(input_win, INPUT_WIN_TITLE, 0);
    console_show(output_win, OUTPUT_WIN_TITLE, 1);
}


int console_readline(char *buffer, int max_buf_sz)
{
    int n = 0;
    console_prints(input_win, "> ");
    wrefresh(input_win);
    int ch = wgetch(input_win);
    while (ch != '\n' && n < (max_buf_sz - 1)) {
        if (ch == KEY_BACKSPACE || ch == KEY_DC || ch == 127) {
            if(n > 0){
                n--;
                buffer[n] = 0;
                //wdelch(input_win);
                int x,y;
                getyx(input_win, y, x);
                mvwprintw(input_win, y, 1, "> %s ", buffer);
                getyx(input_win, y, x);
                wmove(input_win, y, x-1);
                wrefresh(input_win);
            }
        }
        else if (ch == KEY_RESIZE) {
            console_calculate_sizes();
            wresize(info_win, info_height, info_width);
            mvwin(info_win, info_pos_y, info_pos_x);
            console_show(info_win, INFO_WIN_TITLE, 1);

            wresize(input_win, in_height, in_width);
            mvwin(input_win, in_pos_y, in_pos_x);
            console_show(input_win, INPUT_WIN_TITLE, 0);
            wrefresh(input_win);
            
            wresize(output_win, out_height, out_width);
            mvwin(output_win, out_pos_y, out_pos_x);
            console_show(output_win, OUTPUT_WIN_TITLE, 1);
            wrefresh(output_win);
        }
        else if (strcmp(unctrl(ch), "^C") == 0) {
            return -1;
        }
        else {
            buffer[n++] = ch;
            // echo the character
            waddch(input_win, ch);
            wrefresh(input_win);
        }
        ch = wgetch(input_win);
    }
    buffer[n] = 0;
    int x, y;
    getyx(input_win, y, x);
    mvwprintw(input_win, y, 1, PROMPT "%s\n", buffer);
    wrefresh(input_win);
    return n;
}

void console_output(char *str)
{
    console_printline(output_win, str);
    console_show(output_win, OUTPUT_WIN_TITLE, 1);
}

void console_info(char *str)
{
    console_printline(info_win, str);
    console_show(info_win, INFO_WIN_TITLE, 1);
}
