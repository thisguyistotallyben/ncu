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
#include <cstdarg>
#include "ncu.h"

using namespace std;


// setup and cleanup -----------------------------------------------------------------

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

void NCU::hideCursor() {
    cursor = false;
    curs_set(0);
}

void NCU::end() {
    map<string, Element*>::iterator eit;
    
    // clean up elements
    for (eit = elementList.begin(); eit != elementList.end(); eit++) {
        delete eit->second;
    }

    endwin();
}

// element creation and manipulation -------------------------------------------------

// addElement
// creates a new element and adds it to the element list
void NCU::addElement(string id, borderType bt, int sizex, int sizey, int posx, int posy) {
    Element *e;

    // error checking
    check_if_started();

    // setup
    e = new Element;

    // fill in info
    e->id = id;
    e->title = "";
    e->btype = bt;
    e->width = sizex;
    e->win = newwin(sizey, sizex, posy, posx);
    wrefresh(e->win);
    e->panel = new_panel(e->win);
    hide_panel(e->panel);

    // add to map
    elementList.insert(make_pair(id, e));

    // make the proper borders
    switch(e->btype) {
        case NCU_BORDERLESS_BOX:
            wborder(e->win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
            break;
        case NCU_BORDER_BOX:
            box(e->win, 0, 0);
            break;
        case NCU_BASIC_INPUT:
            wborder(e->win, ' ', ' ', ' ', 0, ' ', ' ', 0, 0);
            break;
        default: break;
    }
}

// borderElement
// puts a border on an element after creation
// NOTE: shows element even if not visible
void NCU::borderElement(string id, borderType bt) {
    Element *e = getElement(id);

    if (e != NULL) {
        if (bt != NCU_NO_CHANGE) e->btype = bt;

        switch(e->btype) {
            case NCU_BORDERLESS_BOX:
                wborder(e->win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
                break;
            case NCU_BORDER_BOX:
                box(e->win, 0, 0);
                break;
            case NCU_BASIC_INPUT:
                wborder(e->win, ' ', ' ', ' ', 0, ' ', ' ', 0, 0);
                break;
            default: break;
        }
    }

    // add title back if neccessary
    if (e->title != "") addTitle(id, e->title);

    // update
    update_panels();
    doupdate();
}

// updateElement
// pretty much the same thing as showElement
// probably useless, idk
void NCU::updateElement(string id) {
    wrefresh(getWin(id));
}

// addTitle
// adds a title to an element
// TODO: maybe add possible positons
void NCU::addTitle(string id, string title) {
    (getElement(id))->title = title;

    if (title != "") {
        title = " " + title + " ";
        this->write(id, title, 1, 0);
    }
    else borderElement(id, NCU_NO_CHANGE);
}

// clearElement
// clears all the data in an element except the title
void NCU::clearElement(string id) {
    WINDOW *win;
    win = getWin(id);

    werase(win);
    borderElement(id, NCU_NO_CHANGE);
}

// write
// writes to an element
// not to be confused with the real write
// that'll probably come back and bite me in the butt
void NCU::write(string id, string data, int posx, int posy) {
    WINDOW *win = getWin(id);
    if (win != NULL) mvwprintw(win, posy, posx, data.c_str());
    
    // update
    update_panels();
    doupdate();
}

// read
// reads from an element
// not to be confused with the real read
// also will probably bite me in the butt later
// TODO: add to only read from valid input boxes
string NCU::read(string id) {
    char c;
    string s = "", ns;
    WINDOW *win;

    // setup
    win = getWin(id);
    Element *e = getElement(id);
    cbreak();
    if(!cursor) curs_set(1);
    wmove(win, 1, 2);
    
    while (1) {
        c = wgetch(win);
        switch(c) {
            case 10: // enter
                //clearWin(name);
                curs_set(0);
                return s;
            default:
                s = s + c;
        }

        clearElement(id);
        
        // makes sure the string does not exceed the box size
        if (s.size() > e->width-4) ns = s.substr(s.size()-(e->width-4-1), s.size()-1);
        else ns = s;

        // write to box
        this->write(id, ns, 2, 1);
    }

    
    if (!cursor) curs_set(0);
}

// showElement
// initially shows an element
void NCU::showElement(string id) {
    PANEL *p = getPanel(id);
    if (p != NULL) show_panel(p);

    // update
    update_panels();
    doupdate();
}

// wait
// just a while loop waiting for a key
// i'm not completely sure y tho
void NCU::wait(char key) {
    char c;
    while (c = getch() != key);
}

// check_if_started()
// checks if ncu has been started
void NCU::check_if_started() {
    if (!NCU_STARTED) {
        cout << "NCU NOT INTIALIZED. YOU MUST CALL start()\n";
        exit(1);
    }
}

// group functions -------------------------------------------------------------------

void NCU::addGroup(string id, int num, ...) {
    va_list vl;
    string s;
    Group *g;
    va_start(vl, num);

    // setup
    g = new Group;

    // build group names
    for (int i = 0; i < num; i++) {
        s = va_arg(vl, char*);
        g->elements.push_back(s);
    }

    // insert
    groupList.insert(make_pair(id, g));
}

void NCU::show(string id) {
    map<string, Group*>::iterator git;
    PANEL *p;

    // get the group
    git = groupList.find(id);

    // show the group
    if (git != groupList.end()) {
        for (int i = 0; i < git->second->elements.size(); i++) {
            p = getPanel(git->second->elements[i]);
            if (p != NULL) show_panel(p);
        }
    }

    // update
    update_panels();
    doupdate();
}

// utilitarian crap ------------------------------------------------------------------

int NCU::width() {
    return c;
}

int NCU::height() {
    return r;
}

WINDOW* NCU::getWin(string id) {
    map<string, Element*>::iterator e;

    e = elementList.find(id);
    if (e != elementList.end()) return e->second->win;
    else return NULL;
}

PANEL* NCU::getPanel(string id) {
    map<string, Element*>::iterator e;

    e = elementList.find(id);
    if (e != elementList.end()) return e->second->panel;
    else return NULL;
}

Element* NCU::getElement(string id) {
    map<string, Element*>::iterator e;
    e = elementList.find(id);
    if (e != elementList.end()) return e->second;
    else return NULL;
}
