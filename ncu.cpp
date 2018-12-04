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
	
	// start controller thread
	thControl = thread(controlThread);
}

void NCU::stop() {
	// tell the threads to stop
	Command *c = new Command;
	c->command = KILL;
	commands.push(c);

	thModel.join();
	thControl.join();
}


// element things --------------------------------------------------------------------

// addElement
// creates a new element and adds it to the element list
void NCU::addElement(string id, borderType bt, int sizex, int sizey, int posx, int posy) {
	Command *c = new Command;

	c->command = CREATE_ELEMENT;
	c->id = id;
	c->bType = bt;
	c->x = sizex;
	c->y = sizey;
	c->posx = posx;
	c->posy = posy;

	commands.push(c);
}

void NCU::internalAddElement(Command *c) {
    Element *e;

    // error checking
    //check_if_started();

    // setup
    e = new Element;

    // fill in info
    e->id = c->id;
    e->title = "";
    e->btype = c->bType;
    e->width = c->x;

    // dimensions
    e->sizex = c->x;
    e->sizey = c->y;
    e->posx = c->posx;
    e->posy = c->posy;

    // win creation
    e->win = newwin(c->y, c->x, c->posy, c->posx);
    wrefresh(e->win);
    e->panel = new_panel(e->win);
    hide_panel(e->panel);
	
	// DEBUG
	//if (e->win == NULL) cout << "IN MAKE THIS IS NULL\n";

    // add to map
    elementList.insert(make_pair(c->id, e));

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

// showElement
// initially shows an element
void NCU::showElement(string id) {
	Command *c = new Command;
	c->command = SHOW_ELEMENT;
	c->id = id;
	commands.push(c);
}

void NCU::internalShowElement(Command *c) {
	PANEL *p;

	p = getPanel(c->id);
	if (p != NULL) show_panel(p);

	// update
	update_panels();
	doupdate();
}

// hideElement
// hides an element
void NCU::hideElement(string id) {
	Command *c = new Command;
	c->command = HIDE_ELEMENT;
	c->id = id;
	commands.push(c);
}

void NCU::internalHideElement(Command *c) {
	PANEL *p;

	p = getPanel(c->id);
	if (p != NULL) hide_panel(p);

	// update
	update_panels();
	doupdate();
}


// pop-ups ---------------------------------------------------------------------------


void NCU::notice(string s) {
	mNotice.lock();
	thread tn(noticeThread, s, this);
	tn.join();
	mNotice.unlock();
}

// thready things --------------------------------------------------------------------


void NCU::modelThread(NCU *ncu) {
	PANEL *p;

	ncu->startView();
	//cout << "model\n";

	while(1) {
		if (commands.size() == 0) continue;


		Command *c = commands.front();
		//cout << "THING NOT EMPTY: " << c->command << endl;
		commands.pop();

		// do things
		ncu->mMain.lock();
		switch(c->command) {
			case KILL:
				//cout << "full stop\n";
				ncu->NCU_STARTED = false;
				break;
			case CREATE_ELEMENT:
				ncu->internalAddElement(c);
				break;
			case SHOW_ELEMENT:
				ncu->internalShowElement(c);
				break;
			case HIDE_ELEMENT:
				ncu->internalHideElement(c);
			default:
				// literally no idea
				break;

		}
		ncu->mMain.unlock();

		// kill this thread and ncurses  when ready
		if (!ncu->NCU_STARTED) {
			// get all the mutexes
			ncu->mNotice.lock();
			ncu->mMain.lock();

			endwin();
			break;

			// release the mutexes
			ncu->mNotice.unlock();
			ncu->mMain.unlock();
		}
	}
}

void NCU::startView() {
	mMain.lock();

    initscr();
    raw();
    keypad(stdscr, TRUE);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_BLACK, COLOR_YELLOW);
    init_pair(3, COLOR_WHITE, COLOR_RED);
    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    noecho();
    refresh();
    curs_set(0);
    NCU_STARTED = true;
    r = LINES;
    c = COLS;

	mMain.unlock();
}

void NCU::controlThread() {

}

void NCU::noticeThread(string s, NCU *ncu) {
	//cout << "NOTICE FUNCTION\n";
	Command *c = new Command;

	// create the box
	ncu->addElement("NCU_DO_NOT_USE_NOTICE", NCU_BORDER_BOX, 40, 5, 0, 0);
	ncu->showElement("NCU_DO_NOT_USE_NOTICE");

	usleep(3000000);

	ncu->hideElement("NCU_DO_NOT_USE_NOTICE");

	// FIXME: THIS WILL ONLY WORK ONCE. I NEED TO INITIALIZE THIS ELSEWHERE
}


// utilitarian crap ------------------------------------------------------------------


PANEL* NCU::getPanel(string id) {
	//cerr << "IN GET PANEL: ";
    map<string, Element*>::iterator e;

    e = elementList.find(id);
    if (e != elementList.end()) {
		//cout << "THING FOUND!!!!!!!!!! ";
		//if (e->second->panel == NULL) cout << "THIS IS NULL\n";
		return e->second->panel;
	}
    else return NULL;
}
