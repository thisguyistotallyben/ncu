// Author:  Benjamin Johnson
// Date:    July 29, 2017
// Purpose: Lays out useful functions for handling the interface


#ifndef NCURSES_UTILS_H
#define NCURSES_UTILS_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <ncurses.h>
#include <panel.h>
using namespace std;

enum BoxType {NO_BOX, BOX, INPUT_BOX};

class Window {
	public:
		WINDOW *win;
		PANEL  *panel;
		int h;
		int w;
		int y;
		int x;
		BoxType b; //	STOPPED HERE
};

class Layout {
	public:
		vector<string> l;
		bool overwrite;
		bool clearScreen;
};

class NcursesUtils {
	private:
		map<string, Window> wins;
		map<string, Layout> layouts;
		map<string, bool> visible;

	public:
		void init();
		void addWin(string name, int h, int w, int y, int x, BoxType b);
		void addLayout(string name, vector<string> winlist, bool owb, bool csb);
		void showLayout(string layout);
		void hideLayout(string layout);
		void showWin(string name);
		void hideWin(string name);
		void boxWin(string name, BoxType b);
		void unboxWin(string name);
		bool showing(string layout);
		void clearWin(string name);
		void printWin(string name, int h, int w, string s);
		void clrPrintWin(string name, int h, int w, string s);
		string getInput(string name);
		int getWinWidth(string name);

		WINDOW* getWin(string name);
};

#endif
