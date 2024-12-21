#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <termios.h>
#include <unistd.h>
#include <stdio.h>

void init_keyboard();
void close_keyboard();
int _kbhit();
int _getch();

#endif