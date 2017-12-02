// Author:  Benjamin Johnson
// Date:    2 December, 2017
// Version: 0.2
// Purpose: Provide an abstraction over ncurses to aid in
//          relatively simple command line inteface creation


#include <iostream>
#include <cstdio>
#include <vector>
#include <map>
#include <ncurses.h>
#include <panel.h>
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
    NCU_STARTED = true;
    r = LINES;
    c = COLS;
}

void NCU::addElement(string id, elementType type, int sizex, int sizey, int posx, int posy) {
    check_if_started();

    Element *e;
    
    e = new Element;

    // fill in info
    e->etype = type;

    // add to map
    elementList.insert(make_pair(id, e));
}

void NCU::write(string id, string data, int posx, int posy) {
    WINDOW *win = getWin(id);
    if (win != NULL) mvwprintw(win, posx, posy, data.c_str());
}

void showElement(string id) {
    show_win(getWin(id));
}

void NCU::end() {
    endwin();
}

void NCU::check_if_started() {
    if (!NCU_STARTED) {
        cout << "NCU NOT INTIALIZED. YOU MUST CALL start()\n";
        exit(1);
    }
}

// utilitarian crap -----------------------------------------------

int NCU::cols() {
    return c;
}

int NCU::rows() {
    return r;
}

WINDOW* NCU::getWin(string id) {
    map<string, Element*>::iterator e;

    e = elementList.find(id);
    if (e != elementList.end()) return e->second->win;
    else return NULL;
}
