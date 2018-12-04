// ncu.cpp
// Author:  Benjamin Johnson
// Purpose: Abstract the ncurses library to streamline command line interface creation


#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <ncurses.h>
#include <panel.h>
#include <unistd.h>
#include "ncu.h"

using namespace std;


// global bois
queue<Command*> commands;


void NCU::start() {
	// start model thread
	thModel = thread(modelThread, this);
	
	// start view thread
	thView = thread(viewThread, this);

	// start controller thread
	thControl = thread(controlThread);
}

void NCU::stop() {
	// tell the threads to stop
	Command *c = new Command;
	c->command = "FULLSTOP";
	commands.push(c);

	thView.join();
	thModel.join();
	thControl.join();
}


// element things --------------------------------------------------------------------

#if 0
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
#endif


// pop-ups ---------------------------------------------------------------------------


void NCU::notice(string s) {
	cout << "NOTICE FUNCTION\n";
	Command *c = new Command;

	c->command = "notice";
	c->args.push_back(s);
	commands.push(c);
}

// thready things --------------------------------------------------------------------


void NCU::modelThread(NCU *ncu) {
	cout << "model\n";

	while(1) {
		if (commands.size() == 0) continue;


		Command *c = commands.front();
		cout << "THING NOT EMPTY: " << c->command << endl;
		commands.pop();

		if (c->command == "FULLSTOP") {
			cout << "full stop\n";
			ncu->NCU_STARTED = false;
			break;
		}
		if (c->command == "notice") {
			ncu->mNotice.lock();
			cout << "  in notice if\n";
			usleep(3000000);
			ncu->mNotice.unlock();
		}
	}

}

void NCU::viewThread(NCU *ncu) {
	ncu->mMain.lock();

    initscr();
    raw();
    keypad(stdscr, TRUE);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_BLACK, COLOR_YELLOW);
    init_pair(3, COLOR_WHITE, COLOR_RED);
    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    //noecho();
    refresh();
    ncu->NCU_STARTED = true;
    ncu->r = LINES;
    ncu->c = COLS;

	ncu->mMain.unlock();

	// DEBUG
	
	while(1) {
		ncu->mNotice.lock();
		if (ncu->NCU_STARTED == false) {
			endwin();
			break;
		}
		ncu->mNotice.unlock();
	}
}

void NCU::controlThread() {

}
