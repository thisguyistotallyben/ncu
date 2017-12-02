// Author:  Benjamin Johnson
// Date:    2 December, 2017
// Version: 0.2
// Purpose: Provide an abstraction over ncurses to aid in
//          relatively simple command line inteface creation


#include <iostream>
#include <vector>
#include <map>
#include <ncurses.h>
#include "ncu.h"

using namespace std;


// NCU -----------------------------------------------------------------

void NCU::start() {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    noecho();
    refresh();
}

void NCU::addElement(string name, elementType type,
                int sizex, int sizey,
                int posx, int posy) {

}

void NCU::end() {
    endwin();
}
