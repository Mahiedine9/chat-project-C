#pragma once
#include <ncurses.h>

#define INPUT_H 5
#define INPUT_WIN_TITLE "Console input"
#define OUTPUT_WIN_TITLE "Messages"
#define INFO_WIN_TITLE "MTC CLIENT: Connection infos"
#define PROMPT "> "

void console_init();
void console_exit();

int console_readline(char *buffer, int max_cars);
void console_output(char *str);
void console_info(char *str);


