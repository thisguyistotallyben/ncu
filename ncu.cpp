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
#include <stdio.h>
#include <panel.h>
#include <cstdarg>
#include <algorithm>
#include "ncu.h"

using namespace std;


// IDEA CENTRAL
// TODO: make subelements for form filling or something
// TODO: possibly be able to link subelements together
// TODO: add rightOf(), leftOf(), above(), below() functions for positioning


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

// alert
int NCU::alert(string s) {
	int c;
	int i, next;
	int nl = count(s.begin(), s.end(), '\n');
	nl++;

	alertw = newwin(nl + 2, COLS, LINES/2 - (nl + 2)/2, 0);
	alertp = new_panel(alertw);

	// setup
	wbkgd(alertw, COLOR_PAIR(1));
	
	for (int i = 0; i < nl; i++) {
		next = s.find('\n');
		if (next != -1) s[next] = '\0';
		else next = s.size();

		mvwprintw(alertw, i + 1, COLS/2 - next/2, s.c_str());
		s = s.erase(0, next+1);
	}

	// show panel
	show_panel(alertp);
	
    // update
    update_panels();
    doupdate();
	
	// wait to kill
	c = getch();

	// hide
	hide_panel(alertp);
    // update
    update_panels();
    doupdate();

	return c;
}

// sub element things ----------------------------------------------------------------

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

    // dimensions
    e->sizex = sizex;
    e->sizey = sizey;
    e->posx = posx;
    e->posy = posy;

    // win creation
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
void NCU::cwrite(string id, string data, int posx, int posy) {
	clearElement(id);
	write(id, data, posx, posy);
}

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

// TODO: FIX THIS: IT IS WEIRD AND BROKEN
string NCU::read(string id) {
	bool goodchar;
    int c;
    int curpos = 0;
	int strpos = 0;
	int maxlen;
    string s = "", ns;
    WINDOW *win;


	string sa = ""; // before cursor
	string sb = ""; // after cursor

    // setup
    win = getWin(id);
    Element *e = getElement(id);
	maxlen = e->width-5;
    cbreak();
    if(!cursor) curs_set(1);
    wmove(win, 1, curpos + 2);
    
	// handle key presses
    while (1) {
		goodchar = false;
		wrefresh(win);
        c = getch();
		
		// string modification
		if (strpos > s.size()) strpos--;
		if (strpos < 0) strpos = 0;
			
		sa = s.substr(0, strpos);
		sb = s.substr(strpos, s.size()-1);

		// visible chars
        if (c < 127 && c > 31) {
			goodchar = true;
			curpos++;
			strpos++;
        }

		// commanding chars
        else {
            switch(c) {
                case 10: // enter
                    curs_set(0);
                    return s;
                case 27: // esc
                    clearElement(id);
                    if (!cursor) curs_set(0);
                    return "";
                case KEY_BACKSPACE: // backspace
                    if (sa.size() > 0) {
                        //s = s.substr(0, s.size() - 1);
						sa = sa.substr(0, sa.size()-1);
                        curpos--;
						strpos--;
                    }
                    break;
                case KEY_LEFT:
					curpos--;
					strpos--;
                    break;
				case KEY_RIGHT:
					curpos++;
					strpos++;
					break;
                default:
                    break;
            }
        }

		
		// visible char to add to string at current position
		if (goodchar) {
			sa += (char)c;
		}
		s = sa + sb;

		ns = s;

		// check cursor bounds and set visible substring
		if (curpos > s.size() || curpos > maxlen) {
			// set correct position
			curpos = maxlen;

			// set visible portion of string
			if (sa.size() > e->width-4) {
				ns = sa.substr(sa.size()-1-maxlen, sa.size()-1);
			}
			/*
			else {
				ns = sa + sb.substr(0, sb.size()-e->width-5);
			}
			*/
		}
		if (curpos < 0) curpos = 0;

		// set visible range
		if (s.size() > maxlen) {
			int asize = curpos;
			int bsize = maxlen - curpos;
			// IDK STOPPED HERE

			ns = sa.substr(sa.size()-asize-1, sa.size()-1) +
				 sb.substr(0, sb.size()-bsize-1);
		}

        clearElement(id);
        
		/*
        // makes sure the string does not exceed the box size
        if (s.size() > e->width-4) ns = s.substr(s.size()-(e->width-4-1), s.size()-1);
        else ns = s;
		*/

        // write to box
        this->write(id, ns, 2, 1);

		wmove(win, 1, curpos + 2);
    }

    
    if (!cursor) curs_set(0);
}

// CHARACTERS ARE DISAPPEARING THROUGH SUBSTRINGS
// AND I HAVE NO IDEA WHY
string NCU::testRead(string id, string idb) {
	bool goodchar;
    int c;
    int curpos = 0;
	int strpos = 0;
	int maxlen;
    string s = "", ns;
    WINDOW *win;


	string sa = ""; // before cursor
	string sb = ""; // after cursor

    // setup
    win = getWin(id);
    Element *e = getElement(id);
    Element *eb = getElement(idb);
	maxlen = e->width-5;
    cbreak();
    curs_set(1);
    wmove(win, 1, curpos + 2);
    
	// handle key presses
    while (1) {
		goodchar = false;
		wrefresh(win);
        c = getch();

		// visible chars
        if (c < 127 && c > 31) {
			goodchar = true;
			strpos++;
        }

		// commands
		else {
			switch(c) {
				case 10: // enter
					curs_set(0);
					return s;
				case KEY_LEFT:
					strpos--;
					break;
				case KEY_RIGHT:
					strpos++;
					break;
				default:
					break;
			}
		}

		/* SEG FAULT AVOIDANCE SYSTEM */

		if (strpos < 0) strpos = 0;
		if (strpos > s.size()) strpos = s.size();

		/* PREPARE THE STRING PARTS */

		// after cursor
		sb = s.substr(strpos, s.size());
		
		// before cursor
		sa = s.substr(0, strpos);
		if (goodchar) {
			sa += (char)c;
			if (strpos == s.size())
				strpos++;
		}

		/* MAKING THE STRING */
	
		s = sa + sb;
		
		// DEBUG
        clearElement(idb);
		string tmpa = s + " (" + sa + ") (" + sb + ")";
		string tmpb = "CURSOR: " + to_string(curpos) + "\n  STRING: " + to_string(strpos);
		this->write(idb, tmpa, 2, 1);
		this->write(idb, tmpb, 2, 2);


		// bounds

		ns = s;












#if 0
		
		if (strpos != -1) {
			sa = s.substr(0, strpos-1);
			sb = s.substr(strpos, s.size());
		}

		// visible chars
        if (c < 127 && c > 31) {
			goodchar = true;
			curpos++;
			strpos++;
        }

		// commanding chars
        else {
            switch(c) {
                case 10: // enter
                    curs_set(0);
                    return s;
                case 27: // esc
                    clearElement(id);
                    if (!cursor) curs_set(0);
                    return "";
                case KEY_BACKSPACE: // backspace
                    if (sa.size() > 0) {
                        //s = s.substr(0, s.size() - 1);
						sa = sa.substr(0, sa.size()-1);
                        curpos--;
						strpos--;
                    }
                    break;
                case KEY_LEFT:
					curpos--;
					strpos--;
                    break;
				case KEY_RIGHT:
					curpos++;
					strpos++;
					break;
                default:
                    break;
            }
        }

		
		// visible char to add to string at current position
		if (goodchar) {
			sa += (char)c;
		}
		s = sa + sb;

		ns = s;

		// check cursor bounds and set visible substring
		if (curpos > s.size() || curpos > maxlen) {
			// set correct position
			curpos = maxlen;

			// set visible portion of string
			if (sa.size() > e->width-4) {
				ns = sa.substr(sa.size()-1-maxlen, sa.size()-1);
			}
			/*
			else {
				ns = sa + sb.substr(0, sb.size()-e->width-5);
			}
			*/
		}
		if (curpos < 0) curpos = 0;

		// set visible range
		if (s.size() > maxlen) {
			int asize = curpos;
			int bsize = maxlen - curpos;
			// IDK STOPPED HERE

			ns = sa.substr(sa.size()-asize-1, sa.size()-1) +
				 sb.substr(0, sb.size()-bsize-1);
		}
#endif

        clearElement(id);
        
		/*
        // makes sure the string does not exceed the box size
        if (s.size() > e->width-4) ns = s.substr(s.size()-(e->width-4-1), s.size()-1);
        else ns = s;
		*/

        // write to box
        this->write(id, ns, 2, 1);


		wmove(win, 1, strpos + 2); // CHANGED CURPOS TO STRPOS HERE
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

// hideElement
// hides an element
void NCU::hideElement(string id) {
    PANEL *p = getPanel(id);
    if (p != NULL) hide_panel(p);

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

// form functions --------------------------------------------------------------------

void NCU::addForm(string eid, string fid, formType ft, int posx, int posy) {
	Element *e;
	Form *f;

	// setup
	e = getElement(eid);
	
	if (e != NULL) {
		cout << "in\n";
		f = new Form;
		
		// info filler
		f->id = fid;
		string *s;
		*s = "empty";
		f->data = s;
		f->ftype = ft;
		f->posx = posx;
		f->posy = posy;

		e->formList.insert(make_pair(fid, f));
	}
}

void NCU::linkForm(string eid, string fid, string *data) {
	Form *f = getForm(eid, fid);
	
	if (f != NULL) f->data = data;
}

void NCU::updateForms(string eid) {
	Element *e = getElement(eid);
	map<string, Form*>::iterator fit;
	
	if (e == NULL) return;

	for (fit = e->formList.begin(); fit != e->formList.end(); e++) {
		this->write(eid, fit->second->id + ": " + *(fit->second->data), fit->second->posy, fit->second->posx);
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

void NCU::showGroup(string id) {
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

void NCU::hideGroup(string id) {
    map<string, Group*>::iterator git;
    PANEL *p;

    // get the group
    git = groupList.find(id);

    // show the group
    if (git != groupList.end()) {
        for (int i = 0; i < git->second->elements.size(); i++) {
            p = getPanel(git->second->elements[i]);
            if (p != NULL) hide_panel(p);
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

Form* NCU::getForm(string eid, string fid) {
	map<string, Element*>::iterator e;
	map<string, Form*>::iterator f;

	e = elementList.find(eid);
	if (e != elementList.end()) {
		f = e->second->formList.find(fid);
		if (f != e->second->formList.end()) {
			return f->second;
		}
		else return NULL;
	}
	else return NULL;
}

void NCU::startDebug() {
	def_prog_mode();
	endwin();
}

void NCU::endDebug() {
	reset_prog_mode();
	refresh();
}

// postional

int NCU::above(string id) {
    Element *e = getElement(id);
    return e->posy;
}

int NCU::below(string id) {
    Element *e = getElement(id);
    return e->posy + e->sizey;
}

int NCU::leftof(string id) {
    Element *e = getElement(id);
    return e->posx;
}

int NCU::rightof(string id) {
    Element *e = getElement(id);
    return e->posx + e->sizex;
}

void NCU::test(string id) {
    int c;
    WINDOW *win;

    // setup
    win = getWin(id);
    cbreak();
    
    while (1) {
        c = getch();
		string s = "";
		s += c;

		this->clearElement(id);

		if (c == KEY_LEFT) {
			this->write(id, "LEFT", 2, 1);
		}
		else {
			this->write(id, s, 2, 1);
		}
	}

}
