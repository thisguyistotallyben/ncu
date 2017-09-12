// Author:  Benjamin Johnson
// Date:    July 31, 2017
// Purpose: Defines useful functions for handling the interface


#include <iostream>
#include <map>
#include <sstream>
#include <ncurses.h>
#include <panel.h>
#include "ncurses_utils.h"
using namespace std;

void NcursesUtils::init() {
    initscr();
    raw();
    keypad(stdscr, TRUE);
	start_color();
	init_pair(1, COLOR_BLACK, COLOR_WHITE);
    noecho();
	refresh();
}

void NcursesUtils::addWin(string name, int h, int w, int y, int x, BoxType b) {
	Window nwin;
	
	// make window
	nwin.win = newwin(h, w, y, x);
	nwin.b = b;
	
	// add to panel
	nwin.panel = new_panel(nwin.win);
	hide_panel(nwin.panel);

	wins.insert(make_pair(name, nwin));
	
	// box it
	boxWin(name, b);
}

void NcursesUtils::addLayout(string name, vector<string> winlist, bool owb, bool csb) {
	Layout nlay;

	// make layout
	nlay.l = winlist;
	nlay.overwrite = owb;
	nlay.clearScreen = csb;

	// insert
	layouts.insert(make_pair(name, nlay));
	visible.insert(make_pair(name, false));
}

void NcursesUtils::showLayout(string name) {
	map<string, Layout>::iterator lit, slit;

	lit = layouts.find(name);

	// clear screen
	// TODO: restructure probably
	if (lit->second.clearScreen) {
		for (slit = layouts.begin(); slit != layouts.end(); slit++) {
			if (slit->second.overwrite && visible.find(slit->first)->second == true) {
				hideLayout(slit->first);
			}
		}
	}

	for (int i = 0; i < lit->second.l.size(); i++) {
		show_panel(wins.find(lit->second.l[i])->second.panel);
	}

	// set showing bool
	visible[name] = true;

	// update screen
	update_panels();
	doupdate();
}

void NcursesUtils::hideLayout(string name) {
	map<string, Layout>::iterator lit;

	lit = layouts.find(name);


	for (int i = 0; i < lit->second.l.size(); i++) {
		hide_panel(wins.find(lit->second.l[i])->second.panel);
	}

	// set showing bool
	visible.find(name)->second = false;

	// update screen
	update_panels();
	doupdate();
}

bool NcursesUtils::showing(string name) {
	return visible.find(name)->second;
}

void NcursesUtils::printWin(string name, int h, int w, string s) {
	mvwprintw(getWin(name), h, w, s.c_str());
	wrefresh(getWin(name));
}

void NcursesUtils::clearWin(string name) {
	Window w = wins.find(name)->second;

	werase(w.win);
	boxWin(name, w.b);

	wrefresh(w.win);
}

void NcursesUtils::boxWin(string name, BoxType b) {
	// TODO: ADD BOX TYPE CHANGE STAY

	switch (b) {
		case NO_BOX:
			break;
		case BOX:
			box(getWin(name), 0 , 0);
			break;
		case INPUT_BOX:
			wborder(getWin(name), ' ', ' ', ' ', 0, ' ', ' ', 0, 0);
			break;
		default: break;
	}
}

// lol doesn't work for some reason
void NcursesUtils::clrPrintWin(string name, int h, int w, string s) {
	clearWin(name);
	printWin(name, h, w, s);
}

// go-getters -------------------------------------------------------

WINDOW* NcursesUtils::getWin(string name) {
	return wins.find(name)->second.win;
}

string NcursesUtils::getInput(string name) {
	int cc = 2;
	char ch;
	string s;
	stringstream ss;
	
	// setup
	//echo();
	cbreak();
	curs_set(1);
	wmove(getWin(name), 1, 2);
	//wrefresh(getWin(name));

	// get it
//	wgetstr(getWin(name), s);
	while (1) {
		ch = wgetch(getWin(name));
		switch (ch) {
			case 10: // enter
				clearWin(name);
				curs_set(0);
				return s;
			case 27: // esc
				clearWin(name);
				curs_set(0);
				return "";
			case 127: // backspace
				if (s.size() > 0) {
					s = s.substr(0, s.size() - 1);
					cc--;
				}
				break;
			default:
				s = s + ch;
		}
		cc++;

		// display
		clearWin(name);
		printWin(name, 1, 2, s);
	}
}
